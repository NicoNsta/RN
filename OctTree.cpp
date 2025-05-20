#include "OctTree.h"
#include "Cube.h"
#include <QMatrix4x4>
#include <algorithm>

// 1) Rekursiver Baumaufbau
OctNode* buildOctTree(const QVector<QVector4D>& pts,
                      const QVector4D& bbMin,
                      const QVector4D& bbMax,
                      const QVector<int>& indices,
                      int depth,
                      int maxDepth)
{
    if (indices.isEmpty() || depth > maxDepth)
        return nullptr;
    auto* node = new OctNode{indices, bbMin, bbMax, {}};
    if (depth == maxDepth || indices.size() <= 5)
        return node;

    float midX = 0.5f * (bbMin.x() + bbMax.x());
    float midY = 0.5f * (bbMin.y() + bbMax.y());
    float midZ = 0.5f * (bbMin.z() + bbMax.z());

    QVector<int> childIdx[8];
    for (int i : indices) {
        const auto& p = pts[i];
        int code = (p.x() > midX ? 1 : 0)
                   | (p.y() > midY ? 2 : 0)
                   | (p.z() > midZ ? 4 : 0);
        childIdx[code].append(i);
    }
    for (int i = 0; i < 8; ++i) {
        QVector4D cMin = bbMin, cMax = bbMax;
        if (i & 1) cMin.setX(midX); else cMax.setX(midX);
        if (i & 2) cMin.setY(midY); else cMax.setY(midY);
        if (i & 4) cMin.setZ(midZ); else cMax.setZ(midZ);
        node->children[i] = buildOctTree(pts, cMin, cMax, childIdx[i], depth+1, maxDepth);
    }
    return node;
}

// 2) Visualisierung der ersten Ebenen als Würfel
void visualizeOctTree(OctNode* node,
                      int depth,
                      int maxDepth,
                      SceneManager& scene)
{
    if (!node || depth > maxDepth) return;

    // 1) Bounding-Box und Mittelpunkt
    QVector4D mn = node->bbMin, mx = node->bbMax;
    QVector4D center = (mn + mx) * 0.5f;
    QVector3D halfSize(
        (mx.x() - mn.x()) * 0.5f,
        (mx.y() - mn.y()) * 0.5f,
        (mx.z() - mn.z()) * 0.5f
        );

    // 2) Cube mit Ursprung (0,0,0) und Seitenlänge 1 anlegen
    Cube* cube = new Cube(QVector4D(0,0,0,1), 1.0f);

    // 3) **erst** skalieren, **dann** verschieben
    //    (Matrix-Multiplikation: translate * scale)
    QMatrix4x4 M;
    M.translate(center.x(), center.y(), center.z());
    M.scale(halfSize.x(), halfSize.y(), halfSize.z());
    cube->affineMap(M);

    // 4) in die Szene hängen
    scene.push_back(cube);

    // 5) Rekursion für 8 Teilwürfel
    for (int i = 0; i < 8; ++i) {
        visualizeOctTree(node->children[i], depth+1, maxDepth, scene);
    }
}

