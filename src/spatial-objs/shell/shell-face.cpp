// Copyright © 2018-2019 Tokarev Artem Alekseevich. All rights reserved.
// Licensed under the MIT License.

#include "spatial-objs/shell/shell-face.h"
#include <algorithm>
#include "helpers/spatial-algs/spatial-algs.h"


using namespace pmg;
namespace sfront = shell::front;
using pair_rr = std::pair<real_t, real_t>;
using pair_ff = std::pair<pmg::Face*, pmg::Face*>;
using pair_ee = std::pair<pmg::Edge*, pmg::Edge*>;


#define DEG_1_IN_RAD static_cast<real_t>(0.0174532925199432957)
#define PI           static_cast<real_t>(M_PI)


#define NINE_DIV_SIXTEEN static_cast<real_t>(0.5625)
#define SIXTEEN_DIV_NINE static_cast<real_t>(1.7777777777777777)
#define SQRT3_2          static_cast<real_t>(0.8660254037844386)

#define NOT_TOO_CLOSE  static_cast<real_t>(2e-1)
#define FROM_VERT_COEF static_cast<real_t>(1e-2)
#define EDGES_INTERS_DIST_COEF static_cast<real_t>(1e-4)

#define K_MAXD static_cast<real_t>(0.3)
#define K_D    static_cast<real_t>(0.5)


template <typename T>
constexpr real_t degToRad(T value)
{
    return value * DEG_1_IN_RAD;
}




real_t shell::Face::preferredLength() const
{
    return m_prefLen;
}


const std::list<pmg::Face*>& shell::Face::innerFaces() const
{
    return m_innerFaces;
}


const std::list<pmg::Edge*>& shell::Face::innerEdges() const
{
    return m_innerEdges;
}


const std::vector<pmg::Vert*>& shell::Face::innerVerts() const
{
    return m_innerVerts;
}


const std::list<sfront::Edge*>& shell::Face::frontEdges() const
{
    return m_frontEdges;
}


shell::Vert* shell::Face::findVertNot(const shell::Edge* edge) const
{
    for (auto& face_edge : edges)
    {
        if (edge != face_edge)
        {
            if (!edge->contains(face_edge->verts[0]))
                return face_edge->verts[0];
            else
                return face_edge->verts[1];
        }
    }

    return nullptr;
}


shell::Edge* shell::Face::findShellEdgeContaining(const pmg::Edge* edge) const
{
    for (auto& sedge : edges)
        if (sedge->contains(edge))
            return sedge;

    return nullptr;
}




void shell::Face::triangulate(real_t preferredLen)
{
    m_prefLen = preferredLen;
    initializeFront();
    computeFrontNormals();
    processAngles();
//    if (globalIntersectionCheck())
//        throw std::logic_error("Intersection error.\npmg::shell::Face::globalIntersectionCheck returned true.");

    smoothMesh(20);
    for (int i = 0; i < 3; i++)
    {
        optimizeMesh();
        smoothMesh(20);
    }
}




bool shell::Face::contains(const shell::Edge* edge) const
{
    for (auto& edge_ : edges)
        if (edge_ == edge)
            return true;

    return false;
}


bool shell::Face::contains(const shell::Vert* vert) const
{
    for (auto& edge : edges)
        if (edge->contains(vert))
            return true;

    return false;
}




shell::Face::Face(
    const shell::Edge* edge0,
    const shell::Edge* edge1,
    const shell::Edge* edge2)
{
    edges[0] = const_cast<shell::Edge*>(edge0);
    edges[1] = const_cast<shell::Edge*>(edge1);
    edges[2] = const_cast<shell::Edge*>(edge2);
}




sfront::Vert* shell::Face::findFrontVert(const pmg::Vert* vert) const
{
    for (auto& fvert : m_frontVerts)
        if (fvert->vert == vert)
            return fvert;

    return nullptr;
}




sfront::Edge* shell::Face::addToFront(const pmg::Edge* edge)
{
    sfront::Edge* new_f_edge = new sfront::Edge(this, edge);
    m_frontEdges.push_back(new_f_edge);
    m_innerEdges.push_back(new_f_edge->edge);
    return new_f_edge;
}


sfront::Vert* shell::Face::addToFront(const pmg::Vert* vert)
{
    sfront::Vert* new_f_vert = new sfront::Vert(this, vert);
    m_frontVerts.push_back(new_f_vert);
    m_innerVerts.push_back(new_f_vert->vert);
    return new_f_vert;
}




