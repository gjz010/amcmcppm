#pragma once
#include "defs.h"
#include "BakedRenderScene.h"
#include <vector>
#include "BVHTree.h"
class NurbsModel
{

public:
	NurbsModel();
	~NurbsModel();
	void initialize(const void* nurbssrf);

	IntersectResult intersect(ray r, real startu, real startv);
	BRDF* brdf;
	//std::vector<BoundingBox> m_BoundingBoxes;
	//std::vector<real> us;
	//std::vector<real> vs;
	void visit(BVHTree& tree);
	struct NurbsTrigger :public Trigger{
		NurbsModel* model;
		IntersectResult intersect(const ray r) const;
		real u, v;
	};
	std::vector<NurbsTrigger> m_Quads;

private:
	struct InternalTrigger : public Trigger {
		IntersectResult intersect(const ray r) const;
		real u, v;
	};
	std::vector<InternalTrigger> itgs;
	BVHTree blocks;
	void initialize(int i, int j, int iorder, int jorder, std::vector<vec4> cv, std::vector<real> knoti, std::vector<real> knotj);
	void flatten();
	bool getuv(ray r, real& u, real& v) const;
	bool _enabled;
	int i, j, iorder, jorder;
	const void* model;

	std::vector<vec4> cv;
	std::vector<real> knoti, knotj;
};

