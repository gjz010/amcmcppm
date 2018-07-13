#include "TestScene.h"
#include <algorithm>

IntersectResult TestScene::intersectBall(const ray r) const
{
	IntersectResult result;
	real R = 3.0;
	vec3 center = { 1,R,0 };
	vec3 line = (center - r.pos);
	vec3 dir = normalize(r.dir);
	real costheta = dir*normalize(line);
	//if (r.pos[1] == 2) debug("%lf\n", costheta);
	if (costheta <= 0) return FAIL;
	real sintheta = sqrt(1 - costheta*costheta);
	if (abs(1 - costheta) < 1e-6) sintheta = 0;
	real d = norm2(line)*sintheta;
	//if (r.pos[1] == 2) debug("%lf\n", d);
	if (d >= R) return FAIL;
	real dist = sqrt(R*R - d*d);
	vec3 intersect = r.pos + (line*dir - dist)*dir;
	if(norm2(intersect-r.pos)<1e-5) intersect= r.pos + (2*dist)*dir;
	result.intersect = intersect;
	result.brdf = brdf2;
	result.success = true;
	//debug("[Ball] %lf %lf %lf\n", intersect[0], intersect[1], intersect[2]);
	result.normal = normalize(intersect - center);
	vec3 ball = normalize(intersect - center);
	real phi = atan(ball[0] / ball[2]);
	real theta = acos(ball[1]);
	result.u = phi / (PI)+0.5;
	result.v = 1 - theta / PI;
	result.d = norm2(r.pos - intersect);
	//if (verbose) exit(0);
	
	return result;
}

IntersectResult TestScene::intersectPlane(const ray r) const
{
	IntersectResult result;
	//debug("P1 %lf %lf\n", r.dir[1], r.pos[1]);
	//debug("P2 %lf %lf\n", r.dir[1], r.pos[1]);
	vec3 dir = normalize(r.dir);
	
	if (dir[1]  > -1e-5 || r.pos[1]<0) return FAIL;
	real t = -r.pos[1] / dir[1];
	vec3 intersect = { 0,1e-6,0 };
	intersect[0] = r.pos[0] + t*dir[0];
	intersect[2] = r.pos[2] + t*dir[2];
	if (abs(intersect[0]) > 7.5 || abs(intersect[2]) > 7.5) return FAIL;
	result.intersect = intersect;
	result.brdf = brdf;
	result.success = true;
	//debug("DIR2: %lf %lf %lf %lf %lf %lf\n", dir[0], dir[1], dir[2], r.pos[0], r.pos[1], r.pos[2]);
	//debug("ITS: %lf %lf %lf\n", intersect[0], intersect[1], intersect[2]);
	result.normal = vec3({ 0, 1.0, 0 });
	result.d = norm2(r.pos - intersect);
	result.u = (intersect[0]+7.5)/15;
	result.v = (intersect[2]+7.5)/15;
	
	//if (verbose) exit(0);
	return result;
}
IntersectResult TestScene::intersectPlane2(const ray r) const
{
	IntersectResult result;
	//debug("P1 %lf %lf\n", r.dir[1], r.pos[1]);
	real height = 12.0;
	real half_len = 10;
	if (abs(r.dir[1]) < 1e-5) return FAIL;
	if (abs(r.pos[1]-height) < 1e-5) return FAIL;
	//debug("P2 %lf %lf\n", r.dir[1], r.pos[1]);
	if (r.dir[1] * (r.pos[1]-height) > 0) return FAIL;
	real t = -(r.pos[1]-height) / r.dir[1];
	vec3 intersect = { 0,height+(r.pos[1]>0 ? 1e-3 : -1e-3),0 };
	intersect[0] = r.pos[0] + t*r.dir[0];
	intersect[2] = r.pos[2] + t*r.dir[2];
	if (abs(intersect[0]) > half_len || abs(intersect[2]) > half_len) return FAIL;
	result.intersect = intersect;
	result.brdf = brdf;
	result.success = true;
	result.normal = vec3({ 0, (r.pos[1]>height) ? 1.0 : -1.0, 0 });
	result.d = norm2(r.pos - intersect);
	result.u = (intersect[0] /half_len)+0.5;
	result.v = (intersect[2] /half_len)+0.5;

	//if (verbose) exit(0);
	return result;
}

TestScene::TestScene()
{
	brdf2 = new Diffuse(vec3({ 0.63, 0.065, 0.05 }));
	MultiBRDF* mb = new MultiBRDF();
	mb->add(new Reflection(vec3({ 0.9,0.9,0.9 })),0.9);
	mb->add(new Diffuse(vec3({ 0.63, 0.065, 0.05 })), 0.1);
	brdf = mb;
	//brdf2 = new Refraction(vec3({ 0.9,0.9,0.9 }), 1.05, 1.0);
	//brdf = brdf2;
}

IntersectResult TestScene::intersect(ray r) const
{
	//return intersectPlane(r);
	auto r1 = intersectBall(r), r2 = intersectPlane(r);
	return std::min(r1, r2);
	//return intersectPlane(r);
	//if (verbose) debug("%lf %lf %lf %lf %lf %lf\n", r.dir[0], r.dir[1], r.dir[2], r.pos[0], r.pos[1], r.pos[2]);

}

LightRay TestScene::light(Path & path) const
{
	//if (path.next() > 0) {
		LightRay r;
		vec3 x,y,z;
		r.r.pos = { 4,8,0 };
		real theta = path.next()*2*PI;
		real len = path.next() * 7;
		y = normalize(vec3({ len*cos(theta),0,len*sin(theta) }) - r.r.pos);
		r.r.dir = y;
		r.flux = { 100000, 100000, 100000 };
		//debug("[Rad] %lf\n", rdir*down);4
		//r.r.dir = { -1, 0, 0 };
		return r;
	/*}else{
		LightRay r;
		r.r.pos = 10 * sphere(path.next(), path.next());
		r.r.dir = -normalize(r.r.pos);
		r.flux = { 1,1,1 };
		return r;
	}*/
}


TestScene::~TestScene()
{
	delete brdf;
	delete brdf2;
}
