#include "NurbsModel.h"
#include <algorithm>
#include <opennurbs.h>

NurbsModel::NurbsModel()
{
}


NurbsModel::~NurbsModel()
{
}
inline vec3 pos(const ON_4dPoint hp) {
	vec3 t;
	t[0] = hp.x / hp.w;
	t[1] = hp.y / hp.w;
	t[2] = hp.z / hp.w;
	return t;
}
inline vec3 pos(const vec4 hp) {
	vec3 t;
	t[0] = hp[0] / hp[3];
	t[1] = hp[1] / hp[3];
	t[2] = hp[2] / hp[3];
	return t;
}
void NurbsModel::initialize(const void * _nurbssrf)
{
	model = _nurbssrf;
	const ON_NurbsSurface* nurbssrf = (const ON_NurbsSurface*)_nurbssrf;
	std::vector<vec4> cv;
	std::vector<real> knoti;
	std::vector<real> knotj;
	for (int i = 0; i < nurbssrf->m_cv_count[0]; i++) {
		for (int j = 0; j < nurbssrf->m_cv_count[1]; j++) {
			vec4 v;
			ON_4dPoint vp;
			nurbssrf->GetCV(i, j, vp);
			v[0] = vp.x;
			v[1] = vp.y;
			v[2] = vp.z;
			v[3] = vp.w;
			cv.push_back(v);
		}
	}
	
	knoti.push_back(nurbssrf->Domain(0).Min());
	knotj.push_back(nurbssrf->Domain(1).Min());
	for (int i = 0; i < nurbssrf->KnotCount(0); i++) {
		knoti.push_back(nurbssrf->Knot(0)[i]);
	}
	for (int j = 0; j < nurbssrf->KnotCount(1); j++) {
		knotj.push_back(nurbssrf->Knot(1)[j]);
	}
	knoti.push_back(nurbssrf->Domain(0).Max());
	knotj.push_back(nurbssrf->Domain(1).Max());
	printf("%d %d %d %d %d\n", nurbssrf->CVCount(0), nurbssrf->m_cv_count[0], nurbssrf->m_cv_count[1], nurbssrf->m_order[0],
		nurbssrf->m_order[1] );
	initialize(nurbssrf->m_cv_count[0],
		nurbssrf->m_cv_count[1],
		nurbssrf->m_order[0],
		nurbssrf->m_order[1],
		cv, knoti, knotj
	);
}

