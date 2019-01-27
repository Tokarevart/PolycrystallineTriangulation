#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include "Definitions.h"
#include "Inclusions.h"
#include "PolyMesh.h"
#include "PolyStruct.h"

using std::ifstream;
using std::ofstream;
using std::unique_ptr;
using std::vector;
using std::string;

enum FileType
{
    OBJ,
    LS_DYNA_KEYWORD
};

class Polycrystal3
{
    double _preferredLength;

    PolyMesh* _lastTriangulation = nullptr;

    vector<Crystallite3*> _crystallites;

    vector<ShellFacet3*> _shellFacets;
    vector<ShellEdge3*> _shellEdges;
    vector<ShellVertex3*> _shellVertexes;

    vector<unique_ptr<Facet3>*> _startFrontFacets;
    vector<unique_ptr<Edge3>*> _startFrontEdges;
    vector<unique_ptr<Vertex3>*> _startFrontVertexes;

    // Later replace ShellVertex3::findAttachedVertex with that function
    unique_ptr<Vertex3>* findAttachedVertex(const ShellVertex3* shellVertex);
    ShellEdge3* findShellEdge(const ShellVertex3* v0, const ShellVertex3* v1) const;
    unique_ptr<Edge3>* findStartFrontEdge(const unique_ptr<Vertex3>* v0, const unique_ptr<Vertex3>* v1) const;

    template <class T>
    void removePtrsToNullptr(vector<unique_ptr<T>*>& vec);

    void removePtrsToNullptrFromVectors();
    void triangulateShell();
    void setLinksWithShell();
    void startFrontDelaunayPostprocessing();

    void outputDataObj(string filename) const;
    void outputDataLSDynaKeyword_PART(std::ofstream& file, int polycrystalId = 1) const;
    void outputDataLSDynaKeyword_NODE(std::ofstream& file) const;
    void outputDataLSDynaKeyword_ELEMENT_SOLID(std::ofstream& file, int polycrystalId = 1) const;
    void outputDataLSDynaKeyword(string filename, int polycrystalId = 1) const;

public:
    void generateMeshNoStructGen(const double preferredLength);
    void generateMeshNoStructGen(string filename, const double preferredLength);
    void generateMeshNoStructGen(const PolyStruct* crysesShell, const double preferredLength);
    PolyMesh* structurizeMesh();
    PolyMesh* generateMesh(const double preferredLength);
    PolyMesh* generateMesh(string filename, const double preferredLength);
    PolyMesh* generateMesh(const PolyStruct* crysesShell, const double preferredLength);
    PolyMesh* getLastMesh();

    void inputData(string filename);
    void inputData(const PolyStruct* crysesShell);
    void outputData(string filename = "polycr.obj", FileType filetype = OBJ, int polycrystalId = 1) const;

    Polycrystal3();
    Polycrystal3(string filename);
    Polycrystal3(const PolyStruct* crysesShell);
    ~Polycrystal3();
};