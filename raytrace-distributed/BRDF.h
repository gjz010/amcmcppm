#pragma once
#include "defs.h"
#include "Path.h"
#include <vector>
struct Sampler {
	vec3 indir;
	vec3 normal;
	real current_refraction_index;
	color flux;
	real u, v;
};
struct MatPoint {
	vec3 outdir;
	color outflux;
	real new_refraction_index;
	bool splattable;
};

class BRDF
{
public:
	virtual MatPoint transmit(const Sampler& s, Path& p) const=0;

};

class Diffuse :public BRDF{
public:
	Diffuse(vec3 color);
	virtual MatPoint transmit(const Sampler& s, Path& p) const;
private:
	vec3 wgt;

};
class TexturedDiffuse : public BRDF {
public:
	TexturedDiffuse(const wchar_t* path);
	virtual MatPoint transmit(const Sampler& s, Path& p) const;
private:
	std::vector<vec3> tex;
	int width;
	int height;
};
//Assume that normal points outward
class Refraction : public BRDF {
public:
	Refraction(vec3 weight, real inner, real outer);
	virtual MatPoint transmit(const Sampler& s, Path& p) const;
private:
	vec3 weight;
	real inner, outer;
};

class MultiBRDF : public BRDF{
public:
	MultiBRDF();
	~MultiBRDF();
	void add(BRDF* brdf, real weight);
	virtual MatPoint transmit(const Sampler& s, Path& p) const;
private:
	std::vector<real> lottery;
	std::vector<BRDF*> brdfs;
	real total = 0;
	int size = 0;
};

class Reflection : public BRDF {
public:
	Reflection(vec3 weight);
	virtual MatPoint transmit(const Sampler& s, Path& p) const;
private:
	vec3 weight;

};