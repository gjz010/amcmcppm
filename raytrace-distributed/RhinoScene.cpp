#include "RhinoScene.h"
#include <algorithm>
#include "NurbsModel.h"
#include "MeshModel.h"

RhinoScene::RhinoScene()
{
	 _brdf = new Diffuse(vec3({ 0.6, 0.6, 0.6 }));
	 _brdf2 = new Reflection(vec3({ 0.9,0.9,0.9 }));
}

int RhinoScene::load(const std::string & file)
{
	ON_TextLog dump_to_stdout;
	if (!model.Read(file.c_str(), nullptr)) {
		return 1;
	}
	if (!model.ModelGeometryBoundingBox().IsValid()) return 1;
	if (model.m_settings.m_views.Count() == 0) return 2;
	for (int i = 0; i < model.m_settings.m_views.Count();i++) {
		auto& view = model.m_settings.m_views[i];
		if (view.m_name[0] == L'P') { //Perspective
			ON_3dPoint target=view.TargetPoint();
			auto cam = view.m_vp;
			vec3 cam_target;
			cam_target[0] = target.x;
			cam_target[1] = target.y;
			cam_target[2] = target.z;
			ON_3dPoint loc = cam.CameraLocation();
			
			vec3 cam_pos;
			cam_pos[0] = loc.x;
			cam_pos[1] = loc.y;
			cam_pos[2] = loc.z;
			vec3 cam_dir = normalize(cam_target - cam_pos);
			auto up = cam.CameraUp();

			vec3 cam_up;
			cam_up[0] = up.x;
			cam_up[1] = up.y;
			cam_up[2] = up.z;
			ref = camera(cam_pos, cam_dir, normalize(cross(cam_dir, cam_up)), 40);
			ref.cy = -ref.cy;
			vp_width = 40;
			printf("%lf %lf %lf(%lf %lf %lf)|%lf %lf %lf|%lf %lf %lf|%lf %lf %lf\n", ref.pos[0], ref.pos[1], ref.pos[2], cam_target[0], cam_target[1], cam_target[2], ref.dir[0], ref.dir[1], ref.dir[2], ref.cx[0], ref.cx[1], ref.cx[2], ref.cy[0], ref.cy[1], ref.cy[2]);
		}
	}
	ONX_ModelComponentIterator it_light(model, ON_ModelComponent::Type::RenderLight);
	for (const ON_ModelComponent* model_component = it_light.FirstComponent();
		nullptr != model_component;
		model_component = it_light.NextComponent())
	{
		const ON_ModelGeometryComponent* model_geometry = ON_ModelGeometryComponent::Cast(model_component);
		if (nullptr != model_geometry)
		{
			const ON_Geometry* geometry = model_geometry->Geometry(nullptr);
			const ON_3dmObjectAttributes* attributes = model_geometry->Attributes(nullptr);
			if (nullptr != geometry && nullptr != attributes)
			{
				const ON_Light* light = ON_Light::Cast(geometry);
				if(light){
					printf("Light! %s\n", geometry->ClassId()->ClassName());
					vec3 pos;
					pos[0] = light->Location().x;
					pos[1] = light->Location().y;
					pos[2] = light->Location().z;
					vec3 color;
					color[0] = light->Diffuse().FractionRed();
					color[1] = light->Diffuse().FractionGreen();
					color[2] = light->Diffuse().FractionBlue();
					real strength = light->Intensity();
					addLight(pos, color, strength);
				}
				
			}
		}
		
	}
	int cntr = 0;
	ONX_ModelComponentIterator it(model, ON_ModelComponent::Type::ModelGeometry);
	for (const ON_ModelComponent* model_component = it.FirstComponent(); 
		nullptr != model_component; 
		model_component = it.NextComponent())
	{
		printf("%s\n", model_component->ClassId()->ClassName());
		const ON_ModelGeometryComponent* model_geometry = ON_ModelGeometryComponent::Cast(model_component);
		if (nullptr != model_geometry)
		{
			const ON_Geometry* geometry = model_geometry->Geometry(nullptr);
			const ON_3dmObjectAttributes* attributes = model_geometry->Attributes(nullptr);
			if (nullptr != geometry && nullptr != attributes)
			{
				BRDF* current = _brdf;
				if (attributes->m_material_index >= 0) {
					//auto mat=attributes->m_rendering_attributes.m_materials[];
					printf("Material %d\n", attributes->m_material_index);
					const ON_ModelComponent* _mat=model.ComponentFromIndex(ON_ModelComponent::Type::RenderMaterial, attributes->m_material_index).ModelComponent();
					const ON_Material* mat=ON_Material::Cast(_mat);
					
					MultiBRDF* current_brdf = new MultiBRDF;
					current = current_brdf;
					if (mat->m_textures.Count() > 0 && mat->m_textures[0].m_bOn) {
						printf("Texture Found!\n");
						const wchar_t* path = mat->m_textures[0].m_image_file_reference.FullPathAsPointer();
						current_brdf->add(new TexturedDiffuse(path), 1);
					}else
					if (mat->m_diffuse != ON_UNSET_COLOR) {
						vec3 c;
						
						c[0] = mat->Diffuse().Red()/255.0;
						c[1] = mat->Diffuse().Green()/255.0;
						c[2] = mat->Diffuse().Blue()/255.0;
						printf("Diffuse. %lf %lf %lf\n", VEC3(c*255.0));
						printf("%lf %lf %lf\n", VEC3(c));
						current_brdf->add(new Diffuse(c*0.6 + vec3({0.025, 0.025, 0.025})), 1);
					}

					if (mat->Transparency() > 1e-3 && mat->m_transparent != ON_UNSET_COLOR) {
						vec3 c;
						printf("Refraction.\n");
						c[0] = mat->m_transparent.Red()/255.0;
						c[1] = mat->m_transparent.Green()/255.0;
						c[2] = mat->m_transparent.Blue()/255.0;
						current_brdf->add(new Refraction(c*0.9, 1.2, 1), mat->Transparency());
					}
					
					if (mat->Reflectivity() > 1e-3 && mat->m_reflection != ON_UNSET_COLOR) {
						printf("Reflect: %lf\n", mat->Reflectivity());
						vec3 c;
						c[0] = mat->m_reflection.Red()/255.0;
						c[1] = mat->m_reflection.Green()/255.0;
						c[2] = mat->m_reflection.Blue()/255.0;
						current_brdf->add(new Reflection(c*0.9), mat->Reflectivity());
					}
					
					//mat->m_textures[0].
					printf("%ld\n", mat);
				}
				if (current == _brdf) printf("BUG!\n");
				const ON_Mesh* mesh;
				if (mesh = ON_Mesh::Cast(geometry)) {
					MeshModel* meshModel = new MeshModel();
					meshModel->initialize(mesh);
					meshModel->visit(bvh);
					meshModel->brdf = current;
					//exit(0);
					continue;
				}
				if (geometry->HasBrepForm()) {
					const ON_Brep* brep = geometry->BrepForm();
					for (int srfcnt = 0; srfcnt < brep->m_F.Count(); srfcnt++) {
						ON_NurbsSurface* _nurbssrf = new ON_NurbsSurface();
						ON_NurbsSurface& nurbssrf = *_nurbssrf;
						brep->m_S[srfcnt]->GetNurbForm(nurbssrf);
						ON_4dPoint v;
						for (int i = 0; i < nurbssrf.m_cv_count[0]; i++) {
							for (int j = 0; j < nurbssrf.m_cv_count[1]; j++) {
								nurbssrf.GetCV(i, j, v);
								//printf("%lf %lf %lf %lf\n", v.x, v.y, v.z, v.w);
							}
						}
						//printf("Dimension:%d\n", nurbssrf.Dimension());
						//nurbssrf.Dump(dump_to_stdout);
						vec3 col = { 0.1,0.1,0.1 };
						NurbsModel* model = new NurbsModel;
						model->initialize(&nurbssrf);
						model->visit(bvh);
						model->brdf = current;
						//m = model;
					
					}


					/*
					auto aabb=brep->BoundingBox();
					vec3 a,b;
					a[0] = aabb.m_min.x;
					a[1] = aabb.m_min.y;
					a[2] = aabb.m_min.z;
					b[0] = aabb.m_max.x;
					b[1] = aabb.m_max.y;
					b[2] = aabb.m_max.z;
					
					col[cntr % 3] = 0.9;
					*/
					/*
					for (auto& bb : model.m_BoundingBoxes) {
						_addBoundingBox(bb.a, bb.b, col);
					}
					*/
					//cntr++;
					//printf("%s %lf %lf %lf %lf %lf %lf\n", geometry->ClassId()->ClassName(), aabb.m_min.x, aabb.m_min.y, aabb.m_min.z, aabb.m_max.x, aabb.m_max.y, aabb.m_max.z);
				}
				
					
			}
		}
	}
	bvh.initialize();
	//exit(0);
	return 0;
}


