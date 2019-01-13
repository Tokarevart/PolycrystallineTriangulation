#pragma once

struct PolycrMesh
{
    size_t  nNodes;

    // { x0, y0, z0, x1, y1, z1 ... }
    double* nodesPositions;
    size_t  nTetrs;
    size_t  nCryses;

    // Number of tetrahedrons in each crystalline.
    size_t* nCrysesTetrs;

    // c - crystallite,
    // t - tetrahedron,
    // n - node.
    // { ... c[i]_t[0]_n[0..3] ... c[i]_t[nCrysesTetrs[i]]_n[0..3] }
    size_t* tetrs;

    PolycrMesh();
    ~PolycrMesh();
};