void shell::Face::removeFromFront(sfront::Edge* fEdge)
{
    m_frontEdges.erase(std::find(m_frontEdges.begin(), m_frontEdges.end(), fEdge));
    delete fEdge;
}


void shell::Face::removeFromFront(sfront::Vert* fVert)
{
    m_frontVerts.erase(std::find(m_frontVerts.begin(), m_frontVerts.end(), fVert));
    delete fVert;
}




bool shell::Face::anyVertInsidePotentialTriangCheck(sfront::Vert* fVert) const
{
    auto opp_verts = fVert->oppVerts();
    pmg::Vert* tr[3]
    {
        fVert->vert,
        opp_verts.first,
        opp_verts.second
    };

    for (auto& fvert : m_frontVerts)
        if (fvert->vert != tr[0] && fvert->vert != tr[1] && fvert->vert != tr[2] &&
            spatalgs::isPointOnTriangle(fvert->vert->pos(), tr[0]->pos(), tr[1]->pos(), tr[2]->pos()))
            return true;

    return false;
}


bool shell::Face::doesSegmentIntersectsWithFront(const vec3& p0, const vec3& p1) const
{
    for (auto& fedge : m_frontEdges)
        if (spatalgs::segmentsDistance(
                fedge->edge->verts[0]->pos(), fedge->edge->verts[1]->pos(),
                p0, p1) < EDGES_INTERS_DIST_COEF * m_prefLen)
            return true;

    return false;
}




vec3 shell::Face::computeNormalInTriang(sfront::Edge* fEdge, const vec3& oppVertPos)
{
    vec3 p0 = fEdge->edge->verts[0]->pos();
    vec3 p1 = fEdge->edge->verts[1]->pos();
    return (spatalgs::project(oppVertPos, p0, p1) - oppVertPos).normalize();
}




bool shell::Face::tryComputeNewVertPosType2(sfront::Edge* fEdge, vec3& out_pos)
{
    sfront::Vert* main_f_verts[2];
    main_f_verts[0] = findFrontVert(fEdge->edge->verts[0]);
    main_f_verts[1] = findFrontVert(fEdge->edge->verts[1]);

    auto adj_f_edges0 = main_f_verts[0]->findAdjEdges();
    auto adj_f_edges1 = main_f_verts[1]->findAdjEdges();
    auto en0 = std::get<0>(adj_f_edges0) == fEdge ? std::get<1>(adj_f_edges0) : std::get<0>(adj_f_edges0);
    auto en1 = std::get<0>(adj_f_edges1) == fEdge ? std::get<1>(adj_f_edges1) : std::get<0>(adj_f_edges1);

    vec3 vn0_pos = en0->edge->findNot(fEdge->edge)->pos();
    vec3 vn1_pos = en1->edge->findNot(fEdge->edge)->pos();

    vec3 v0_pos = main_f_verts[0]->vert->pos();
    vec3 v1_pos = main_f_verts[1]->vert->pos();

    vec3 e0 = (vn0_pos - 2.0 * v0_pos + main_f_verts[1]->vert->pos()).normalize();
    vec3 e1 = (vn1_pos - 2.0 * v1_pos + v0_pos).normalize();

    vec3 new_pos = spatalgs::linesClosestPoint(v0_pos, v0_pos + e0, v1_pos, v1_pos + e1);

    vec3 v0_to_np = new_pos - v0_pos;
    vec3 v1_to_np = new_pos - v1_pos;
    if (doesSegmentIntersectsWithFront(v0_pos + FROM_VERT_COEF * v0_to_np, new_pos + NOT_TOO_CLOSE * v0_to_np) ||
        doesSegmentIntersectsWithFront(v1_pos + FROM_VERT_COEF * v1_to_np, new_pos + NOT_TOO_CLOSE * v1_to_np))
        return false;

    out_pos = new_pos;
    return true;
}