RhinoScene::~RhinoScene()
{
	delete _brdf;
}

camera RhinoScene::getReferenceCamera() const
{
	return ref;
}

IntersectResult RhinoScene::intersect(ray r) const
{
	return bvh.intersect(r);
	/*
	IntersectResult rs = m->intersect(r, 0.5, 0.5);
	rs.brdf = _brdf;
	return rs;
	*/
	//return _planehit(r);
}

LightRay RhinoScene::light(Path & path) const
{
	real r = path.next()*total_light;
	r -= 0.001;
	for (auto& light : lights) {
		r -= light.strength;
		if (r <= 0) {
			vec3 dir = { path.next()-0.5, path.next()-0.5 , path.next()-0.5 };
			dir = normalize(dir);
			LightRay ry;
			if (dir[2] > 0) dir = -dir;
			//printf("%lf %lf %lf\n", VEC3(light.color));
			ry.flux = (light.color^vec3({ 100000,100000,100000 }));
			ray rr;
			rr.pos = light.pos;
			rr.dir = dir;
			ry.r = rr;
			return ry;

		}
	}
	auto& light = lights[0];
	vec3 dir = { path.next()-0.5, path.next()-0.5 , path.next()-0.5 };
	dir = normalize(dir);
	if (dir[2] > 0) dir = -dir;
	LightRay ry;
	ry.flux = (light.color^vec3({ 100000,100000,100000 }));
	ray rr;
	rr.pos = light.pos;
	rr.dir = dir;
	ry.r = rr;
	return ry;
}