void NurbsModel::initialize(int i, int j, int iorder, int jorder, std::vector<vec4> cv, std::vector<real> knoti, std::vector<real> knotj)
{

	this->cv = cv;
	this->i = i;
	this->j = j;
	this->iorder = iorder;
	this->jorder = jorder;
	this->knoti = knoti;
	this->knotj = knotj;
	printf("%d\n", i);
#ifdef DEBUG
	printf("NurbsModel::initialize i=%d j=%d order=<%d, %d>, knot=<%d, %d>\n", i, j, iorder, jorder, knoti.size(), knotj.size());
	printf("KnotI: {");
	for (real r : knoti) printf("%lf ", r);
	printf("}\nKnotJ: {");
	for (real r : knotj) printf("%lf ", r);
	printf("}\n");
#endif
	//Tesselation
	flatten();
	_enabled = true;
}
#undef min
#undef max
std::vector<real> solveKnots(std::vector<real> knot, int order, std::vector<vec4>& cv, int vorder, int jo, bool u) {
	std::vector<int> pole;
	pole.resize(knot.size() - 1, 0);
	for (int v_index = 0; v_index < jo;v_index++) {
		for (int k = 0; k < knot.size() - 1; k++) {
			if (knot.at(k + 1) - knot.at(k) < 1e-4) continue;
			if (pole.at(k) == 0) pole.at(k) = 1;
			vec3 lastV;
			real maxAj = 0;
			real totalVj = 0;
			int lower = std::max(k - order + 1, 0);
			for (int t0 = lower + 1; t0 <= k; t0++) {
				int index = (u ? t0*vorder + v_index : v_index*order + t0);
				int index2 = (u ? (t0 - 1)*vorder + v_index : v_index*order + t0 - 1);
				//printf("%d %d\n", index, index2);
				vec3 V = (pos(cv.at(index)) - pos(cv.at(index2)))*(order - 1) / (knot.at(t0 + order - 1) - knot.at(t0));
				if (t0 > lower + 1) {
					vec3 Aj = (order - 2)*(V - lastV) / (knot.at(t0 + order - 2) - knot.at(t0));
					real nAj = norm2(Aj);
					if (nAj > maxAj) maxAj = nAj;
				}
				lastV = V;
				totalVj += norm2(V) / (order - 1);
			}
			pole[k] = std::max(pole[k], (int)ceil(0.2*pow(knot.at(k + 1) - knot.at(k), 1.5)*maxAj / sqrt(totalVj)));
		}
		
	}
	std::vector<real> result;
	for (int i = 0; i < pole.size(); i++) {
		if (pole[i] > 0) {
			real delta = (knot[i + 1] - knot[i]) / pole[i];
			for (int j = 0; j < pole[i]; j++) {
				result.push_back(j*delta + knot[i]);
			
			}
		}
	}
	return result;
}
void NurbsModel::flatten()
{
	const ON_NurbsSurface* nurbssrf = (const ON_NurbsSurface*)model;
	std::vector<real> splitu;
	std::vector<real> splitv;
	//std::vector<vec3> V;
	splitu = solveKnots(knoti, iorder, cv, jorder, j, true);
	splitv = solveKnots(knotj, jorder, cv, iorder, i, false);

	//nurbssrf->InsertKnot

	splitu.push_back(nurbssrf->Domain(0).Max());
	splitv.push_back(nurbssrf->Domain(1).Max());
	ON_NurbsSurface tmp(*nurbssrf);
	ON_TextLog dump_to_stdout;
	printf("Before Flatten\n");
	//tmp.Dump(dump_to_stdout);
	std::vector<int> lacking_dim;
	int kid = 0, lid=0;
	for (int k = 0; k < splitu.size() - 1; k++) {
		int current = 0;
		while (tmp.m_knot[0][kid] == splitu[k]) { current++; kid++; if (kid >= tmp.m_knot_capacity[0]) break; }
		lacking_dim.push_back(iorder - 1 - current);
	}
	for (int k = 0; k < splitu.size() - 1; k++) {
		if (lacking_dim[k] > 0) tmp.InsertKnot(0, splitu[k], lacking_dim[k]);
	}
	lacking_dim.clear();
	for (int l = 0; l < splitv.size() - 1; l++) {
		int current = 0;
		while (tmp.m_knot[0][lid] == splitv[l]) { current++; lid++; if (lid >= tmp.m_knot_capacity[1]) break; }
		lacking_dim.push_back(jorder - 1 - current);
	}
	for (int l = 0; l < splitv.size() - 1; l++) {
		if (lacking_dim[l] > 0) tmp.InsertKnot(1, splitv[l], lacking_dim[l]);
	}
	printf("After Flatten %d %d %d %d\n", splitu.size(), splitv.size(), tmp.m_cv_count[0], tmp.m_cv_count[1]);
	//tmp.Dump(dump_to_stdout);
	BoundingBox total;
	total.a = total.b = pos(tmp.ControlPoint(0, 0));
	for (int p = 0; p < splitu.size() - 1; p++) {
		for (int q = 0; q < splitv.size() - 1; q++){
			int ukstart = p*(iorder - 1);
			int ukend = ukstart + iorder - 1;
			int vkstart = q*(jorder - 1);
			int vkend = vkstart + jorder - 1;
			BoundingBox bb;
			bb.a = bb.b = pos(tmp.ControlPoint(ukstart, ukend));
			for (int r = ukstart; r <= ukend; r++) {
				for (int s = vkstart; s <= vkend; s++) {
					bb.update(pos(tmp.ControlPoint(r, s)));
					total.update(pos(tmp.ControlPoint(r, s)));
				}
			}
			//m_BoundingBoxes.push_back(bb);
			real startu = (splitu[p] + splitu[p + 1]) / 2;
			real startv = (splitv[q] + splitv[q + 1]) / 2;
			NurbsTrigger trigger;
			trigger.box = bb;
			trigger.u = startu;
			trigger.v = startv;
			//printf("%lf %lf %lf -> %lf %lf %lf uv %lf %lf %d->%d %d->%d\n", VEC3(bb.a), VEC3(bb.b), startu, startv, ukstart, ukend, vkstart, vkend);
			trigger.model = this;
			m_Quads.push_back(trigger);
			//us.push_back(startu);
			//vs.push_back(startv);
			/*
			InternalTrigger itg;
			itg.box = bb;
			itg.u = startu;
			itg.v = startv;
			itgs.push_back(itg);
			*/
		}

	}
	/*
	for (auto& itg : itgs) {
		blocks.triggers.push_back(&itg);
	}
	blocks.initialize();
	*/
	//real buffer[1024];
	/*
	if (nurbssrf->Evaluate(startu, startv, 1, 32, (double*)buffer)) {
	printf("%lf %lf %lf (%lf %lf %lf %lf %lf %lf)\n", buffer[0], buffer[1], buffer[2], VEC3(bb.a), VEC3(bb.b));
	}
	*/
	//m_Quads.push_back(trigger);
	/*
	union {
		double buffer[1024];
		vec3 v;
	} u;
	int hint[2];
	for (int k = 0; k < splitu.size() - 1; k++) {
		for (int l = 0; l < splitv.size() - 1; l++) {
			quad q;
			nurbssrf->Evaluate(splitu[k], splitv[l], 0, 32, (double*)u.buffer, 0, (int*)hint);
			q.p[0] = u.v;
			nurbssrf->Evaluate(splitu[k+1], splitv[l], 0, 32, (double*)u.buffer, 0, (int*)hint);
			q.p[1] = u.v;
			nurbssrf->Evaluate(splitu[k+1], splitv[l+1], 0, 32, (double*)u.buffer, 0, (int*)hint);
			q.p[2] = u.v;
			nurbssrf->Evaluate(splitu[k], splitv[l+1], 0, 32, (double*)u.buffer, 0, (int*)hint);
			q.p[3] = u.v;
			q.u = (splitu[k] + splitu[k + 1]) / 2;
			q.v = (splitv[l] + splitv[l + 1]) / 2;
			m_Quads.push_back(q);
			//printf("%lf %lf %lf|%lf %lf %lf|%lf %lf %lf|%lf %lf %lf\n", q.p[0][0], q.p[0][1], q.p[0][2],
			//	q.p[1][0], q.p[1][1], q.p[1][2], q.p[2][0], q.p[2][1], q.p[2][2], q.p[3][0], q.p[3][1], q.p[3][2]);
		}
	}
	*/
	//printf("%d\n",us.size());
}

