#pragma once
#include "BakedRenderScene.h"
#include <vector>
struct BoundingBox {
	vec3 a, b;
	void update(vec3 p);
	bool intersect(const ray r, double* dists) const;
};
struct Trigger{
	BoundingBox box;
	virtual IntersectResult intersect(const ray r) const = 0;
};
class BVHTree
{
public:
	BVHTree();
	void initialize();
	IntersectResult intersect(const ray r) const;
	std::vector<Trigger*> triggers;
	struct Node {
		BoundingBox box;
		int start, count, right;
	};
	std::vector<Node> nodes;
	~BVHTree();
};