bool shell::Face::tryComputeNewVertPosType1(sfront::Edge* fEdge, vec3& out_pos, int smallAngleIndex)
{
    auto main_vert = fEdge->edge->verts[smallAngleIndex];
    auto sec_vert  = fEdge->edge->findNot(main_vert);
    auto main_f_vert = findFrontVert(main_vert);

    auto adj_f_edges = main_f_vert->findAdjEdges();
    auto en = std::get<0>(adj_f_edges) == fEdge ? std::get<1>(adj_f_edges) : std::get<0>(adj_f_edges);
    auto vn = en->edge->findNot(main_vert);

    vec3 e = (sec_vert->pos() - static_cast<real_t>(2.0) * main_vert->pos() + vn->pos()).normalize();

    real_t av_magn = static_cast<real_t>(0.5) * (fEdge->edge->magnitude() + en->edge->magnitude());
    real_t raw_deform = K_D * (m_prefLen - av_magn);
    real_t deform = raw_deform < av_magn * K_MAXD ? raw_deform : av_magn * K_MAXD;
    real_t magn_d = av_magn + deform;
    vec3 new_pos = main_vert->pos() + magn_d * e;

//    vec3 new_pos = main_vert->pos() + av_magn * e;

    vec3 v0_to_np = new_pos - main_vert->pos();
    vec3 v1_to_np = new_pos - sec_vert->pos();
    if (doesSegmentIntersectsWithFront(main_vert->pos() + FROM_VERT_COEF * v0_to_np, new_pos + NOT_TOO_CLOSE * v0_to_np) ||
        doesSegmentIntersectsWithFront(sec_vert->pos()  + FROM_VERT_COEF * v1_to_np, new_pos + NOT_TOO_CLOSE * v1_to_np))
        return false;

    out_pos = new_pos;
    return true;
}


bool shell::Face::tryComputeNewVertPosType0(sfront::Edge* fEdge, vec3& out_pos)
{
    real_t magn = fEdge->edge->magnitude();
    real_t raw_deform = K_D * (m_prefLen - magn);
    real_t deform = raw_deform < magn * K_MAXD ? raw_deform : magn * K_MAXD;
    real_t magn_d = magn + deform;
    vec3 new_pos = fEdge->computeCenter() + static_cast<real_t>(0.5) * std::sqrt(static_cast<real_t>(4.0) * magn_d * magn_d - magn * magn) * fEdge->normal;

//    vec3 new_pos = fEdge->computeCenter() + fEdge->normal * m_prefLen * SQRT3_2;

    vec3 v0_pos = fEdge->edge->verts[0]->pos();
    vec3 v1_pos = fEdge->edge->verts[1]->pos();
    vec3 v_to_np0 = (new_pos - v0_pos);
    vec3 v_to_np1 = (new_pos - v1_pos);
    if (doesSegmentIntersectsWithFront(v0_pos + v_to_np0 * FROM_VERT_COEF, new_pos + v_to_np0 * NOT_TOO_CLOSE) ||
        doesSegmentIntersectsWithFront(v1_pos + v_to_np1 * FROM_VERT_COEF, new_pos + v_to_np1 * NOT_TOO_CLOSE))
        return false;

    out_pos = new_pos;
    return true;
}


bool shell::Face::tryComputeNewVertPos(sfront::Edge* fEdge, vec3& out_pos)
{
    real_t angs[2]
    {
        findFrontVert(fEdge->edge->verts[0])->angle(),
        findFrontVert(fEdge->edge->verts[1])->angle()
    };
    int indexes[2];
    int small_angs_num = 0;
    if (angs[0] < degToRad(120) /*cosDeg<120, real_t>*/) indexes[small_angs_num++] = 0;
    if (angs[1] < degToRad(120) /*cosDeg<120, real_t>*/) indexes[small_angs_num++] = 1;

    switch (small_angs_num)
    {
    case 0: return tryComputeNewVertPosType0(fEdge, out_pos);
    case 1: return tryComputeNewVertPosType1(fEdge, out_pos, indexes[0]);
    case 2: return tryComputeNewVertPosType2(fEdge, out_pos);
    }

    return false;
}




pair_rr shell::Face::computeMinMaxEdgesLengths(const vec3& p0, const vec3& p1, const vec3& p2)
{
    auto min_max = computeMinMaxEdgesSqrLengths(p0, p1, p2);
    min_max.first  = std::sqrt(min_max.first);
    min_max.second = std::sqrt(min_max.second);
    return min_max;
}


pair_rr shell::Face::computeMinMaxEdgesSqrLengths(const vec3& p0, const vec3& p1, const vec3& p2)
{
    real_t sqr_magns[3];
    sqr_magns[0] = (p1 - p0).sqrMagnitude();
    sqr_magns[1] = (p2 - p0).sqrMagnitude();
    sqr_magns[2] = (p2 - p1).sqrMagnitude();
    return std::minmax({ sqr_magns[0], sqr_magns[1], sqr_magns[2] });
}


