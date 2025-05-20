#pragma once
#include <QVector>
#include <QVector4D>
#include "SceneManager.h"
#include "Cube.h"

// Ein Oct-Tree–Knoten
struct OctNode {
    QVector<int>   indices;   // Indizes der Punkte in diesem Würfel
    QVector4D      bbMin;     // AABB-Untere Ecke
    QVector4D      bbMax;     // AABB-Obere Ecke
    OctNode*       children[8]{};  // bis zu 8 Teilwürfel

    ~OctNode() {
        for (auto*& c : children)
            delete c;
    }
};

// Rekursiver Aufbau bis maxDepth
OctNode* buildOctTree(const QVector<QVector4D>& pts,
                      const QVector4D& bbMin,
                      const QVector4D& bbMax,
                      const QVector<int>& indices,
                      int depth,
                      int maxDepth);

// Zeichnet alle Knoten bis Tiefe maxDepth als Würfel in den SceneManager
// (nur Deklaration – keine Implementierung im Header!)
void visualizeOctTree(OctNode* node,
                      int depth,
                      int maxDepth,
                      SceneManager& scene);
