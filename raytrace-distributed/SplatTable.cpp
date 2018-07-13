#include "SplatTable.h"
#include <set>
#include "ImageWriter.h"
#include <algorithm>
#define HASH(x, y, z) ((((x) * 1000000007) + (y)) * 1000000007 + (z));
#define USE_HASH
SplatTable::SplatTable(int width, int height):width(width), height(height)
{
	
}

SplatTable::~SplatTable()
{
}

void SplatTable::initializeTable(std::vector<EyePoint>& points)
{
	debug("[SplatTable::initializeTable] Start\n");
	this->points = &points;
	debug("[SplatTable::initializeTable] Set\n");
	for (auto& p : points) {
		p.count = 0;
		p.flux = { 0,0,0 };
		p.rad2 = MAX_RAD*MAX_RAD;
		p.count = 0;
	}

	reconstructOctree();
	
	std::vector<vec3> image;
	image.resize(width * height, vec3({ 0,0,0 }));
	for (auto& ep : points) {
		//debug("%d %d\n", ep.x, ep.y);
		if (ep.loc[1] < 1e-2) {
			image[ep.y*width + ep.x] = image[ep.y*width + ep.x]+vec3({ 0,1,0 });
		}
		else {
			image[ep.y*width + ep.x] = image[ep.y*width + ep.x]+vec3({ 1,0,0 });
		}
	}
	ImageWriter::write("c:\\MinGW\\eyephase.png", image, width, height);
	
	debug("[SplatTable::initializeTable] Set Done\n");
}
const real alpha = 0.6666666667;

