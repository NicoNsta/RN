#pragma once
#include <QVector>
#include <QVector4D>
#include "SceneManager.h"

// Knoten im 3d-kd-Tree
struct KdNode {
    QVector4D point;                        // der „Median“-Punkt, an dem wir splitten
    float      splitValue;                  // die Koordinate dieses Punktes auf der Split-Achse
    int        axis;                        // 0 = X-Achse, 1 = Y-Achse, 2 = Z-Achse
    KdNode*    left  = nullptr;
    KdNode*    right = nullptr;
    ~KdNode() { delete left; delete right; }
};

// Baumaufbau
KdNode* buildKdTree(const QVector<QVector4D>& pts,
                    QVector<int>& idxX,
                    QVector<int>& idxY,
                    QVector<int>& idxZ,
                    int l, int r, int depth);

// Partitionierung (Hilfsfunktion)
void partitionIndex(QVector<int>& idx,
                    const QVector<QVector4D>& pts,
                    int axis, float split, int l, int r);

// Freie Funktion zur Visualisierung der Splitting-Ebenen
// Zeichnet Level 0…maxDepth über SceneManager
void visualizeKdTree(KdNode*   node,
                     int       depth,
                     int       maxDepth,
                     const QVector4D& bbMin,
                     const QVector4D& bbMax,
                     SceneManager&    scene);

