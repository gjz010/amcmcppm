#pragma once
#include "defs.h"
#include "vector"
#include <unordered_map>
const real MAX_RAD = 0.5;
/*
struct OctreeNode {
	OctreeNode();
	~OctreeNode();
	bool leaf;
	std::vector<int> points;
	vec3 p1t, p2t;
	OctreeNode* children[8];
	void initialize(const std::vector<EyePoint>& ref);
	void fillin(const std::vector<EyePoint>& ref, const std::vector<int>& indices, vec3 p1, vec3 p2, int max_layer);
	bool lap(vec3 p1, vec3 p2, vec3 center) const;
	bool inside(vec3 p1, vec3 p2, vec3 center) const;
	void splitCube(vec3 p1, vec3 p2, vec3 p1s[8], vec3 p2s[8]) const;
	const OctreeNode* findLeaf(vec3 pos) const;

};
*/
class SplatTable
{
public:
	SplatTable(int width, int height);
	
	~SplatTable();
	void initializeTable(std::vector<EyePoint>& points);
	bool splat(const HitPoint& hp);
	void reconstructOctree();
	std::vector<vec3> finalize() const;
	//OctreeNode octree;
private:
	int width;
	int height;
	std::unordered_multimap<unsigned long long, int> hash;
	vec3 p1, p2;
	std::vector<EyePoint>* points;
};

