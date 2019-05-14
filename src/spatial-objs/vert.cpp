// Copyright © 2018-2019 Tokarev Artem Alekseevich. All rights reserved.
// Licensed under the MIT License.

#include "spatial-objs/vert.h"




const vec3& pmg::Vert::pos() const
{
    return *m_pos;
}


void pmg::Vert::setPos(const vec3& newPos)
{
    *m_pos = newPos;
}


void pmg::Vert::setPos(real_t coor0, real_t coor1, real_t coor2)
{
    m_pos->x[0] = coor0;
    m_pos->x[1] = coor1;
    m_pos->x[2] = coor2;
}


real_t& pmg::Vert::operator[](unsigned axis)
{
    return m_pos->x[axis];
}


const real_t& pmg::Vert::operator[](unsigned axis) const
{
    return m_pos->x[axis];
}


vec3 pmg::Vert::operator-(const pmg::Vert& other) const
{
    return *m_pos - *other.m_pos;
}


vec3 pmg::Vert::operator-(const shell::Vert& other) const
{
    return *m_pos - other.pos();
}


pmg::Vert& pmg::Vert::operator+=(const vec3& other)
{
    if (belongsToSVert)
    {
        return *this;
    }
    else if (belongsToSEdge)
    {
        (*m_pos) += vec3(other).project(*belongsToSEdge->verts[0] - *belongsToSEdge->verts[1]);
        return *this;
    }
    else if (belongsToSFace)
    {
        (*m_pos) += vec3(other).project(
            *belongsToSFace->edges[0]->verts[1] - *belongsToSFace->edges[0]->verts[0],
            *belongsToSFace->edges[1]->verts[1] - *belongsToSFace->edges[1]->verts[0]);
        return *this;
    }
    else
    {
        return *this;
    }
}


pmg::Vert& pmg::Vert::operator-=(const vec3& other)
{
    if (belongsToSVert)
    {
        return *this;
    }
    else if (belongsToSEdge)
    {
        (*m_pos) -= vec3(other).project(*belongsToSEdge->verts[0] - *belongsToSEdge->verts[1]);
        return *this;
    }
    else
    {
        (*m_pos) -= other;
        return *this;
    }
}




pmg::Vert::Vert()
{
    m_pos.reset(new vec3());
}


pmg::Vert::Vert(real_t coor0, real_t coor1, real_t coor2)
{
    m_pos.reset(new vec3(coor0, coor1, coor2));
}


pmg::Vert::Vert(const vec3& position)
{
    m_pos.reset(new vec3(position));
}
