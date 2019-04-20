// Copyright © 2018-2019 Tokarev Artem Alekseevich. All rights reserved.
// Licensed under the MIT License.

#pragma once
#include <string>
#include <fstream>
#include <list>
#include <vector>
#include <memory>
#include "spatial-objs/polyhedron.h"
#include "spatial-objs/shell/shell-face.h"
#include "spatial-objs/shell/shell-edge.h"
#include "spatial-objs/shell/shell-vertex.h"
#include "spatial-objs/tetr.h"
#include "spatial-objs/face.h"
#include "spatial-objs/edge.h"
#include "spatial-objs/vert.h"
#include "data-structures/polymesh.h"
#include "data-structures/polyshell.h"
#include "helpers/logger.h"
#include "real-type.h"
#include "pmg-settings.h"

#include "definitions.h"


namespace pmg {

class PolyhedralSet
{
public:
    struct Log
    {
        real_t minQuality;
        real_t avQuality;
        real_t minMeshAbsGrad;
        real_t avMeshAbsGrad;
        size_t nPolyhs;
        size_t nElems;
        real_t prefLen;
        double shellTrTime;
        double volumeExhTime;
        double meshFileWritingTime;

        void write( std::ostream& stream ) const;
    };

    enum class FileType
    {
        WavefrontObj,
        LsDynaKeyword
    };

    void generateMesh( real_t preferredLength );
    void optimizeMesh( settings::Optimization optSettings = settings::Optimization() );
    const PolyMesh* structurizeMesh();
    const PolyMesh* getLastMesh();

    Log log();
    std::string generateLogFileName() const;

    void input( std::string_view polyStructFileName );
    void input( const psg::PolyShell& polyStruct );
    void output( FileType filetype = FileType::WavefrontObj, std::string_view filename = "_AUTO_", unsigned PolyhedralSetId = 1u );

    PolyhedralSet();
    PolyhedralSet( std::string_view polyStructFileName );
    PolyhedralSet( const psg::PolyShell& polyStruct );
    ~PolyhedralSet();


private:
    Log m_log;
    bool m_isLogged = false;

    real_t m_prefLen;
    PolyMesh* m_lastMesh = nullptr;

    std::vector<Polyhedron*> m_polyhedrons;

    std::vector<shell::Face*> m_shellFaces;
    std::vector<shell::Edge*> m_shellEdges;
    std::vector<shell::Vert*> m_shellVerts;

    shell::Edge* findShellEdge( const shell::Vert* v0, const shell::Vert* v1 ) const;

    void triangulateShell();

    void outputObj( std::string_view filename ) const;
    void outputLSDynaKeyword_PART( std::ofstream& file ) const;
    void outputLSDynaKeyword_NODE( std::ofstream& file ) const;
    void outputLSDynaKeyword_ELEMENT_SOLID( std::ofstream& file, unsigned PolyhedralSetId = 1u ) const;
    void outputLSDynaKeyword( const std::string& filename, unsigned PolyhedralSetId = 1u ) const;

    std::string generateOutputFilename( FileType filetype, std::string_view filename ) const;
};

} // namespace pmg
