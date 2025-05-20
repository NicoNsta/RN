#include "KdTree.h"
#include "Plane.h"
#include <QMatrix4x4>
#include <algorithm>

// ------------------------------------------------------------------
// Partitioniere idx[l..r] in-place entlang pts[*][axis] <= split
// ------------------------------------------------------------------
void partitionIndex(QVector<int>& idx,
                    const QVector<QVector4D>& pts,
                    int axis, float split, int l, int r)
{
    int i = l, j = r;
    while (i <= j) {
        while (i <= j && pts[idx[i]][axis] <= split) ++i;
        while (i <= j && pts[idx[j]][axis] >  split) --j;
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
    int m    = (l + r) / 2;
    float split = pts[idx[m]][axis];

    auto* node = new KdNode{ pts[idx[m]], split, axis };

    // Partitioniere die drei Listen
    partitionIndex(idxX, pts, axis, split, l, r);
    partitionIndex(idxY, pts, axis, split, l, r);
    partitionIndex(idxZ, pts, axis, split, l, r);

    node->left  = buildKdTree(pts, idxX, idxY, idxZ, l,   m-1, depth+1);
    node->right = buildKdTree(pts, idxX, idxY, idxZ, m+1, r,   depth+1);
    return node;
}

// ------------------------------------------------------------------
// Visualisiert die ersten maxDepth-Ebenen des kd-Trees
// ------------------------------------------------------------------
void visualizeKdTree(KdNode* node,
                     int depth,
                     int maxDepth,
                     const QVector4D& bbMin,
                     const QVector4D& bbMax,
                     SceneManager& scene)
{
    if (!node || depth > maxDepth) return;

    // Ebene als Plane: Ursprung (Mittelpunkt) + Normal
    QVector4D origin, normal;
    float sx, sy;
    switch (node->axis) {
    case 0:
        origin = { node->splitValue,
                  0.5f*(bbMin.y()+bbMax.y()),
                  0.5f*(bbMin.z()+bbMax.z()),
                  1.0f };
        normal = {1,0,0,0};
        sx = 0.5f*(bbMax.y()-bbMin.y());
        sy = 0.5f*(bbMax.z()-bbMin.z());
        break;
    case 1:
        origin = { 0.5f*(bbMin.x()+bbMax.x()),
                  node->splitValue,
                  0.5f*(bbMin.z()+bbMax.z()),
                  1.0f };
        normal = {0,1,0,0};
        sx = 0.5f*(bbMax.x()-bbMin.x());
        sy = 0.5f*(bbMax.z()-bbMin.z());
        break;
    default:
        origin = { 0.5f*(bbMin.x()+bbMax.x()),
                  0.5f*(bbMin.y()+bbMax.y()),
                  node->splitValue,
                  1.0f };
        normal = {0,0,1,0};
        sx = 0.5f*(bbMax.x()-bbMin.x());
        sy = 0.5f*(bbMax.y()-bbMin.y());
        break;
    }

    Plane* plane = new Plane(origin, normal);
    QMatrix4x4 M;
    M.scale(sx, sy, 1.0f);
    plane->affineMap(M);
    scene.push_back(plane);

    // Bounding-Box für linke/rechte Teilbäume anpassen
    QVector4D leftMin = bbMin, leftMax = bbMax;
    QVector4D rightMin = bbMin, rightMax = bbMax;
    if      (node->axis == 0) leftMax.setX(node->splitValue), rightMin.setX(node->splitValue);
    else if (node->axis == 1) leftMax.setY(node->splitValue), rightMin.setY(node->splitValue);
    else                      leftMax.setZ(node->splitValue), rightMin.setZ(node->splitValue);

    // Rekursion
    visualizeKdTree(node->left,  depth+1, maxDepth, leftMin,  leftMax,  scene);
    visualizeKdTree(node->right, depth+1, maxDepth, rightMin, rightMax, scene);
}
