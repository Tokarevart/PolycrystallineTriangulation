// Copyright © 2018-2019 Tokarev Artem Alekseevich. All rights reserved.
// Licensed under the MIT License.

#include "spatial-objs/polyhedron-front/polyhedron-front-edge.h"
#include <cmath>
#include "helpers/spatial-algs/spatial-algs.h"


#define PI static_cast<real_t>(M_PI)

#define K_ALPHA static_cast<real_t>(4.0)


using FrSuFace = pmg::front::Face;
using FrSuEdge  = pmg::front::Edge;
using pair_vv = std::pair<pmg::Vert*, pmg::Vert*>;
using pair_ff = std::pair<FrSuFace*, FrSuFace*>;




void FrSuEdge::refreshAngleData()
{
    m_needExCosProcessing      = true;
    m_needComplexityProcessing = true;
}


real_t FrSuEdge::complexity()
{
    if (!m_needComplexityProcessing)
        return m_complexity;

    return computeComplexity();
}


real_t FrSuEdge::angleExCos()
{
    if (!m_needExCosProcessing)
        return m_exCos;

    return computeAngleExCos();
}


real_t FrSuEdge::computeComplexity()
{
    m_needComplexityProcessing = false;
    return m_complexity = m_relatedPolyhedron->preferredLength() / edge->magnitude() + K_ALPHA * PI / computeAngle();
}


real_t FrSuEdge::computeAngleExCos()
{
    auto adj_faces = getAdjFFaces();

    real_t normals_cos = Vec::dot(std::get<0>(adj_faces)->normal, std::get<1>(adj_faces)->normal);

    m_needExCosProcessing = false;
    return m_exCos = spatalgs::cpaTime(
            std::get<0>(adj_faces)->computeCenter(), std::get<0>(adj_faces)->normal,
            std::get<1>(adj_faces)->computeCenter(), std::get<1>(adj_faces)->normal) < static_cast<real_t>(1e-6) ?
        static_cast<real_t>(-2.0) + normals_cos :
        -normals_cos;
}


real_t FrSuEdge::computeAngle()
{
    return angleExCos() < static_cast<real_t>(-1.0) ?
        acosReal(m_exCos + static_cast<real_t>(2.0)) + PI :
        acosReal(m_exCos);
}




pair_vv FrSuEdge::findOppVerts()
{
    auto adj_faces = getAdjFFaces();

    return { std::get<0>(adj_faces)->face->findVertNot(edge),
             std::get<1>(adj_faces)->face->findVertNot(edge) };
}


FrSuEdge* FrSuEdge::findOppEdge()
{
    auto opp_verts = findOppVerts();
    std::vector<FrSuEdge*> opp_fedges;
    for (auto& f_edge : m_relatedPolyhedron->frontEdges())
    {
        if (((f_edge->edge->verts[0] == std::get<0>(opp_verts) &&
              f_edge->edge->verts[1] == std::get<1>(opp_verts)) ||
             (f_edge->edge->verts[1] == std::get<0>(opp_verts) &&
              f_edge->edge->verts[0] == std::get<1>(opp_verts))))
        {
            opp_fedges.push_back(f_edge);
        }
    }

    if (opp_fedges.size() == 1)
        return opp_fedges.front();

    std::vector<pair_ff> adj_ffaces_vec;
    adj_ffaces_vec.reserve(opp_fedges.size());
    for (auto& fedge : opp_fedges)
        adj_ffaces_vec.push_back(fedge->getAdjFFaces());

    Vec main_vert_pos = edge->verts[0]->pos();
    Vec main_vert_proj = spatalgs::project(main_vert_pos, opp_verts.first->pos(), opp_verts.second->pos());
    Vec main_vec = main_vert_pos - main_vert_proj;

    FrSuEdge* max_cos_fedge = nullptr;
    real_t max_cos = -1.0;
    for (size_t i = 0; i < adj_ffaces_vec.size(); i++)
    {
        Vec adj_opp_pos0 = adj_ffaces_vec[i].first->face->findVertNot(opp_fedges.front()->edge)->pos();
        Vec adj_opp_pos1 = adj_ffaces_vec[i].second->face->findVertNot(opp_fedges.front()->edge)->pos();
        Vec adj_opp_proj0 = spatalgs::project(adj_opp_pos0, opp_verts.first->pos(), opp_verts.second->pos());
        Vec adj_opp_proj1 = spatalgs::project(adj_opp_pos1, opp_verts.first->pos(), opp_verts.second->pos());
        Vec adj_vec0 = adj_opp_pos0 - adj_opp_proj0;
        Vec adj_vec1 = adj_opp_pos1 - adj_opp_proj1;
        real_t cos0 = Vec::cos(adj_vec0, main_vec);
        real_t cos1 = Vec::cos(adj_vec1, main_vec);
        real_t max_cur_cos = std::max(cos0, cos1);
        if (max_cur_cos > max_cos)
        {
            max_cos = max_cur_cos;
            max_cos_fedge = opp_fedges[i];
        }
    }

    return max_cos_fedge;
}




pair_ff FrSuEdge::getAdjFFaces()
{
    if (!isAdjFacesFull())
        fillAdjFFaces();

    return m_adjFFaces;
}




bool FrSuEdge::addAdjFFace(const FrSuFace* fFace)
{
    if (!std::get<0>(m_adjFFaces))
        std::get<0>(m_adjFFaces)  = const_cast<FrSuFace*>(fFace);
    else if (!std::get<1>(m_adjFFaces))
        std::get<1>(m_adjFFaces) = const_cast<FrSuFace*>(fFace);
    else
        return false;

    return true;
}


bool FrSuEdge::removeAdjFFace(const FrSuFace* fFace)
{
    if (std::get<0>(m_adjFFaces) == const_cast<FrSuFace*>(fFace))
        std::get<0>(m_adjFFaces) = nullptr;
    else if (std::get<1>(m_adjFFaces) == const_cast<FrSuFace*>(fFace))
        std::get<1>(m_adjFFaces) = nullptr;
    else
        return false;

    return true;
}


bool FrSuEdge::adjFFacesContains(const FrSuFace* fFace) const
{
    return std::get<0>(m_adjFFaces) == const_cast<FrSuFace*>(fFace) ||
           std::get<1>(m_adjFFaces) == const_cast<FrSuFace*>(fFace);
}


void FrSuEdge::fillAdjFFaces(const FrSuFace* fFace0, const FrSuFace* fFace1)
{
    m_adjFFaces.first  = const_cast<FrSuFace*>(fFace0);
    m_adjFFaces.second = const_cast<FrSuFace*>(fFace1);
}




FrSuEdge::Edge(const Polyhedron* relatedPolyhedron, const pmg::Edge* edge)
    : edge(const_cast<pmg::Edge*>(edge)), m_relatedPolyhedron(const_cast<Polyhedron*>(relatedPolyhedron)) {}




bool FrSuEdge::isAdjFacesFull()
{
    return std::get<0>(m_adjFFaces) && std::get<1>(m_adjFFaces);
}


pair_ff FrSuEdge::fillAdjFFaces()
{
    for (auto& fface : m_relatedPolyhedron->frontFaces())
    {
        if (fface->contains(this))
        {
            if (adjFFacesContains(fface))
                continue;

            addAdjFFace(fface);
            if (isAdjFacesFull())
                break;
        }
    }

    if (!isAdjFacesFull())
        throw std::logic_error("pmg::front::Edge::fillAdjFFaces didn't find 2 adjacent front Faces.");

    return m_adjFFaces;
}