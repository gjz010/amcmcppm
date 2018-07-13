#pragma once
#include "defs.h"
#include <vector>
#include <random>

class Path{
private:
	std::vector<real> vec;
	std::mt19937_64 engine;

	
	unsigned long long seed;
	real uniform();
	real normal();
	int current = 0;
public:
    Path();
    int getSize() const;
    real nextUniform();
	real nextNormal();
	real next();
	void reset();
    real operator[](int index);
	void mutate(real r);
	Path mutateClone(real r);
    ~Path();
};
