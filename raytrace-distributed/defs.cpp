#include "defs.h"
#include "algorithm"
camera::camera(vec3 pos, vec3 dir, vec3 cx, real len)
{
	this->pos = pos;
	this->dir = normalize(dir);
	vec3 rx = cx-(cx*dir)*dir;
	rx = normalize(rx);
	vec3 ry = normalize(cross(rx, dir));
	this->cx = rx;
	this->cy = ry;
	this->len = len;
}

camera::camera()
{
}

vec3 cross(const vec3 & v1, const vec3 & v2)
{
	return { v1[1] * v2[2] - v1[2] * v2[1],v1[2] * v2[0] - v1[0] * v2[2] , v1[0] * v2[1] - v1[1] * v2[0] };
}

vec3 sphere(real theta, real phi)
{
	theta = theta * 2 * PI;
	phi = phi * 2 * PI;
	vec3 dir = { cos(theta)*cos(phi), cos(theta)*sin(phi), sin(theta) };
	return dir;
}

