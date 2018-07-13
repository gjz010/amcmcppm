#include "RaytraceWorker.h"
#include "Path.h"
#include "BakedRenderScene.h"
#include "ImageWriter.h"
RaytraceWorker::RaytraceWorker(const BakedRenderScene & scene, const RaytraceWorkerOptions& options):options(options), splatter(options.viewportWidth, options.viewportHeight)
{
	this->scene = &scene;
	

}
void RaytraceWorker::prepare()
{
	totalImage.resize(this->options.viewportWidth*this->options.viewportHeight, { 0,0,0 });
	uniformCount = 1;
	accepted = 0;
	mutated = 0;
	mutationSize = 1.0;
}
//extern bool aa_started;
void RaytraceWorker::run()
{
	

	//for (int _ = 0; _ < options.totalPhases; _++) {

		//aa_started = false;
		eyePhase();
		
		Path current;
		bool first_ready = false;
		int init_count = 0;
		debug("[Worker] Trying to find a first valid path\n");
		while (!first_ready) {
			current = Path();
			first_ready = trySplat(current);
			totalPhotons++;
			if(first_ready) splatPhotons++;
		}

		debug("[Worker] Start Ray Phase\n");
		//aa_started = true;
		for (int i = 0; i < options.totalPhotons; i++) {
			
			real ratio = 0;
			if (totalPhotons > 0) ratio = ((real)splatPhotons / (real)totalPhotons);
			if (i % 10000 == 9999) debug("Ray Phase %d/%d ratio=%lf\n", i, options.totalPhotons, ratio);
			//debug("%lf\n", ratio);
			//debug("[Worker] Ray Phase %d\n", i);
			Path uniformPath;
			if (trySplat(uniformPath, ratio)) {
				current = uniformPath;
				current.reset();
				uniformCount++;
				totalPhotons++;
				splatPhotons++;
			}
			else {
				totalPhotons++;
				uniformPath.reset();
				//uniformPath = Path();
				uniformPath.mutate(mutationSize);
				
				mutated++;
				if (trySplat(uniformPath, ratio)) {
					current = uniformPath;
					current.reset();
					accepted++;
					real R = ((real)accepted) / mutated;
					mutationSize += (R - 0.234) / mutated;
					if (mutationSize < 1e-3) mutationSize = 1e-3;
				}
				else {
					current.reset();
					trySplat(current, ratio);
				}
			}

		}
		

		
		
	//}
	totalImage = splatter.finalize();
	debug("Saving...");
	/*
	for (int i = 0; i < this->options.viewportWidth*this->options.viewportHeight; i++) {
		totalImage[i] = totalImage[i] / options.totalPhases;
	}*/
	
}

bool RaytraceWorker::trySplat(Path & path, real ratio)
{
	path.reset();
	auto lightray = scene->light(path);
	return raytrace(lightray.r, path, lightray.flux, nullptr, ratio);
}
bool verbose = false;
void RaytraceWorker::eyePhase()
{
	debug("[Worker] Start Eye Phase\n");
	std::vector<EyePoint> &eps = eyelist;
	bool first_table = !initialized_eyelist;
	if (!initialized_eyelist) {
		eps.resize(options.viewportWidth*options.viewportHeight);
		initialized_eyelist = true;
	}
	//vec3 testpoint;
	int total_index = 0;
	for (int j = 0; j < options.viewportHeight; j++) {
		debug("[Worker.eyePhase] Line %d\n", j);
		for (int i = 0; i < options.viewportWidth; i++, total_index++) {
			//debug("[Worker.rayPhase] Column  %d\n",i);
			Path gen;
			const camera& cam = options.cam;
			real px = ((real(i) + gen.next()) / options.viewportWidth - 0.5);
			real py = ((real(j) + gen.next()) / options.viewportHeight - 0.5) / (((real)options.viewportWidth) / ((real)(options.viewportHeight)));
			//if(i==400) debug("%d (%lf %lf %lf) (%lf %lf %lf) %lf (%lf %lf %lf) (%lf %lf %lf)\n", j, cam.pos[0], cam.pos[1], cam.pos[2],cam.dir[0], cam.dir[1], cam.dir[2], cam.len, cam.cx[0], cam.cx[1], cam.cx[2], cam.cy[0], cam.cy[1], cam.cy[2]);
			//px in strict -0.5 to 0.5 .
			vec3 dir = normalize(cam.dir*cam.len + cam.cx*px*options.cameraWidth + cam.cy*py*options.cameraWidth);
			ray r = { cam.pos, dir };
			//HitPoint hp;
			//if (i == 400 && j==300) debug("T1 %lf %lf %lf %lf %lf %lf\n",r.dir[0],r.dir[1],r.dir[2],r.pos[0],r.pos[1], r.pos[2]);
			//if (i == 400 && j == 300) verbose = true;
			HitPoint hp;
			bool success = raytrace(r, gen, { 1,1,1 }, &hp, 1);
			EyePoint& ep = eps[total_index];
			ep.enabled = false;
			//if (i == 400 && j==300) debug("T2\n");
			if (success) {
				ep.enabled = true;
				ep.x = i; ep.y = j;
				ep.normal = hp.normal; ep.loc = hp.loc; ep.weight = hp.flux;
			}
			//debug("[Worker.eyePhase] %d %d hit\n", i, j);
			//for (auto& hp : hps) {

			//}
		}
	}

	//exit(0);
	if (first_table) {
		splatter = SplatTable(this->options.viewportWidth, this->options.viewportHeight);
		splatter.initializeTable(eps);
	}
	else {
		splatter.reconstructOctree();
	}
	/*
	auto leaf = splatter.octree.findLeaf(testpoint);
	for (int i : leaf->points) {
		auto& tp = eps[i];
		debug("%d %d %lf %lf %lf\n", tp.x, tp.y, tp.loc[0], tp.loc[1], tp.loc[2]);
	}*/
}



bool RaytraceWorker::raytrace(ray r, Path & path, color flux, HitPoint* eye, real ratio)
{
	real refraction_index = 1.0;
	bool visible = false;
	int traces = 10;
	while (traces--) {
		auto intersection = scene->intersect(r);
		if (!eye) {
			//debug("%lf %lf %lf %lf %lf %lf %lf\n", VEC3(r.pos), VEC3(intersection.intersect), intersection.d);
		}
		if (!intersection.success) {
			//if (!eye) debug("FlyOut\n");
			break;
		}
		else {
			//if (!eye) debug("Success\n");
		}
		auto brdf = intersection.brdf;
		
		Sampler rayin;
		rayin.current_refraction_index = refraction_index;
		rayin.normal = intersection.normal;
		rayin.flux = flux;//*exp(-0.9*intersection.d);
		rayin.indir = r.dir;
		rayin.u = intersection.u;
		rayin.v = intersection.v;
		MatPoint rayout = brdf->transmit(rayin, path);
		flux = rayout.outflux;
		//if (normInfinity(flux) < 1e-4) break;
		r = { intersection.intersect, rayout.outdir };
		vec3 real_flux = ratio*flux;
		//if(ratio>0) debug("%lf %lf %lf\n", real_flux[0], real_flux[1], real_flux[2]);
		HitPoint hp= { r.pos, intersection.normal,  ratio*flux};

		if (!eye) {
			if (rayout.splattable) {
				if (splat(hp)) visible = true;
			}
		}
		else {
			if (rayout.splattable) {
				*eye=(hp);
				visible = true;
				break;
			}
		}
	}
	return visible;
}

//Return: has affected any visible point.
bool RaytraceWorker::splat(const HitPoint & hp)
{
	return splatter.splat(hp);
}