bool SplatTable::splat(const HitPoint & hp)
{
	bool visible = false;
	vec3 delta = (hp.loc- p1) / MAX_RAD;
	if (delta[0] <= 1 || delta[1] <= 1 || delta[2] <= 1) return false;
	unsigned long long x_ = floor(delta[0])-1, y_ = floor(delta[1])-1, z_ = floor(delta[2])-1;
	bool flag = false;
	
#ifdef USE_HASH
	for (int a_ = 0; a_ < 3; a_++, x_++) {
		for (int b_ = 0; b_ < 3; b_++, y_++) {
			for (int c_ = 0; c_ < 3; c_++, z_++) {
				unsigned long long hashcode = HASH(x_, y_, z_);
				auto iter = hash.find(hashcode);
				int ppp = 0;
				while (iter!=hash.end() && iter->first==hashcode) {
					ppp++;
					int i = iter->second;
					auto& p = (*(this->points))[i];

					//debug("Find!\n");
					if (!p.enabled) debug("FAIL!\n");
#else
	auto iter = points->begin();
	while (iter != points->end()) {
		auto& p = *iter;
		if (!p.enabled) continue;
#endif
					if (p.normal*hp.normal > 1e-6) {
						vec3 ab = p.loc - hp.loc;
						//debug("[Distance] (%lf %lf %lf) (%lf %lf %lf) %lf %lf\n", p.loc[0], p.loc[1], p.loc[2], hp.loc[0], hp.loc[1], hp.loc[2], ab*ab, p.rad2);
						if (ab*ab < p.rad2) {
							

							visible = true;
							real beta = ((alpha + (real)p.count) / (1 + (real)p.count));
							p.rad2 *= beta;
							p.count++;
							//debug("%lf %lf %lf %lf\n", hp.flux[0], hp.flux[1], hp.flux[2], beta);
							//debug("%lf %lf %lf\n", p.weight[0], p.weight[1], p.weight[2]);

							p.flux = (p.flux + (p.weight^hp.flux))*beta;
							if (p.x == 936 && p.y == 466) {
								debug("DarkPoint! %lf %lf %lf\n", p.flux[0], p.flux[1], p.flux[2]);
								debug("%lf %lf %lf %lf\n", hp.flux[0], hp.flux[1], hp.flux[2], beta);
								debug("%lf %lf %lf\n", p.weight[0], p.weight[1], p.weight[2]);
							}
							if (p.x == 943 && p.y == 467) {
								debug("LightPoint! %lf %lf %lf\n", p.flux[0], p.flux[1], p.flux[2]);
							}
						}
					}
					iter++;
				}
			//if(ppp>0) debug("%d\n", ppp);
#ifdef USE_HASH
			}
z_ -= 3;
		}
		y_ -= 3;
	}
#endif

	
	return visible;
}

void SplatTable::reconstructOctree()
{
	this->hash = std::unordered_multimap<unsigned long long, int>();
	std::set<unsigned long long> s;
	p1 = { 10000000,10000000,10000000 }, p2 = { -10000000,-10000000,-10000000 };
	for (auto& ep : *(this->points)) {
		for (int i = 0; i < 3; i++) {
			if (ep.loc[i] < p1[i]) p1[i] = ep.loc[i];
			if (ep.loc[i] > p2[i]) p2[i] = ep.loc[i];
		}

	}
	for (int i = 0; i < 3; i++) {
		p1[i] -= 10 * MAX_RAD;
		p2[i] += 10 * MAX_RAD;
	}
	debug("%lf %lf %lf %lf %lf %lf\n", p1[0], p1[1], p1[2], p2[0], p2[1], p2[2]);
	for (int i = 0; i < points->size(); i++) {
		if ((*points)[i].enabled) {
			auto& p = (*points)[i];
			vec3 delta = (p.loc - p1) / MAX_RAD;
			unsigned long long x = floor(delta[0]), y = floor(delta[1]), z = floor(delta[2]);
			unsigned long long hvalue = HASH(x, y, z);
			//debug("%ulld %lf %lf %lf\n", hvalue, delta[0], delta[1], delta[2]);
			this->hash.insert(std::make_pair(hvalue, i));
			s.insert(hvalue);
		}
	}
	debug("%d %d\n", hash.size(), s.size());
	
}

std::vector<vec3> SplatTable::finalize() const
{
	std::vector<vec3> image;
	image.resize(width*height, { 0,0,0 });
	//real sum = 0;
	for (const auto& p : *points) {
		//if (normInfinity(p.flux) > 0) debug("%lf %lf %lf\n", p.flux[0], p.flux[1], p.flux[2]);
		//debug("%lf\n", p.rad2);
		image[p.y*width + p.x] = image[p.y*width + p.x]+p.flux / (p.rad2 * PI);
	}
	/*
	for (auto& v : image) {
		sum += norm1(v);
	}
	sum = 3.0 * 0.18*(real)width*(real)height / sum; //TODO
	for (auto& v : image) {
		v = v * sum;
	}
	*/
	return image;
}
/*
OctreeNode::OctreeNode(): leaf(true)
{
	
}

OctreeNode::~OctreeNode()
{
	if (!leaf) {
		for (int i = 0; i < 8; i++) {
			delete children[i];
		}
	}
}
unsigned long long max_leaf = 0;
int sss = 0;
OctreeNode* biggest;
void OctreeNode::initialize(const std::vector<EyePoint>& ref)
{
	debug("Start Octree initialization.\n");
	//*this = OctreeNode();
	vec3 p1 = { 10000000,10000000,10000000 }, p2 = { -10000000,-10000000,-10000000 };
	//real r = 0;
	for (auto& ep : ref) {
		for (int i = 0; i < 3; i++) {
			if (ep.loc[i] < p1[i]) p1[i] = ep.loc[i];
			if (ep.loc[i] > p2[i]) p2[i] = ep.loc[i];
		}
		//if (ep.rad2 > r) r = ep.rad2;
	}
	//this->max_rad = r;
	for (int i = 0; i < 3; i++) {
		p1[i] -= 10 * MAX_RAD;
		p2[i] += 10 * MAX_RAD;
	}
	std::vector<int> init;
	init.reserve(ref.size());
	for (int i = 0; i < ref.size(); i++) {
		if (ref[i].enabled) {
			if (!lap(p1, p2, ref[i].loc)) {
				debug("Bad point found!\n");
			}
			init.push_back(i);
		}
	}
	debug("(%lf %lf %lf) (%lf %lf %lf)\n", p1[0], p1[1], p1[2], p2[0], p2[1], p2[2]);
	fillin(ref, init, p1, p2, 10); 
	debug("%lld %d\n", max_leaf, sss);
}

//All points in indices are lapped within p1 and p2.
void OctreeNode::fillin(const std::vector<EyePoint>& ref, const std::vector<int>& indices, vec3 p1, vec3 p2, int max_layer)
{

	this->p1t = p1;
	this->p2t = p2;
	std::vector<int> vertices[8];
	
	vec3 p1s[8];
	vec3 p2s[8];
	splitCube(p1, p2, p1s, p2s);
	bool oversplit = true;
	int total_points = 0;
	if (indices.size() >= 10000 && max_layer>0) {
		for (auto& arr : vertices) arr.reserve(indices.size()/8);
		for (int i : indices) {
			bool touched = false;
			oversplit = false;
			vec3 middle = (p1 + p2) / 2;
			vec3& pos = ref[i].loc;
			int index = 0;
			if (pos[0] > middle[0]) index |= 1;
			if (pos[1] > middle[1]) index |= 2;
			if (pos[2] > middle[2]) index |= 4;
			if (!touched) {
				debug("Bad point 2 found!\n");
			}
			if (total_points >= 7 * indices.size()) oversplit = true;
		}
		//debug("%d %d %d %d %d %d %d %d %d %d\n", indices.size(), vertices[0].size(), vertices[1].size(), vertices[2].size(), vertices[3].size(), vertices[4].size(), vertices[5].size(), vertices[6].size(), vertices[7].size(), oversplit);
	}
	//
	if (!oversplit) {
		//if (max_layer <=2) debug("%d %d %d | %d %d %d %d %d %d %d %d\n", total_points, indices.size(), max_layer, vertices[0].size(), vertices[1].size(), vertices[2].size(), vertices[3].size(), vertices[4].size(), vertices[5].size(), vertices[6].size(), vertices[7].size());
		leaf = false;
		for (int i = 0; i < 8; i++) {
			if (vertices[i].size() > 0) {
				OctreeNode* subtree = new OctreeNode();
				sss++;
				subtree->fillin(ref, vertices[i], p1s[i], p2s[i], max_layer - 1);
				this->children[i] = subtree;
			}
			else this->children[i] = nullptr;
		}
	}
	else {
		//if (indices.size()>70000) debug("%d %d %d | %d %d %d %d %d %d %d %d\n", total_points, indices.size(), max_layer, vertices[0].size(), vertices[1].size(), vertices[2].size(), vertices[3].size(), vertices[4].size(), vertices[5].size(), vertices[6].size(), vertices[7].size());
		this->points = indices;
		leaf = true;
		if (indices.size() > max_leaf) max_leaf = indices.size();
		
	}
}

bool OctreeNode::lap(vec3 p1, vec3 p2, vec3 center) const
{
	real sum = 0;
	for (int i = 0; i < 3; i++) {
		if (center[i] < p1[i]) sum += (center[i] - p1[i])*(center[i] - p1[i]);
		else if (center[i] > p2[i]) sum += (center[i] - p2[i])*(center[i] - p2[i]);
	}
	return sum <= MAX_RAD*2;
}

bool OctreeNode::inside(vec3 p1, vec3 p2, vec3 center) const
{
	return (center[0] >= p1[0] && center[1] >= p1[1] && center[2] >= p1[2]) && (center[0] <= p1[0] && center[1] <= p1[1] && center[2] <= p1[2]);
}

void OctreeNode::splitCube(vec3 p1, vec3 p2, vec3 p1s[8], vec3 p2s[8]) const
{
	vec3 delta;
	delta = (p2 - p1) / 2;
	for (int i = 0; i < 8; i++) {
		p1s[i] = p1 + vec3({ (i & 1) ? delta[0] : 0, (i & 2) ? delta[1] : 0, (i & 4) ? delta[2] : 0 });
		p2s[i] = p1s[i] + delta;
	}
}

const OctreeNode * OctreeNode::findLeaf(vec3 pos) const
{
	if (pos[0] > p2t[0] || pos[1] > p2t[1] || pos[2] > p2t[2] || pos[0] < p1t[0] || pos[1] < p1t[1] || pos[2] < p1t[2]) {
		return nullptr;
	}
	const OctreeNode* leaf = this;
	int layer = 0;
	while (leaf!=nullptr && (!leaf->leaf)) {
		vec3 middle = (leaf->p1t+ leaf->p2t) / 2;
		//debug("layer %d, (%lf %lf %lf) to (%lf %lf %lf), middle (%lf %lf %lf) pos (%lf %lf %lf)\n", layer++, leaf->p1t[0], leaf->p1t[1], leaf->p1t[2], leaf->p2t[0], leaf->p2t[1], leaf->p2t[2], middle[0], middle[1], middle[2], pos[0], pos[1], pos[2]);
		int index = 0;
		if (pos[0] > middle[0]) index |= 1;
		if (pos[1] > middle[1]) index |= 2;
		if (pos[2] > middle[2]) index |= 4;
		leaf = leaf->children[index];
	}
	return leaf;
}
*/