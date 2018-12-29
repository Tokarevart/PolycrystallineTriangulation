#pragma once
#include <list>
#include <memory>
#include "ClassDefinitions.h"
#include "ClassInclusions.h"
#include "unique_ptr_helper.h"

using std::unique_ptr;

class Facet3 : public unique_ptr_helper<Facet3>
{
public:
	unique_ptr<Edge3>* edges[3];

	static unique_ptr<Edge3>* intersectAlongAnEdge(const Facet3& facet0, const Facet3& facet1);

	const bool intersectsBy(const Vec3& origin, const Vec3& dir);
	unique_ptr<Vertex3>* findVertexNotIncludedInEdge(const Edge3& edge) const;
	unique_ptr<Edge3>*   findEdgeNotContainingVertex(const Vertex3& vert) const;
	unique_ptr<Edge3>*   findEdge(const Vertex3& vert0, const Vertex3& vert1);
	unique_ptr<Edge3>* shortestEdge();
	unique_ptr<Edge3>* longestEdge();

	const bool contains(const Edge3&   edge) const;
	const bool contains(const Vertex3& vert) const;

	Facet3();
	Facet3(Edge3& edge0,   Edge3& edge1,   Edge3& edge2);
	Facet3(Vertex3& vert0, Vertex3& vert1, Vertex3& vert2);
	~Facet3();
};

class FrontFacet3 : public unique_ptr_helper<FrontFacet3>
{
	Vec3 _normal;
	//Vec3 _center;

public:
	Facet3* facet;

	const Vec3 getNormal();
	const void setNormal(const Vec3& vec);
	const Vec3 computeNormal(const vector<unique_ptr<FrontFacet3>*>& frontFacets);

	//const Vec3 getCenter();
	const Vec3 computeCenter();

	FrontFacet3(Facet3* facet);
	~FrontFacet3();
};