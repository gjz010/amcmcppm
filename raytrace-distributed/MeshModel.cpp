#include "MeshModel.h"
#include <opennurbs.h>


MeshModel::MeshModel()
{
}

void MeshModel::initialize(const void * _mesh)
{
	const ON_Mesh& mesh = *(const ON_Mesh*)_mesh;
	int i0, i1, i2, j0, j1, j2;
	int fi;
	ON_3fPoint v[4];
	ON_3fVector n[4];
	ON_2fPoint t[4];

	const int face_count = mesh.FaceCount();
	const bool bHasNormals = mesh.HasVertexNormals();
	const bool bHasTCoords = mesh.HasTextureCoordinates();

	triangle tg;
	for (fi = 0; fi < face_count; fi++) {
		const ON_MeshFace& f = mesh.m_F[fi];

		v[0] = mesh.m_V[f.vi[0]];
		v[1] = mesh.m_V[f.vi[1]];
		v[2] = mesh.m_V[f.vi[2]];


		if (bHasNormals) {
			n[0] = mesh.m_N[f.vi[0]];
			n[1] = mesh.m_N[f.vi[1]];
			n[2] = mesh.m_N[f.vi[2]];
		}

		if (bHasTCoords) {
			t[0] = mesh.m_T[f.vi[0]];
			t[1] = mesh.m_T[f.vi[1]];
			t[2] = mesh.m_T[f.vi[2]];
		}

		if (f.IsQuad()) {
			// quadrangle - render as two triangles
			v[3] = mesh.m_V[f.vi[3]];
			if (bHasNormals)
				n[3] = mesh.m_N[f.vi[3]];
			if (bHasTCoords)
				t[3] = mesh.m_T[f.vi[3]];
			if (v[0].DistanceTo(v[2]) <= v[1].DistanceTo(v[3])) {
				i0 = 0; i1 = 1; i2 = 2;
				j0 = 0; j1 = 2; j2 = 3;
			}
			else {
				i0 = 1; i1 = 2; i2 = 3;
				j0 = 1; j1 = 3; j2 = 0;
			}
		}
		else {
			// single triangle
			i0 = 0; i1 = 1; i2 = 2;
			j0 = j1 = j2 = 0;
		}

		// first triangle
		
		if (bHasNormals)
			tg.norm(0, n[i0].x, n[i0].y, n[i0].z);
		if (bHasTCoords)
			tg.tex(0, t[i0].x, t[i0].y);
		tg.vert(0, v[i0].x, v[i0].y, v[i0].z);

		if (bHasNormals)
			tg.norm(1, n[i1].x, n[i1].y, n[i1].z);
		if (bHasTCoords)
			tg.tex(1, t[i1].x, t[i1].y);
		tg.vert(1, v[i1].x, v[i1].y, v[i1].z);

		if (bHasNormals)
			tg.norm(2, n[i2].x, n[i2].y, n[i2].z);
		if (bHasTCoords)
			tg.tex(2, t[i2].x, t[i2].y);
		tg.vert(2, v[i2].x, v[i2].y, v[i2].z);
		this->mesh.push_back(tg);

		if (j0 != j1) {
			// if we have a quad, second triangle
			if (bHasNormals)
				tg.norm(0, n[j0].x, n[j0].y, n[j0].z);
			if (bHasTCoords)
				tg.tex(0, t[j0].x, t[j0].y);
			tg.vert(0, v[j0].x, v[j0].y, v[j0].z);

			if (bHasNormals)
				tg.norm(1, n[j1].x, n[j1].y, n[j1].z);
			if (bHasTCoords)
				tg.tex(1, t[j1].x, t[j1].y);
			tg.vert(1, v[j1].x, v[j1].y, v[j1].z);

			if (bHasNormals)
				tg.norm(2, n[j2].x, n[j2].y, n[j2].z);
			if (bHasTCoords)
				tg.tex(2, t[j2].x, t[j2].y);
			tg.vert(2, v[j2].x, v[j2].y, v[j2].z);
			this->mesh.push_back(tg);
		}

	}
	for (int i = 0; i < this->mesh.size(); i++) {
		MeshTrigger trigger;
		BoundingBox box;
		box.a = box.b = this->mesh[i].verts[0];
		box.update(this->mesh[i].verts[1]);
		box.update(this->mesh[i].verts[2]);
		trigger.box = box;
		trigger.model = this;
		trigger.index = i;
		triggers.push_back(trigger);
	}
}

