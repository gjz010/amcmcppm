#include "BRDF.h"

#include <vector>

#include "ImageWriter.h"

MultiBRDF::MultiBRDF()
{
	//sizeof(Internal);
	total = 0;
	size = 0;

}

void MultiBRDF::add(BRDF * brdf, real weight)
{
	total += weight;
	lottery.push_back(weight);
	brdfs.push_back(brdf);
	size++;
}

MultiBRDF::~MultiBRDF() {
	//delete data;
}
MatPoint MultiBRDF::transmit(const Sampler& s, Path& p) const {
	double idx = p.nextUniform()*total;
	//debug("%lf %d %d\n", total, brdfs.size(), lottery.size());
	for (int i = 0; i < brdfs.size(); i++) {
		idx -= lottery[i];
		if (idx <= 0) return brdfs[i]->transmit(s, p);
	}
	return brdfs[0]->transmit(s, p);
};

Diffuse::Diffuse(vec3 color)
{
	wgt = color;
}
bool ingrid(real u, real v) {
	//debug("%lf %lf\n", u, v);
	int a = floor(u * 10);
	int b = floor(v * 10);
	return (((a & 1) == 0) + ((b & 1) == 0))&1;
}
MatPoint Diffuse::transmit(const Sampler & s, Path & p) const
{
	MatPoint mat;
	mat.splattable = true;
	mat.new_refraction_index = s.current_refraction_index;
	real theta = p.next();
	real phi = p.next();
	vec3 dir = sphere(theta, phi);
	if (dir*s.normal < 0) dir = -dir;
	mat.outdir = dir;
	//if (ingrid(s.u, s.v)) { 
		mat.outflux = (wgt^s.flux);
		//debug("INGrid!\n");
	//}
	//else mat.outflux = (vec3({ 0.725, 0.71, 0.68 }) ^ s.flux);
	return mat;
}

Refraction::Refraction(vec3 weight, real inner, real outer) :weight(weight), inner(inner), outer(outer)
{

}

MatPoint Refraction::transmit(const Sampler & s, Path & p) const
{

	MatPoint m;
	m.splattable = false;
	vec3 I = normalize(s.indir);
	vec3 N = normalize(s.normal);
	//debug("%lf %lf %lf %lf %lf %lf\n", s.indir[0], s.indir[1], s.indir[2],s.normal[0], s.normal[1], s.normal[2]);
	//if (s.indir * s.normal < 0) debug("RayOut!\n"); else debug("RayIn!\n");
	//if (I * N< 0) debug("RayOut2!\n");
	double cosi = I*N;
	if (cosi < -1) cosi = -1;
	else if (cosi > 1) cosi = 1;
	double etai = s.current_refraction_index, etat = this->inner;
	vec3 n = N;
	if (cosi < 0) { cosi = -cosi; }
	else { etai = this->outer; etat = s.current_refraction_index; n = -N; }
	double eta = etai / etat;
	double k = 1 - eta * eta * (1 - cosi * cosi);
	if (k < 0) {
		m.outflux = { 0,0,0 };
		m.new_refraction_index = 0;
		m.outdir = { 1,0,0 };
		debug("No refraction!");
	}
	else {
		//if(cosi<0) debug("Refraction %lf %lf %lf %s\n", N[0], N[1], N[2],(cosi<0?"IN":"OUT"));
		vec3 out = normalize(eta * I + (eta * cosi - sqrt(k)) * n);
		//debug("Refraction! %lf %lf %lf %lf %lf %lf %lf %lf\n", etai, etat, I[0], I[1], I[2], out[0], out[1], out[2]);
		m.outdir = out;
		//m.outdir = out;
		m.outflux = weight^s.flux;
		m.new_refraction_index = etat;
	}
	return m;
	
}

Reflection::Reflection(vec3 weight) :weight(weight)
{
	
}

MatPoint Reflection::transmit(const Sampler & s, Path & p) const
{
	
	const vec3& I = normalize(s.indir);
	const vec3& N = normalize(s.normal);
	vec3 out = normalize(I - 2 * (I*N)*N);
	MatPoint m;
	m.splattable = false;
	m.outdir = out;
	m.outflux = s.flux^weight;
	//debug("%lf %lf\n", m.outdir*s.indir, (m.outdir+s.indir)*N);
	//debug("%lf %lf %lf\n", m.outflux[0], m.outflux[1], m.outflux[2]);
	m.new_refraction_index = s.current_refraction_index;
	return m;
}

TexturedDiffuse::TexturedDiffuse(const wchar_t * path)
{
	ImageWriter::read(path, tex, width, height);
}

MatPoint TexturedDiffuse::transmit(const Sampler & s, Path & p) const
{
	MatPoint mat;
	mat.splattable = true;
	mat.new_refraction_index = s.current_refraction_index;
	real theta = p.next();
	real phi = p.next();
	vec3 dir = sphere(theta, phi);
	if (dir*s.normal < 0) dir = -dir;
	mat.outdir = dir;
	//if (ingrid(s.u, s.v)) {
	//printf("%lf %lf\n", s.u, s.v);
	int i = s.u*width;
	int j = s.v*height;
	if (i < 0)i = 0;
	if (i >= width) i = width - 1;
	if (j < 0) j = 0;
	if (j >= height) j = height - 1;
	vec3 wgt = tex[j*width + i];
	mat.outflux = (wgt^s.flux);
	//debug("INGrid!\n");
	//}
	//else mat.outflux = (vec3({ 0.725, 0.71, 0.68 }) ^ s.flux);
	return mat;
}
