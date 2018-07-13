#include "BVHTree.h"

#include <algorithm>

BVHTree::BVHTree()
{
}

void BVHTree::initialize()
{
	struct {
		int parent;
		int start;
		int end;
	} stack[128];
	int leaf = 0;
	stack[0].start = 0;
	stack[0].end = triggers.size();
	stack[0].parent = -1;
	nodes.reserve(2 * triggers.size() + 100);
	int esp = 1;
	while (esp > 0) {
		auto& frame = stack[--esp];
		Node n;
		n.right = -1;
		n.start = frame.start;
		n.count = frame.end - frame.start;
		BoundingBox b1=triggers[n.start]->box;
		BoundingBox b2;
		b2.a = b2.b = ((b1.a + b1.b) / 2);
		for (int i = frame.start + 1; i < frame.end; i++) {
			b1.update(triggers[i]->box.a);
			b1.update(triggers[i]->box.b);
			b2.update((triggers[i]->box.a + triggers[i]->box.b) / 2);
		}
		n.box = b1;
		if (n.count <= 4) {
			n.right = 0;
			leaf++;
		}
		nodes.push_back(n);
		if (frame.parent != -1) {
			nodes[frame.parent].right--;
			if (nodes[frame.parent].right == -3) {
				nodes[frame.parent].right = nodes.size() - 1 - frame.parent;
			}
		}
		if (n.right == 0) continue;
		int split = 0;
		vec3 delta = b2.b - b2.a;
		if (delta[1] > delta[0]) split = 1;
		if (delta[2] > delta[1] && delta[2] > delta[0]) split = 2;
		real v = (b2.a[split] + b2.b[split]) / 2;
		int j = frame.start;
		for (int i = frame.start; i < frame.end; i++) {
			vec3 center = (triggers[i]->box.a + triggers[i]->box.b) / 2;
			if ( center[split]< v) {
				Trigger* t = triggers[i];
				triggers[i] = triggers[j];
				triggers[j] = t;
				j++;
			}
		}
		if (j == frame.start || j == frame.end) j = frame.start + (frame.end - frame.start) / 2;
		int lo = frame.start, hi = frame.end;
		stack[esp].start = j;
		stack[esp].end = hi;
		stack[esp].parent = nodes.size() - 1;
		esp++;
		stack[esp].start = lo;
		stack[esp].end = j;
		stack[esp].parent = nodes.size() - 1;
		esp++;
	}
	debug("Built BVH (%d nodes, with %d leafs) %d\n", nodes.size(), leaf, nodes[0].right);
}
//bool aa_started = false;
IntersectResult BVHTree::intersect(const ray r) const
{
	struct {
		int i;
		real dist;
	} stack[128];
	IntersectResult result = FAIL;
	result.d = 1e+8;
	int esp = 1;
	stack[0].i = 0;
	stack[0].dist = -1e+16;
	//debug("Start\n");
	while (esp > 0) {
		auto& frame = stack[--esp];
		const auto& node = nodes[frame.i];
		if (frame.dist > result.d) continue;
		if (!node.right) {
			for (int i = 0; i < node.count; i++) {
				//debug("Intersect %d %d %d.\n", frame.i, node.count, node.right);
				IntersectResult curr = triggers[node.start + i]->intersect(r);
				if (curr.success && curr.d<result.d) {
					result = curr;
				}
			}
		}
		else {
			double hits[4];
			bool left = nodes[frame.i + 1].box.intersect(r, &hits[0]);
			bool right = nodes[frame.i + node.right].box.intersect(r, &hits[2]);
			if (left && right) {
				int l = frame.i + 1;
				int r = frame.i + node.right;
				if (hits[0] < hits[2]) {
					stack[esp].i = l; stack[esp].dist = hits[0];
					stack[esp+1].i = r; stack[esp+1].dist = hits[2];
				}
				else {
					stack[esp].i = r; stack[esp].dist = hits[2];
					stack[esp + 1].i = l; stack[esp + 1].dist = hits[0];
				}
				esp += 2;
			}
			else if (left) {
				frame.i++;
				frame.dist = hits[0];
				esp++;
			}
			else if (right) {
				frame.i += node.right;
				frame.dist = hits[2];
				esp++;
			}
		}
	}
	//debug("ENd");
	//printf("ITS! %lf\n", r.dir[0]);
	return result;
}


BVHTree::~BVHTree()
{
}

void BoundingBox::update(vec3 p)
{
	a[0] = std::min(p[0], a[0]);
	a[1] = std::min(p[1], a[1]);
	a[2] = std::min(p[2], a[2]);
	b[0] = std::max(p[0], b[0]);
	b[1] = std::max(p[1], b[1]);
	b[2] = std::max(p[2], b[2]);
}






struct vec_t { float x, y, z, pad; };
struct aabb_t {
	vec_t	min;
	vec_t	max;
};

struct ray_t {
	vec_t	pos;
	vec_t	inv_dir;
};
struct ray_segment_t {
	float	t_near, t_far;
};


extern bool ray_box_intersect(const aabb_t &box, const ray_t &ray, ray_segment_t &rs);
bool BoundingBox::intersect(const ray r, double * dists) const
{
	aabb_t box;
	vec_t v;
	v.x = a[0];
	v.y = a[1];
	v.z = a[2];
	box.min = v;
	v.x = b[0];
	v.y = b[1];
	v.z = b[2];
	box.max = v;
	ray_t rs;
	rs.inv_dir.x = 1 / r.dir[0];
	rs.inv_dir.y = 1 / r.dir[1];
	rs.inv_dir.z = 1 / r.dir[2];
	rs.pos.x = r.pos[0];
	rs.pos.y = r.pos[1];
	rs.pos.z = r.pos[2];
	ray_segment_t result;
	bool rrr=ray_box_intersect(box, rs, result);
	dists[0] = result.t_near;
	dists[1] = result.t_far;
	return rrr;
}
