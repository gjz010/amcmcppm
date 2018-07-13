#pragma once
//Some annotations
typedef double real;
typedef unsigned char byte;
#define DEBUG
#ifdef DEBUG
#include <cstdio>
#define debug printf
#else
#define debug
#endif
#define VEC3(p) (p)[0], (p)[1], (p)[2]
const unsigned int Infinity = 1<<31;
const real PI = 3.1415926;
template <int N>
struct vec{
    real arr[N];
    real& operator[](int index){
        return arr[index];
    }
    const real& operator[](int index) const{
        return arr[index];
    }
};
template <int N>
vec<N> operator-(const vec<N>& v){
    vec<N> vr;
    for(int i=0;i<N;i++){
        vr[i]=-v[i];
    }
    return vr;
};
template <int N>
vec<N> operator+(const vec<N>& v1, const vec<N>& v2){
    vec<N> v;
    for(int i=0;i<N;i++){
        v[i]=v1[i]+v2[i];
    }
    return v;
};

template <int N>
vec<N> operator-(const vec<N>& v1, const vec<N>& v2){
    vec<N> nv2=-v2;
    return v1+nv2;
};
template<int N>
vec<N> operator*(real r, const vec<N>& v){
    vec<N> vr;
    for(int i=0;i<N;i++){
        vr[i]=r*v[i];
    }
    return vr;
};
template<int N>
vec<N> operator*(const vec<N>& v, real r){
    return r*v;
};
template<int N>
vec<N> operator/(const vec<N>& v, real r) {
	return (1.0/r)*v;
};
template<int N>
real operator*(const vec<N>& v1, const vec<N>& v2){
    real v=0;
    for(int i=0;i<N;i++){
        v+=v1[i]*v2[i];
    }
    return v;
};

template<int N>
vec<N> operator^(const vec<N>& v1, const vec<N>& v2) {
	vec<N> v;
	for (int i = 0; i<N; i++) {
		v[i] = v1[i] * v2[i];
	}
	return v;
};


template<int N>
real norm1(const vec<N>& v) {
	real sum = 0;
	for (int i = 0; i < N; i++) {
		sum += abs(v[i]);
	}
	return sum;
}
template<int N>
real norm2(const vec<N>& v) {
	return sqrt(v*v);
}
template<int N>
real normInfinity(const vec<N>& v) {
	real sum = 0;
	for (int i = 0; i < N; i++) {
		if (abs(v[i]) > sum) sum = abs(v[i]);
	}
	return sum;
}

template<int N>
vec<N> normalize(const vec<N>& v) {
	real norm = norm2(v);
	return v / norm;
}


using vec2=vec<2>;
using vec3=vec<3>;
using vec4 = vec<4>;
using color = vec<3>;
struct ray {
	vec<3> pos;
	vec<3> dir;
};

vec3 cross(const vec3& v1, const vec3& v2);
struct camera {
	vec3 pos;
	vec3 dir;
	vec3 cx;
	vec3 cy;
	real len;
	camera(vec3 pos, vec3 dir, vec3 cx, real len);
	camera();
};
struct HitPoint {
	vec3 loc;
	vec3 normal;
	vec3 flux;
};
struct EyePoint {
	bool enabled;
	vec3 loc;
	vec3 normal;
	vec3 weight;
	int x;
	int y;
	real rad2;
	int count;
	vec3 flux;
};
vec3 sphere(real theta, real phi);