real_t shell::Face::computeTriangSimpleQuality(const vec3& p0, const vec3& p1, const vec3& p2)
{
    auto sqr_min_max = computeMinMaxEdgesSqrLengths(p0, p1, p2);
    return std::sqrt(sqr_min_max.first / sqr_min_max.second);
}


real_t shell::Face::computeTriangSimpleSqrQuality(const vec3& p0, const vec3& p1, const vec3& p2)
{
    auto sqr_min_max = computeMinMaxEdgesSqrLengths(p0, p1, p2);
    return sqr_min_max.first / sqr_min_max.second;
}




sfront::Edge* shell::Face::chooseEdgeForExhaustionWithNewVert(sfront::Vert* fVert)
{
    auto adj_edges = fVert->findAdjEdges();
    return std::get<0>(adj_edges)->edge->sqrMagnitude() < std::get<1>(adj_edges)->edge->sqrMagnitude() ?
                std::get<0>(adj_edges) : std::get<1>(adj_edges);
}


void shell::Face::exhaustWithNewVert(sfront::Edge* fEdge, const vec3& vertPos)
{
    pmg::Vert* main_verts[2] { fEdge->edge->verts[0], fEdge->edge->verts[1] };

    pmg::Vert* new_vert = addToFront(new pmg::Vert(vertPos))->vert;
    auto new_f_edge0 = addToFront(new pmg::Edge(main_verts[0], new_vert));
    auto new_f_edge1 = addToFront(new pmg::Edge(main_verts[1], new_vert));
    new_f_edge0->normal = computeNormalInTriang(new_f_edge0, fEdge->edge->findNot(new_f_edge0->edge)->pos());
    new_f_edge1->normal = computeNormalInTriang(new_f_edge1, fEdge->edge->findNot(new_f_edge1->edge)->pos());

    m_innerFaces.push_back(new pmg::Face(fEdge->edge, new_f_edge0->edge, new_f_edge1->edge));

    findFrontVert(main_verts[0])->refreshAngleData();
    findFrontVert(main_verts[1])->refreshAngleData();

    removeFromFront(fEdge);
}


void shell::Face::exhaustWithoutNewVert(sfront::Vert* fVert)
{
    auto adj_edges = fVert->findAdjEdges();
    std::pair<pmg::Vert*, pmg::Vert*> opp_verts =
    {
        adj_edges.first->edge->findNot(fVert->vert),
        adj_edges.second->edge->findNot(fVert->vert)
    };

    auto new_f_edge = addToFront(new pmg::Edge(opp_verts.first, opp_verts.second));
    new_f_edge->normal = computeNormalInTriang(new_f_edge, fVert->vert->pos());

    m_innerFaces.push_back(new pmg::Face(adj_edges.first->edge, adj_edges.second->edge, new_f_edge->edge));

    findFrontVert(opp_verts.first)->refreshAngleData();
    findFrontVert(opp_verts.second)->refreshAngleData();

    removeFromFront(fVert);
    removeFromFront(adj_edges.first);
    removeFromFront(adj_edges.second);
}




bool shell::Face::tryExhaustWithoutNewVert(sfront::Vert* fVert)
{
    if (anyVertInsidePotentialTriangCheck(fVert))
        return false;

    exhaustWithoutNewVert(fVert);
    return true;
}


bool shell::Face::tryExhaustWithNewVert(sfront::Vert* fVert)
{
    auto exhaust_f_edge = chooseEdgeForExhaustionWithNewVert(fVert);
    vec3 new_vert_pos;
    if (!tryComputeNewVertPos(exhaust_f_edge, new_vert_pos))
        return false;

    exhaustWithNewVert(exhaust_f_edge, new_vert_pos);
    return true;
}




sfront::Vert* shell::Face::currentFrontVert(real_t maxCompl) const
{
    real_t cur_max_compl = 0.0;
    sfront::Vert* cur_max_f_edge = nullptr;
    for (auto& f_vert : m_frontVerts)
    {
        real_t cur_compl = f_vert->complexity();
        if (cur_compl > cur_max_compl &&
            cur_compl < maxCompl)
        {
            cur_max_compl  = cur_compl;
            cur_max_f_edge = f_vert;
        }
    }

    return cur_max_f_edge;
}


