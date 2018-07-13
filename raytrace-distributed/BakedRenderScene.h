/*
* Everything that a raytrace worker required to perform a raytrace.
* RenderScene is the form generated using scene file, while BakedRenderScene is the pre-processed form of RenderScene.
* Raytracing is done using BakedRenderScene.
* Note that BakedRenderScene should be read-only to workers. Moreover, BakedRenderScene should be a self-contained object so that it can be easily serialized and carried to other machines.
*/
#pragma once
#include "defs.h"
#include "Path.h"
#include "Buffer.h"
#include "BRDF.h"
struct IntersectResult {
	bool success;
	const BRDF* brdf;
	vec3 intersect;
	vec3 normal;
	real u, v;
	real d;
};
const IntersectResult FAIL = { false };

bool operator<(const IntersectResult& r1, const IntersectResult& r2);
struct LightRay {
	ray r;
	color flux;

};
class BakedRenderScene {
public:
	BakedRenderScene();
	virtual ~BakedRenderScene();
	
	
	virtual IntersectResult intersect(ray r) const=0;
	virtual LightRay light(Path& path) const=0;


};
