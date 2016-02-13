#include "glwl\window.h"

//#include "glwl\core.h"
//#include "glwl\buffer2.h"

#include "glwl\mesh.h"
#include "glwl\camera.h"
#include "glwl\light.h"
#include "glwl\material.h"
#include "glwl\util.h"

#include "fractals.h"

#include <time.h>

struct lamp {
	glwl::light::point& light;
	glwl::material material;
	glwl::location location;

	lamp(glwl::light::point& prop, glwl::material& m, glwl::location& loc)
		: light(prop), material(m), location(loc) {
		material.prop.emission = light.prop.color;
		material.prop.ambient = light.prop.color;
		material.prop.emission.w = 1;
		material.prop.ambient.w = 1;
	}

	void toggle() {
		light.toggle();
		if (light.prop.enable) material.prop.emission.w = 1;
		else material.prop.emission.w = 0;
	}

	/*void update() {
		location.spawn(light.prop.position);
		location.update();
		material.update();
	}*/
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	try {
		//glwl::file f(L"lol");
		glwlInitSetWindowSize(1366, 768);
		glwlInitSetWindowName(L"OpenGL 4.4");
		glwlInitContextProfile(GLWL_CORE_PROFILE);
		glwlInitContextVersion(3, 1);
		glwlCreateWindow();
		glwlShowWindow();

		glEnable(GL_CULL_FACE);
		//glEnable(GL_LINE_SMOOTH);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glwl::buf::ubo<> ubuf(GL_STATIC_DRAW, 2048);

		glwl::shader_program prog;
		prog << glwl::shader(GL_VERTEX_SHADER, (char*)glwl::blob("vs.glsl").data());
		prog << glwl::shader(GL_FRAGMENT_SHADER, (char*)glwl::blob("ps.glsl").data());
		prog.link();
		prog.bind();

		glwl::_uniform uf(prog.id());
		//auto material_block = uf.get_block("MaterialBlock");
		auto matrix_block = uf.get_block("MatrixBlock");
		//auto light_block = uf.get_block("LightBlock");

		glwl::buf::stream<glwl::buf::ubo<>, 
			glwl::buf::a::no_cached, glwl::buf::b::manual> smbase(&ubuf);
		glwl::_uniform::var<glm::mat4> proj; 
		uf.require<glwl::_uniform::pos>(proj, "mProjection");
		proj.value = glm::perspectiveFov(75.0f, 1366.f, 768.f, 0.001f, 100.0f);
		smbase << proj;

		glwl::camera cam(uf, "mView");
		smbase << cam;

		glwl::ut::box box;
		glwl::location boxpos(uf, "mWorld");
		boxpos.move({ 0, 0, 8 });
		smbase << boxpos;

		glwl::material box_material({ { 1, 0, 0, 1 }, { 1, 0, 0, 1 }, { 1, 1, 1, 1 } },
			glwl::material::offsets::default(uf), glwl::texture());

		glwl::input input;
		input.event({ glwlQuit, GLWL_IK_ESC });

		/*glwl::light::global light_global({ &cam.pos(), glm::vec4{ 1, 1, 1, 0.05 } },
			glwl::light::global::offsets::default(uf));
		glwl::light::point light_point_1({ true, { 1, 1, 1, 0.3 }, { 2, 0, 10 },
			glm::vec3(1, 0.1, 0.01) }, glwl::light::point::offsets::default(uf));
		glwl::light::point light_point_2({ true, { 0, 0, 1, 0.3 }, { 3, 3, 3 }, 
			glm::vec3(1, 0.1, 0.01) }, glwl::light::point::offsets::default(uf));*/

		/*lamp lamp1 = lamp(light_point_1, glwl::material(materials, glwl::texture()),
			glwl::location(ubuf, world.offset));
		lamp1.location.scale({ 0.1, 0.1, 0.1 });

		lamp lamp2 = lamp(light_point_2, glwl::material(materials, glwl::texture()),
			glwl::location(ubuf, world.offset));
		lamp2.location.scale({ 0.1, 0.1, 0.1 });

		int amb = 1, diff = 1, spec = 1, phong = 1;*/

		box.array.bind();
		box.indices.bind();

		ubuf.bind(0, 0, 256);
		matrix_block.bind(0);

		glwlMainLoop([&]() {
			input.update();

			cam.look_up(float(input.mouse(GLWL_IM_Y)) / 300);
			cam.look_right(-float(input.mouse(GLWL_IM_X)) / 300);

			if (input.key(GLWL_IK_W)) cam.move_forward(0.25f);
			if (input.key(GLWL_IK_S)) cam.move_back(0.25f);
			if (input.key(GLWL_IK_A)) cam.move_left(0.25f);
			if (input.key(GLWL_IK_D)) cam.move_right(0.25f);
			if (input.key(GLWL_IK_R)) cam.reset();

			/*if (input.key(DIK_UPARROW)) lamp1.light.position += glm::vec3(0, 0, 0.1);
			if (input.key(DIK_DOWNARROW)) lamp1.light.position -= glm::vec3(0, 0, 0.1);
			if (input.key(DIK_LEFTARROW)) lamp1.light.position -= glm::vec3(0.1, 0, 0);
			if (input.key(DIK_RIGHTARROW)) lamp1.light.position += glm::vec3(0.1, 0, 0);

			if (input.key(DIK_NUMPAD8)) lamp2.light.position += glm::vec3(0, 0, 0.1);
			if (input.key(DIK_NUMPAD5)) lamp2.light.position -= glm::vec3(0, 0, 0.1);
			if (input.key(DIK_NUMPAD4)) lamp2.light.position -= glm::vec3(0.1, 0, 0);
			if (input.key(DIK_NUMPAD6)) lamp2.light.position += glm::vec3(0.1, 0, 0);
			if (input.key(DIK_NUMPAD3)) lamp2.light.position += glm::vec3(0, 0.1, 0);
			if (input.key(DIK_NUMPAD9)) lamp2.light.position -= glm::vec3(0, 0.1, 0);*/

			/*static clock_t time = clock();
			if (clock() - time > 100) {
				if (input.key(DIK_1)) lamp1.toggle();
				if (input.key(DIK_2)) lamp2.toggle();

				//uf_light.bind();
				//if (input.key(DIK_F1)) { uf_light["ambient"] = amb; amb = !amb; }
				//if (input.key(DIK_F2)) { uf_light["diffuse"] = diff; diff = !diff; }
				//if (input.key(DIK_F3)) { uf_light["specular"] = spec; spec = !spec; }
				//if (input.key(DIK_F4)) { uf_light["phong"] = phong; phong = !phong; }

				time = clock();
			}*/

			//light.update();

			smbase << cam;
			//smbase << boxpos;
			
			//cam.update();
			//boxpos.update();
			//box_material.update();
			box.indices.draw();

			//lamp1.update();
			//box.indices.draw();

			//lamp2.update();
			//box.indices.draw();
		});
	}
	catch (_STD exception& error) {
		MessageBoxA(NULL, error.what(), "ERROR", MB_OK | MB_ICONEXCLAMATION);
	}
	return 0;
}