#pragma once
#include "defs.h"
#include "BakedRenderScene.h"
#include <vector>
#include <opennurbs.h>
#include "BVHTree.h"
class NurbsModel;
//A baked scene that can be loaded from some terrible Rhino file.
class RhinoScene :public BakedRenderScene
{
public:
	RhinoScene();
	int load(const std::string& file);
	virtual ~RhinoScene();
	camera getReferenceCamera() const;
	virtual IntersectResult intersect(ray r) const;
	virtual LightRay light(Path& path) const;
	real vp_width = 1;
private:
	BRDF* _brdf, *_brdf2;
	camera ref;
	struct PointLight {
		vec3 pos;
		vec3 color;
		real strength;
	};
	real total_light = 0;
	std::vector<PointLight> lights;
	void addLight(vec3 pos, vec3 color, real strength);
	ONX_Model model;

	NurbsModel* m;

	BVHTree bvh;

	void _addBoundingBox(vec3 a, vec3 b, vec3 col);
	struct _plane {
		vec3 a;
		vec3 b;
		int lock;
		vec3 color;
	};
	std::vector<_plane> _planes;
	void _addplane(vec3 a, vec3 b, vec3 col, int lock);
	IntersectResult _planehit(ray r) const;
};

