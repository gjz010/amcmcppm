#pragma once
#include "defs.h"
#include <vector>
#include "BVHTree.h"
#include "BRDF.h"
class MeshModel
{
public:
	MeshModel();
	void initialize(const void* mesh);
	void visit(BVHTree& tree);
	~MeshModel();
	const BRDF* brdf;
	struct triangle {
		vec3 verts[3];
		vec3 norms[3];
		vec2 texs[3];
		triangle();
		void vert(int c, real x, real y, real z);
		void norm(int c, real x, real y, real z);
		void tex(int c, real x, real y);
		
	};
	std::vector<triangle> mesh;
	
	struct MeshTrigger :Trigger{
		MeshModel* model;
		int index;
		IntersectResult intersect(const ray r) const;
	};
	std::vector<MeshTrigger> triggers;
};

