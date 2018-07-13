#include <vector>
#include <random>
#include "defs.h"
#include "Path.h"

Path::Path()
{
	std::random_device seeder;
	seed = seeder();
	engine = std::mt19937_64(seed);
}

real Path::uniform(){
	std::uniform_real_distribution<real> guniform;

	real n = guniform(engine);
	//vec.push_back(n);
	return n;
}
real Path::normal(){
	std::normal_distribution<real> gnormal;
	real n = gnormal(engine);
	//vec.push_back(n);
	return n;
}
int Path::getSize() const
{
	return vec.size();
}

real Path::nextUniform()
{
	return uniform();
}
real Path::nextNormal()
{
	return normal();
}

real Path::next()
{
	//return uniform();
	
	if (current < getSize()) {
	}
	else {
		vec.push_back(nextUniform());
	}
	return vec[current++];
}

void Path::reset()
{
	current = 0;
}

real Path::operator[](int index)
{
	return vec[index];
}


void Path::mutate(real rad)
{
	for (int i = 0; i < getSize();i++) {
		real value = vec[i];
		real r = (uniform());
		
		if (r < 0.5) {
			r *= 2.0;
			value += pow(r, 1 / rad + 1);
		}
		else {
			r = (r - 0.5) * 2.0;
			value -= pow(r, 1 / rad + 1);
		}
		value -= floor(value);
		
		//debug("%lf %lf\n", *iter, r);
		vec[i] = value;
		
	};
	//debug("%d\n", getSize()==rawsize);
}

Path Path::mutateClone(real r)
{
	Path p = *this;
	p.reset();
	p.mutate(r);
	return p;
}

Path::~Path()
{

}

