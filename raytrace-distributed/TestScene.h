#pragma once
#include "BakedRenderScene.h"
class TestScene :
	public BakedRenderScene
{
private:
	BRDF* brdf;
	BRDF* brdf2;
	IntersectResult intersectBall(const ray r) const;
	IntersectResult intersectPlane(const ray r) const;
	IntersectResult intersectPlane2(const ray r) const;
public:
	TestScene();
	virtual IntersectResult intersect(ray r) const;
	virtual LightRay light(Path& path) const;
	~TestScene();
};