bool NurbsModel::getuv(ray r, real & u, real & v) const
{
	auto rs = blocks.intersect(r);
	if (!rs.success) return false;
	u = rs.u; v = rs.v;
	return true;
}


IntersectResult NurbsModel::intersect(ray r, real startu, real startv)
{
	const ON_NurbsSurface* nurbssrf = (const ON_NurbsSurface*)model;
	//printf("%d %d\n",nurbssrf->m_cv_count[0], nurbssrf->m_cv_count[1]);
	real u = startu, v = startv;
	double buffer[1024];
	for (int _ = 0; _ < 1024; _++) buffer[_] = 114.514;
	int hint[2];
	vec3 N1 = { 1,0,0 };
	vec3 N2 = { 0,1,0 };
	if (norm2(cross(N1,r.dir)) < 1e-4) {
		N1 = normalize(cross(r.dir, N2));
		N2 = normalize(cross(r.dir, N1));
	}
	else {
		N2 = normalize(cross(r.dir, N1));
		N1 = normalize(cross(r.dir, N2));
	}
	//printf("%lf %lf %lf\n", N1*N2, N1*r.dir, N2*r.dir);
	real d1 = N1*r.pos;
	real d2 = N2*r.pos;
	//printf("%lf %lf %lf|%lf %lf %lf|%lf %lf %lf\n", r.dir[0], r.dir[1], r.dir[2], N1[0], N1[1], N1[2], N2[0], N2[1], N2[2]);
	int M = 100;
	real last_pp = 1e+8;
	//printf("%lf %lf\n", u, v);
	while (M--) {
		if(!nurbssrf->Evaluate(u, v, 1, 32, (double*)buffer,0,(int*)hint)) return FAIL;
		vec3 y = { buffer[0], buffer[1], buffer[2] };
		vec3 du = { buffer[32], buffer[33], buffer[34] };
		vec3 dv = { buffer[64], buffer[65], buffer[66] };
		//printf("%lf %lf %lf|%lf %lf %lf|%lf %lf %lf\n", y[0], y[1], y[2], du[0], du[1], du[2], dv[0], dv[1], dv[2]);
		real p1 = N1*y - d1;
		real p2 = N2*y - d2;
		//if (p1*p1 + p2*p2 > last_pp) {
			//return FAIL;
		//}
		//last_pp = p1*p1+p2*p2;
		if (p1*p1 + p2*p2 < 1e-3) {
			//printf("[%lf,%lf,%lf] [%lf,%lf,%lf] %lf %lf\n", N1[0], N1[1], N1[2], N2[0], N2[1], N2[2], p1, p2);
			//printf("[%lf,%lf,%lf] [%lf,%lf,%lf]\n", y[0], y[1], y[2],r.pos[0], r.pos[1], r.pos[2]);
			//printf("%lf,%lf,%lf\n", r.dir[0], r.dir[1], r.dir[2]);
			//printf("!!! %lf %lf\n", N1*y - d1, N1*y - d2);
			vec3 normal = normalize(cross(du, dv));
			IntersectResult rs;
			rs.intersect = y;
			rs.normal = normal;
			rs.success = true;
			rs.u = u;
			rs.v = v;
			rs.brdf = brdf;
			rs.d = norm2(y - r.pos);
			if (u<0 || u>nurbssrf->Domain(0).Max() || v<0 || v>nurbssrf->Domain(1).Max()) return FAIL;
			//printf("%lf %lf %lf %lf %lf\n", y[0], y[1], y[2], u, v);
			//real dd = (y - r.pos)*r.dir;
			//real cc =norm2(cross(normalize(y - r.pos), normalize(r.dir)));
			//printf("S! %lf %lf\n", dd,cc);
			//printf("%d\n", 100-M);
			return rs;
		}
		else {
			real a, b, c, d;
			a = N1*du; b = N1*dv; c = N2*du; d = N2*dv;
			real det = a*d - b*c;
			real a_, b_, c_, d_;
			a_ = d; d_ = a; b_ = -b; c_ = -c;
			u = u - (a_*p1 + b_*p2) / det;
			v = v - (c_*p1 + d_*p2) / det;

		}
		//printf("Evaluate:%lf %lf %lf %lf %lf %lf %lf\n", buffer[0], buffer[1], buffer[2], buffer[32], buffer[33], buffer[34],buffer[3]);
		//break;
	}
	//printf("TIMEOUT!\n");
	//exit(0);
	return FAIL;

}

void NurbsModel::visit(BVHTree & tree)
{
	for (int i = 0; i < m_Quads.size(); i++) {
		tree.triggers.push_back(&m_Quads[i]);
	}
}


IntersectResult NurbsModel::NurbsTrigger::intersect(const ray r) const
{
	//printf("Slow Op\n");
	return model->intersect(r, u, v);
	/*
	real tu, tv;
	if(!model->getuv(r, tu, tv)) return FAIL;
	auto rs= model->intersect(r, tu, tv);
	//if(rs.success) printf("%lf %lf %lf (%lf %lf %lf) (%lf %lf %lf) (%lf %lf %lf) (%lf %lf %Lf)\n", VEC3(rs.intersect), VEC3(box.a), VEC3(box.b), VEC3(r.dir), VEC3(r.pos));
	return rs;
	*/
}

IntersectResult NurbsModel::InternalTrigger::intersect(const ray r) const
{

	double d[2];
	bool succ = this->box.intersect(r, d);
	IntersectResult rs;
	rs.u = u;
	rs.v = v;
	rs.success = succ;
	rs.d = d[0];
	return rs;
}