bool shell::Face::exhaustWithoutNewVertPriorityPredicate(sfront::Vert* fEdge)
{
    if (fEdge->angle() < degToRad(60))
        return true;

    if (fEdge->angle() > degToRad(80))
        return false;

    auto adj_edges = fEdge->findAdjEdges();
    real_t div = adj_edges.first->edge->sqrMagnitude() / adj_edges.second->edge->sqrMagnitude();
    if (NINE_DIV_SIXTEEN < div && div < SIXTEEN_DIV_NINE)
        return true;

    return false;
}


bool shell::Face::exhaustWithNewVertPriorityPredicate(sfront::Vert* fEdge)
{
    if (fEdge->angle() > degToRad(100))
        return true;

    return false;
}


shell::Face::ExhaustType shell::Face::computeExhaustionTypeQualityPriority(
        sfront::Vert* fVert, sfront::Edge*& out_withNWFrontEdge, vec3*& out_withNWNewVertPos)
{
    if (anyVertInsidePotentialTriangCheck(fVert))
        return ExhaustType::WithNewVert;

    auto opp_verts = fVert->oppVerts();
    real_t without_nv_quality = computeTriangSimpleSqrQuality(
        fVert->vert->pos(),
        std::get<0>(opp_verts)->pos(),
        std::get<1>(opp_verts)->pos());

    sfront::Edge* f_edge = chooseEdgeForExhaustionWithNewVert(fVert);
    vec3 new_vert_pos;
    if (!tryComputeNewVertPos(f_edge, new_vert_pos))
        return ExhaustType::DontExhaust;

    real_t with_nv_quality = computeTriangSimpleSqrQuality(
        f_edge->edge->verts[0]->pos(),
        f_edge->edge->verts[1]->pos(),
        new_vert_pos);

    if (without_nv_quality > with_nv_quality)
        return ExhaustType::WithoutNewVert;

    out_withNWFrontEdge = f_edge;
    out_withNWNewVertPos = new vec3(new_vert_pos);
    return ExhaustType::WithNewVert;
}




void shell::Face::processLastFace()
{
    pmg::Edge* edges[3];
    int i = 0;
    for (auto& f_edge : m_frontEdges)
        edges[i++] = f_edge->edge;

    m_innerFaces.push_back(new pmg::Face(edges[0], edges[1], edges[2]));

    for (auto& f_edge : m_frontEdges)
        delete f_edge;
    m_frontEdges.clear();

    for (auto& f_vert : m_frontVerts)
        delete f_vert;
    m_frontVerts.clear();
}


void shell::Face::processAngles()
{
    if (m_frontEdges.size() < 3)
        throw std::logic_error("Wrong input data.\nError in function: pmg::shell::Face::processAngles");

    if (m_frontEdges.size() == 3)
    {
        processLastFace();
        return;
    }

    real_t max_compl = std::numeric_limits<real_t>::max();
    for (sfront::Vert* cur_f_vert = currentFrontVert(max_compl);; cur_f_vert = currentFrontVert(max_compl))
    {
        if (!cur_f_vert)
            throw std::logic_error("pmg::shell::Face::currentFrontVert returned nullptr");

        if (exhaustWithoutNewVertPriorityPredicate(cur_f_vert))
        {
            if (!tryExhaustWithoutNewVert(cur_f_vert))
            {
                max_compl = cur_f_vert->complexity();
                continue;
            }
        }
        else if (exhaustWithNewVertPriorityPredicate(cur_f_vert))
        {
            if (!tryExhaustWithNewVert(cur_f_vert))
            {
                max_compl = cur_f_vert->complexity();
                continue;
            }
        }
        else
        {
            sfront::Edge* exhaust_from_f_edge = nullptr;
            vec3* new_vert_pos = nullptr;
            switch (computeExhaustionTypeQualityPriority(cur_f_vert, exhaust_from_f_edge, new_vert_pos))
            {
            case ExhaustType::WithoutNewVert:
                exhaustWithoutNewVert(cur_f_vert);
                break;

            case ExhaustType::WithNewVert:
                if (new_vert_pos)
                {
                    exhaustWithNewVert(exhaust_from_f_edge, *new_vert_pos);
                    delete new_vert_pos;
                }
                else
                {
                    if (!tryExhaustWithNewVert(cur_f_vert))
                    {
                        max_compl = cur_f_vert->complexity();
                        continue;
                    }
                }
                break;

            case ExhaustType::DontExhaust:
                max_compl = cur_f_vert->complexity();
                continue;
            }
        }
        max_compl = std::numeric_limits<real_t>::max();

        if (m_frontEdges.size() == 3)
        {
            processLastFace();
            return;
        }
    }
}




