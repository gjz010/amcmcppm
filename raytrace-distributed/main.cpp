#include <cstdio>

#include "defs.h"
#include "RaytraceWorker.h"
#include "TestScene.h"
#include "ImageWriter.h"
#include "algorithm"
#include "RhinoScene.h"
#include <opennurbs.h>
#include <omp.h>
int main(){
	
	if (true) {
		std::vector<vec3> totalImage;
		totalImage.resize(1024 * 1024);
		FILE* f = fopen("c:\\KK_Movies\\raytrace_photon_4.dat", "rb");
		fread(totalImage.data(), sizeof(vec3), totalImage.size(), f);
		real sum = 0;
		for (int i = 0; i < 1024 * 1024; i++) {
			sum += norm1(totalImage[i]);
		}
		sum = sum / 1024 / 1024 / 3;
		for (int i = 0; i < 1024 * 1024; i++) {
			vec3 gm;

			totalImage[i] = totalImage[i] / sum*0.5 ;
			gm[0] = pow(totalImage[i][0], 1 / 2.2);
			gm[1] = pow(totalImage[i][1], 1 / 2.2);
			gm[2] = pow(totalImage[i][2], 1 / 2.2);
			totalImage[i] = gm;
		}
		ImageWriter::write("c:\\KK_Movies\\raytrace_photon_4.png", totalImage, 1024, 1024);
		return 0;
	}
	
	ON::Begin();
	//return 0;
	//TestScene s;
	RhinoScene rhino;
	rhino.load("c:\\MinGW\\my_curve.3dm");
	RaytraceWorkerOptions options;
	options.cameraWidth = rhino.vp_width;
	options.viewportWidth = 1024;
	options.viewportHeight = 1024;
	options.totalPhotons = 10000000;
	options.totalPhases = 100;
	options.cameraWidth = rhino.vp_width;
	camera cam(vec3({ 9,12,0 }), vec3({ -3,-4,0 }), vec3({ 0,0,1 }), 1.0);
	options.cam = rhino.getReferenceCamera();//cam;
	const int max_workers = 4;
	RaytraceWorker* worker[max_workers];
	debug("Maximum: %d threads\n", omp_get_max_threads());
	for (int i = 0; i < max_workers; i++) {
		worker[i] = new RaytraceWorker(rhino, options);
		worker[i]->prepare();
	}
#pragma omp parallel
	{
		for (int _ = 0; _ < options.totalPhases; _++) {

#pragma omp for
			for (int i = 0; i < max_workers; i++) {
				if (i == 0) {
					debug("Using %d threads\n", omp_get_num_threads());
				}
				
				worker[i]->run();
			}
#pragma omp barrier
#pragma omp single 
				{
					debug("Summarizing %d...", _);
					std::vector<vec3> totalImage;
					debug("%d %d\n", worker[0]->splatPhotons, worker[0]->totalPhotons);
					totalImage.resize(options.viewportWidth*options.viewportHeight, { 0,0,0 });
					real sum = 0;
					int a = 0, b = 0;
					for (int i = 0; i < max_workers; i++) {
						for (int j = 0; j < options.viewportHeight*options.viewportWidth; j++) {
							totalImage[j] = totalImage[j] + worker[i]->totalImage[j];
							sum += norm1(worker[i]->totalImage[j]);
						}
						a += worker[i]->splatPhotons;
						b += worker[i]->totalPhotons;
					}
					char name[8192];
					sprintf(name, "c:\\MinGW\\raytrace_photon_%d.dat", _);
					FILE* f = fopen(name, "wb");
					fwrite(totalImage.data(), sizeof(vec3), totalImage.size(), f);

					for (int j = 0; j < options.viewportHeight*options.viewportWidth; j++) {
						real r = worker[0]->eyelist[j].rad2;
						fwrite(&r, sizeof(double), 1, f);
						//if (worker[0]->eyelist[j].count > 0) debug("%d %d %d\n", worker[0]->eyelist[j].x, worker[0]->eyelist[j].y,worker[0]->eyelist[j].count);
						fwrite(&(worker[0]->eyelist[j].count), sizeof(int), 1, f);
						fwrite(&(worker[0]->eyelist[j].loc), sizeof(vec3), 1, f);

					}
					fclose(f);
					sum = 3 * options.viewportWidth*options.viewportHeight / sum / 2;
					for (int j = 0; j < options.viewportHeight*options.viewportWidth; j++) {
						//if(norm2(totalImage[j])>1e-4) debug("%lf %lf %lf %lf\n", totalImage[j][0], totalImage[j][1], totalImage[j][2], sum);
						totalImage[j] = totalImage[j] * sum;
					}
					
					sprintf(name, "c:\\MinGW\\raytrace_out_%d.png", _);
					ImageWriter::write(name, totalImage, options.viewportWidth, options.viewportHeight);




				}

			}

		
	}
    return 0;
}
