#include "BakedRenderScene.h"
#include <cstring>


BakedRenderScene::BakedRenderScene()
{
}

BakedRenderScene::~BakedRenderScene()
{
}

bool operator<(const IntersectResult & r1, const IntersectResult & r2)
{
	if(r1.success && r2.success) return r1.d < r2.d;
	else if (r1.success && !r2.success) return true;
	else return false;
}
