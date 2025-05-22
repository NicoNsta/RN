#include "KdTree.h"
#include "Plane.h"
#include <QMatrix4x4>
#include <algorithm>



// Hilfsklasse: speichert Farbe + Transparenz, ruft dann Plane::draw mit genau diesen Parametern auf
class KdPlane : public Plane {
public:
    QColor    col;
    float     transp;
    KdPlane(const QVector4D& origin,
            const QVector4D& normal,
            const QColor&    c,
            float            t)
        : Plane(origin, normal),
        col(c),
        transp(t)
    {
        // sorgt dafür, dass SceneManager::draw() uns als Plane behandelt
        type = SceneObjectType::ST_PLANE;
    }

    // Überschreibt draw und ignoriert die übergebenen Default-Parameter
    virtual void draw(const RenderCamera& renderer,
                      const QColor&        /*color*/,
                      float                /*transparency*/) const override
    {
        Plane::draw(renderer, col, transp);
    }
};


// ------------------------------------------------------------------
// Partitioniere idx[l..r] in-place entlang pts[*][axis] <= split
// ------------------------------------------------------------------
void partitionIndex(QVector<int>& idx,
                    const QVector<QVector4D>& pts,
                    int axis, float split, int l, int r)
{
    int i = l, j = r;
    while (i <= j) {
        // suche von links den ersten, der > split ist
        while (i <= j && pts[idx[i]][axis] <= split) ++i;
        // suche von rechts den ersten, der ≤ split ist
        while (i <= j && pts[idx[j]][axis] >  split) --j;
        // tausche beide, falls sie sich überschneiden
        if (i < j) {
            std::swap(idx[i], idx[j]);
            ++i; --j;
        }
    }
}

// ------------------------------------------------------------------
// Rekursiver Aufbau eines balancierten kd-Trees
// ------------------------------------------------------------------
KdNode* buildKdTree(const QVector<QVector4D>& pts,
                    QVector<int>& idxX,
                    QVector<int>& idxY,
                    QVector<int>& idxZ,
                    int l, int r, int depth)
{
    if (l > r) return nullptr;
    int axis = depth % 3;
    auto& idx = (axis == 0 ? idxX : axis == 1 ? idxY : idxZ);
    int m    = (l + r) / 2;                                 // Median-Index im gewählten Array
    float split = pts[idx[m]][axis];                        // Median

     // 1) Erzeuge neuen Knoten mit diesem Median
    auto* node = new KdNode{ pts[idx[m]], split, axis };

    // 2) Partitioniere alle drei Index-Arrays entlang dieser Achse --> Damit in jedem Array rechts/links konsistent aufgeteilt bleibt
    partitionIndex(idxX, pts, axis, split, l, r);
    partitionIndex(idxY, pts, axis, split, l, r);
    partitionIndex(idxZ, pts, axis, split, l, r);

    // 3) Rekursiv linkes und rechtes Teil-Unterbäume bauen
    node->left  = buildKdTree(pts, idxX, idxY, idxZ, l,   m-1, depth+1);
    node->right = buildKdTree(pts, idxX, idxY, idxZ, m+1, r,   depth+1);

    return node;
}

// ------------------------------------------------------------------
// Visualisiert die ersten maxDepth-Splitting-Ebenen des KD-Trees
// ------------------------------------------------------------------
void visualizeKdTree(KdNode*          node,
                     int              depth,
                     int              maxDepth,
                     const QVector4D& bbMin,
                     const QVector4D& bbMax,
                     SceneManager&    scene)
{
    // 0) Abbruch, wenn kein Knoten oder Tiefe überschritten
    if (!node || depth > maxDepth)
        return;

    // 1) Bestimme die volle Kantenlänge der lokalen AABB
    //    (bbMin und bbMax wurden schon auf den Teilraum zugeschnitten)
    float fullW, fullH;
    if (node->axis == 0) {
        // X-Split → Ebene in YZ → Breite = Y-Range, Höhe = Z-Range
        fullW = bbMax.y() - bbMin.y();
        fullH = bbMax.z() - bbMin.z();
    }
    else if (node->axis == 1) {
        // Y-Split → Ebene in XZ → Breite = X-Range, Höhe = Z-Range
        fullW = bbMax.x() - bbMin.x();
        fullH = bbMax.z() - bbMin.z();
    }
    else {
        // Z-Split → Ebene in XY → Breite = X-Range, Höhe = Y-Range
        fullW = bbMax.x() - bbMin.x();
        fullH = bbMax.y() - bbMin.y();
    }

    // 2) Ursprung = der Median-Punkt im Knoten (node->point)
    //    Normalen-Vektor ebenfalls pro Achse
    QVector4D origin = node->point;
    QVector4D normal = (node->axis == 0 ? QVector4D{1,0,0,0}
                        : node->axis == 1 ? QVector4D{0,1,0,0}
                                          : QVector4D{0,0,1,0});

    // 3) Farbe und Transparenz nach Tiefe
    float t = qMax(0.2f, (255 - (depth * 200 / maxDepth)) / 255.0f);
    QColor col = (node->axis==0 ? QColor(255,0,0)
                  : node->axis==1 ? QColor(0,255,0)
                                    : QColor(0,0,255));

    // 4) Plane direkt am Median-Punkt erzeugen
    //    Intern: Quad von [-1,-1] bis [+1,+1]
    KdPlane* plane = new KdPlane(origin, normal, col, t);

    // 5) Skalieren:  [-1,+1] → [−fullW/2,+fullW/2] in Breite
    //               [−1,+1] → [−fullH/2,+fullH/2] in Höhe
    QMatrix4x4 M;
    M.scale(fullW * 0.5f, fullH * 0.5f, 1.0f);
    plane->affineMap(M);

    // 6) Ebene in die Szene einhängen
    scene.push_back(plane);

    // 7) Lokale AABB in zwei Teilräume splitten
    QVector4D leftMin  = bbMin, leftMax  = bbMax;
    QVector4D rightMin = bbMin, rightMax = bbMax;
    if      (node->axis == 0) { leftMax .setX(node->splitValue); rightMin.setX(node->splitValue); }
    else if (node->axis == 1) { leftMax .setY(node->splitValue); rightMin.setY(node->splitValue); }
    else                      { leftMax .setZ(node->splitValue); rightMin.setZ(node->splitValue); }

    // 8) Rekursion für linke und rechte Teilbäume
    visualizeKdTree(node->left,  depth+1, maxDepth, leftMin,  leftMax,  scene);
    visualizeKdTree(node->right, depth+1, maxDepth, rightMin, rightMax, scene);
}



