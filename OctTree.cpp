#include "OctTree.h"
#include "Cube.h"
#include <QMatrix4x4>
#include <algorithm>

// ------------------------------------------------------------------
// 1) Rekursiver Aufbau des Oct-Trees
// ------------------------------------------------------------------
// pts       : Referenz auf die Punktwolke (homogene 4D-Punkte)
// bbMin, bbMax : Achsen-parallele Bounding‐Box (AABB) des aktuellen Knotens
// indices  : Liste der Punkt-Indizes, die in dieser Box liegen
// depth    : Aktuelle Tiefe im Baum (0 = Root)
// maxDepth : Maximal gewünschte Tiefe
//
// Rückgabe : Zeiger auf den erstellten OctNode oder nullptr, wenn keine Punkte
OctNode* buildOctTree(const QVector<QVector4D>& pts,
                      const QVector4D& bbMin,
                      const QVector4D& bbMax,
                      const QVector<int>& indices,
                      int depth,
                      int maxDepth)
{
    // Abbruch, wenn keine Punkte mehr oder Tiefe überschritten
    if (indices.isEmpty() || depth > maxDepth)
        return nullptr;

    // Knoten mit aktueller Indexliste und AABB anlegen
    auto* node = new OctNode{ indices, bbMin, bbMax, {} };

    // Abbruch, wenn maxDepth erreicht oder zu wenige Punkte
    if (depth == maxDepth || indices.size() <= 5)
        return node;

    // Mittelpunkt-Koordinaten der AABB berechnen
    float midX = 0.5f * (bbMin.x() + bbMax.x());
    float midY = 0.5f * (bbMin.y() + bbMax.y());
    float midZ = 0.5f * (bbMin.z() + bbMax.z());

    // Indizes in 8 Teil-Oktanten aufteilen
    QVector<int> childIdx[8];
    for (int idx : indices) {
        const auto& p = pts[idx];
        int code = (p.x() > midX ? 1 : 0)
                   | (p.y() > midY ? 2 : 0)
                   | (p.z() > midZ ? 4 : 0);
        childIdx[code].append(idx);
    }

    // Rekursion: für jeden Oktanten seine neue AABB berechnen
    for (int i = 0; i < 8; ++i) {
        QVector4D cMin = bbMin, cMax = bbMax;
        if (i & 1) cMin.setX(midX); else cMax.setX(midX);
        if (i & 2) cMin.setY(midY); else cMax.setY(midY);
        if (i & 4) cMin.setZ(midZ); else cMax.setZ(midZ);

        node->children[i] =
            buildOctTree(pts, cMin, cMax, childIdx[i], depth + 1, maxDepth);
    }

    return node;
}


// ------------------------------------------------------------------
// 2) Visualisierung der ersten Ebenen des Oct-Trees
// ------------------------------------------------------------------
// node     : aktueller Knoten im Oct-Tree
// depth    : aktuelle Rekursionstiefe (0 = Root)
// maxDepth : maximale darzustellende Tiefe (inklusive)
// scene    : SceneManager, der die Cubes rendert
void visualizeOctTree(OctNode* node,
                      int depth,
                      int maxDepth,
                      SceneManager& scene)
{
    // Abbruch, wenn kein Knoten oder Tiefe überschritten
    if (!node || depth > maxDepth)
        return;

    // AABB-Koordinaten auslesen
    QVector4D mn = node->bbMin;
    QVector4D mx = node->bbMax;

    // Volle Kantenlänge des AABB berechnen
    QVector3D size3(
        mx.x() - mn.x(),
        mx.y() - mn.y(),
        mx.z() - mn.z()
        );

    // Unit-Cube um den Ursprung anlegen (Eckpunkte [0..1]^3)
    Cube* cube = new Cube(QVector4D(0,0,0,1), 1.0f);

    // Transformationsmatrix:
    //  a) Translate zum AABB-Min (untere Ecke)
    //  b) Scale auf volle Kantenlänge
    QMatrix4x4 M;
    M.translate(mn.x(), mn.y(), mn.z());
    M.scale(size3.x(), size3.y(), size3.z());
    cube->affineMap(M);

    // Cube in die Szene einhängen
    scene.push_back(cube);

    // Rekursive Visualisierung aller vorhandenen 8 Kinder
    for (int i = 0; i < 8; ++i) {
        visualizeOctTree(node->children[i], depth + 1, maxDepth, scene);
    }
}