void shell::Face::smoothMesh(unsigned nIterations)
{
    for (unsigned i = 0; i < nIterations; i++)
    {
        for (auto& vert : m_innerVerts)
        {
            vec3 shift;
            int delta_shifts_num = 0;
            for (auto& edge : m_innerEdges)
            {
                if (vert == edge->verts[0])
                {
                    shift += *edge->verts[1] - *vert;
                    delta_shifts_num++;
                }
                else if (vert == edge->verts[1])
                {
                    shift += *edge->verts[0] - *vert;
                    delta_shifts_num++;
                }
            }
            shift /= delta_shifts_num;
            vert->setPos(vert->pos() + shift);
        }
    }
}




pair_ff shell::Face::find2AdjFaces(pmg::Edge* edge) const
{
    pair_ff res;
    bool not_found_yet = true;
    for (auto& face : m_innerFaces)
    {
        if (face->contains(edge))
        {
            if (not_found_yet)
            {
                res.first = face;
                not_found_yet = false;
            }
            else
            {
                res.second = face;
                return res;
            }
        }
    }

    throw std::logic_error("pmg::shell::Face::find2AdjFaces didn't find 2 adjacent Faces.");
}


bool shell::Face::flipIfNeeded(pmg::Edge* edge)
{
    pmg::Vert* opp_nodes[2];
    auto around_faces = find2AdjFaces(edge);
    opp_nodes[0] = std::get<0>(around_faces)->findVertNot(edge);
    opp_nodes[1] = std::get<1>(around_faces)->findVertNot(edge);

    real_t alpha = std::acos(vec3::cos(*edge->verts[0] - *opp_nodes[0], *edge->verts[1] - *opp_nodes[0]));
    real_t beta  = std::acos(vec3::cos(*edge->verts[0] - *opp_nodes[1], *edge->verts[1] - *opp_nodes[1]));

    if (alpha + beta <= PI)
        return false;


    auto old_edge   = std::find(m_innerEdges.begin(), m_innerEdges.end(), edge);
    auto old_face0 = std::find(m_innerFaces.begin(), m_innerFaces.end(), std::get<0>(around_faces));
    auto old_face1 = std::find(m_innerFaces.begin(), m_innerFaces.end(), std::get<1>(around_faces));

    pmg::Edge* new_edge = new pmg::Edge(opp_nodes[0], opp_nodes[1]);
    pmg::Face* new_face0 = new pmg::Face(
            std::get<0>(around_faces)->findEdge(opp_nodes[0], edge->verts[0]),
            std::get<1>(around_faces)->findEdge(opp_nodes[1], edge->verts[0]),
            new_edge);
    pmg::Face* new_face1 = new pmg::Face(
            std::get<0>(around_faces)->findEdge(opp_nodes[0], edge->verts[1]),
            std::get<1>(around_faces)->findEdge(opp_nodes[1], edge->verts[1]),
            new_edge);

    delete *old_edge;
    delete *old_face0;
    delete *old_face1;

    *old_edge = new_edge;
    *old_face0 = new_face0;
    *old_face1 = new_face1;

    return true;
}


void shell::Face::optimizeMesh()
{
    for (auto& edge : m_innerEdges)
        flipIfNeeded(edge);
}




void shell::Face::computeFrontNormals() const
{
    for (auto& f_edge : m_frontEdges)
        f_edge->computeNormal();
}


void shell::Face::initializeFront()
{
    std::vector<shell::Vert*> sverts;
    for (auto& this_edge : edges)
        for (auto& svert : this_edge->verts)
            if (std::find(sverts.begin(), sverts.end(), svert) == sverts.end())
                sverts.push_back(svert);

    for (auto& svert : sverts)
        m_frontVerts.push_back(new sfront::Vert(this, svert->attachedVert));
    sverts.clear();

    for (auto& this_edge : edges)
        for (auto& vert : this_edge->innerVerts())
            m_frontVerts.push_back(new sfront::Vert(this, vert));

    for (auto& this_edge : edges)
        for (auto& edge : this_edge->innerEdges())
            m_frontEdges.push_back(new sfront::Edge(this, edge));
}