void MeshModel::visit(BVHTree & tree)
{
	for (int i = 0; i < triggers.size(); i++) {
		tree.triggers.push_back(&triggers[i]);
	}
}


MeshModel::~MeshModel()
{
}

MeshModel::triangle::triangle()
{
	verts[0] = vec3({0}); verts[1] = vec3({0}); verts[2] = vec3({0});
	norms[0] = vec3({ 0 }); norms[1] = vec3({ 0 }); norms[2] = vec3({ 0 });
	texs[0] = vec2({ 0 }); texs[1] = vec2({ 0 }); texs[2] = vec2({ 0 });
}

void MeshModel::triangle::vert(int c,real x, real y, real z)
{
	this->verts[c][0] = x;
	this->verts[c][1] = y;
	this->verts[c][2] = z;
}

void MeshModel::triangle::norm(int c,real x, real y, real z)
{
	this->norms[c][0] = x;
	this->norms[c][1] = y;
	this->norms[c][2] = z;
}

void MeshModel::triangle::tex(int c,real x, real y)
{
	this->texs[c][0] = x;
	this->texs[c][1] = y;
}

IntersectResult MeshModel::MeshTrigger::intersect(const ray r) const
{
	//printf("????\n");
	//printf("%lf %lf %lf %lf %lf %lf\n", r.dir[0], r.dir[1], r.dir[2], r.pos[0], r.pos[1], r.pos[2]);
	triangle tg = this->model->mesh[this->index];
	
	vec3 v0 = tg.verts[0], v1 = tg.verts[1], v2 = tg.verts[2];
	vec3 v0v1 = v1 - v0;
	vec3 v0v2 = v2 - v0;
	vec3 pvec = cross(r.dir, v0v2);
	real det = v0v1*pvec;
	//printf("%lf %lf %lf %lf %lf %lf %lf %lf %lf\n", VEC3(tg.verts[0]), VEC3(tg.verts[1]), VEC3(tg.verts[2]));
	// ray and triangle are parallel if det is close to 0
	if (abs(det) < 1e-4) return FAIL;
	real invDet = 1 / det;

	vec3 tvec = r.pos - v0;
	real u = (tvec*pvec) * invDet;
	if (u < 0 || u > 1) return FAIL;

	vec3 qvec = cross(tvec, v0v1);
	real v = (r.dir*qvec) * invDet;
	if (v < 0 || u + v > 1) return FAIL;

	real t = (v0v2 * qvec) * invDet;
	if (t < 1e-4) return FAIL;
	IntersectResult ir;
	ir.success = true;
	ir.d = t;
	ir.intersect = r.pos + r.dir*t;
	if (norm2(tg.norms[0]) < 1e-5) {
		//printf("114!\n");
		ir.normal = normalize(cross(v0v1, v0v2));
		if (ir.normal*r.dir > 0) ir.normal = -ir.normal;
	}
	else {
		//printf("514!\n");
		ir.normal = (1 - u - v)*tg.norms[0] + u*tg.norms[1] + v*tg.norms[2];
		
	}
	//vec3 pos = (1 - u - v)*tg.verts[0] + u*tg.verts[1] + v*tg.verts[2];
	//printf("%lf %lf %lf %lf %lf %lf\n", VEC3(pos), VEC3(ir.intersect));
	vec2 tx = (1 - u - v)*tg.texs[0] + u*tg.texs[1] + v*tg.texs[2];
	ir.u = tx[0]; 
	ir.v = tx[1];
	ir.brdf = this->model->brdf;
	//printf("!!!!\n");
	return ir;
	
}