void RhinoScene::addLight(vec3 pos, vec3 color, real strength)
{
	PointLight l;
	l.pos = pos;
	l.color = color;
	l.strength = strength;
	total_light += strength;
	lights.push_back(l);
}

void RhinoScene::_addBoundingBox(vec3 a, vec3 b, vec3 col)
{
	vec3 delta = b - a;
	printf("%lf %lf %lf\n", delta[0], delta[1], delta[2]);
	vec3 dx = { 0,0,0 }, dy = { 0,0,0 }, dz = { 0,0,0 };
	dx[0] = delta[0]; dy[1] = delta[1]; dz[2] = delta[2];
	_addplane(a, a + dy+dz, col, 0);
	_addplane(b - dy-dz, b, col, 0);
	_addplane(a, a + dx+dz, col, 1);
	_addplane(b - dx-dz, b, col, 1);
	_addplane(a, a + dx+dy, col, 2);
	_addplane(b - dx-dy, b, col, 2);
}

void RhinoScene::_addplane(vec3 a, vec3 b, vec3 col, int lock)
{
	_plane p;
	p.a = a;
	p.b = b;
	p.color = col;
	p.lock = lock;
	_planes.push_back(p);
}

IntersectResult RhinoScene::_planehit(ray r) const
{
	int locktable[3][2] = { {1,2},{0,2},{0,1} };
	auto result = FAIL;
	for (const auto& plane : _planes) {
		int m = locktable[plane.lock][0];
		int n = locktable[plane.lock][1];
		//if (plane.a[plane.lock] - r.pos[plane.lock]<1e-5) continue;
		real ldist = (plane.a[plane.lock] - r.pos[plane.lock]) / r.dir[plane.lock];
		if (ldist < 1e-5) continue;
		vec3 isct = r.pos + (r.dir*ldist);
		if (!(isct[m] >= plane.a[m] && isct[m] <= plane.b[m] && isct[n] >= plane.a[n] && isct[n] <= plane.b[n])) {
			continue;
		}
		IntersectResult rs;
		rs.u = (isct[m] - plane.a[m]) / (plane.b[m] - plane.a[m]);
		rs.v = (isct[n] - plane.a[n]) / (plane.b[n] - plane.a[n]);
		rs.brdf = _brdf;
		rs.success = true;
		rs.intersect = isct;
		rs.d = ldist;
		vec3 normal = { 0,0,0 };
		normal[plane.lock] = -r.dir[plane.lock];
		rs.normal = normalize(normal);
		//printf("Dot:%lf\n", rs.normal*r.dir);
#undef min
		result = std::min(rs, result);
	}
	//printf("%d %lf %lf %lf\n", result.success, result.intersect[0], result.intersect[1], result.intersect[2]);
	return result;
}
