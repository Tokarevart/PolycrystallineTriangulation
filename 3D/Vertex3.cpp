#include "Vertex3.h"


void Vertex3::setPosition(const Vec3 &newPos)
{
	*_position = newPos;
}

void Vertex3::setPosition(const double &coor0, const double &coor1, const double &coor2)
{
	(*_position)[0] = coor0;
	(*_position)[1] = coor1;
	(*_position)[2] = coor2;
}

const Vec3& Vertex3::getPosition() const
{
	return *_position;
}

//void Vertex3::DestroyIfNoLinks()
//{
//	if (inclInEdges.empty())
//	{
//		delete _uniquePtr->release();
//	}
//}

double& Vertex3::operator[](const int &axisIndex)
{
	return (*_position)[axisIndex];
}

Vec3 Vertex3::operator-(const Vertex3 &right) const
{
	return *_position - *right._position;
}

Vec3 Vertex3::operator-(const ShellVertex3 &right) const
{
	return *_position - right.getPosition();
}

Vertex3& Vertex3::operator+=(const Vec3 &right)
{
	if (belongsToShellVertex)
	{
		return *this;
	}
	else if (belongsToShellEdge)
	{
		(*_position) += Vec3(right).project(*belongsToShellEdge->vertexes[0] - *belongsToShellEdge->vertexes[1]);
		//(*_position) += right;
		return *this;
	}
	else if (belongsToShellFacet)
	{
		(*_position) += Vec3(right).project(
			*belongsToShellFacet->edges[0]->vertexes[1] - *belongsToShellFacet->edges[0]->vertexes[0],
			*belongsToShellFacet->edges[1]->vertexes[1] - *belongsToShellFacet->edges[1]->vertexes[0]);
		//(*_position) += right;
		return *this;
	}
	else
	{
		//(*_position) += right;

		return *this;
	}
}

Vertex3& Vertex3::operator-=(const Vec3& right)
{
	if (belongsToShellVertex)
	{
		return *this;
	}
	else if (belongsToShellEdge)
	{
		(*_position) -= Vec3(right).project(*belongsToShellEdge->vertexes[0] - *belongsToShellEdge->vertexes[1]);
		return *this;
	}
	else
	{
		(*_position) -= right;
		return *this;
	}
}

Vertex3::Vertex3() : unique_ptr_helper<Vertex3>(this)
{
	_position.reset(new Vec3());
}

Vertex3::Vertex3(const double& coor0, const double& coor1, const double& coor2) : unique_ptr_helper<Vertex3>(this)
{
	_position.reset(new Vec3(coor0, coor1, coor2));
}

Vertex3::Vertex3(const Vec3& position) : unique_ptr_helper<Vertex3>(this)
{
	_position.reset(new Vec3(position));
}

Vertex3::~Vertex3()
{
	//for (auto &edge : inclInEdges)
	//{
	//	if (*edge)
	//	{
	//		delete edge->release();
	//	}
	//}
}