#pragma once
#include <QVector>
#include <QVector4D>
#include "SceneManager.h"

// Knoten im 3d-kd-Tree
struct KdNode {
    QVector4D point;    // der Punkt im Knoten
    float      splitValue; // Koordinate entlang axis
    int        axis;    // 0=x, 1=y, 2=z
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
