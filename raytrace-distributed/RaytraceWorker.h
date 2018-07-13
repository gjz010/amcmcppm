#include "defs.h"
#include "BakedRenderScene.h"
#include "SplatTable.h"
#include "vector"
struct RaytraceWorkerOptions {
	int viewportWidth;
	int viewportHeight;
	int totalPhotons;
	int totalPhases;
	camera cam;
	real cameraWidth;
};

class RaytraceWorker{
public:
	
	RaytraceWorker(const BakedRenderScene& scene, const RaytraceWorkerOptions& options);
	void prepare();
	void run();
	std::vector<vec3> totalImage;
	int photonCounter = 0;
	int totalPhotons = 0;
	int splatPhotons = 0;


	int uniformCount = 1;
	int accepted = 0;
	int mutated = 0;
	real mutationSize = 1.0;
	std::vector<EyePoint> eyelist;
private:
	const BakedRenderScene* scene;
	SplatTable splatter;
	bool trySplat(Path& path, real ratio = 0);
	void eyePhase(); //This is a bit like SPPM, since every worker has different photon maps.
	bool raytrace(ray r, Path& path, color c, HitPoint* eye, real ratio = 0);
	bool splat(const HitPoint& hp);
	const RaytraceWorkerOptions options;
	
	bool initialized_eyelist = false;
};
