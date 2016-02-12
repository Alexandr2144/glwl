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
	glwl::light_point& light;
	glwl::material material;
	glwl::location location;

	lamp(glwl::light_point& prop, glwl::material& m, glwl::location& loc)
		: light(prop), material(m), location(loc) {
		material.emission = light.color;
		material.ambient = light.color;
		material.emission.w = 1;
		material.ambient.w = 1;
	}

	void toggle() {
		light.toggle();
		if (light.enable) material.emission.w = 1;
		else material.emission.w = 0;
	}

	void update() {
		location.spawn(light.position);
		location.update();
		material.update();
	}
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	try {
		//glwl::file f(L"lol");
		glwlInitSetWindowSize(1366, 768);
		glwlInitSetWindowName(L"OpenGL 3.1");
		glwlInitContextProfile(GLWL_CORE_PROFILE);
		glwlInitContextVersion(4, 4);
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

		glwl::_uniform::block matrixes(prog.id(), "Matrixes");
		auto world = matrixes.require<glwl::_uniform::array>("mWorld");
		auto view = matrixes.require<glwl::_uniform::array>("mView");
		auto proj = matrixes.require<glwl::_uniform::array>("mProjection");
		//ubuf.write(proj.offset, 64,
		//	&glm::perspectiveFov(75.0f, 1366.f, 768.f, 0.001f, 100.0f));

		glwl::camera cam(ubuf, view.offset);


		glwl::ut::box box;
		glwl::location boxpos(ubuf, world.offset); boxpos.move({ 0, 0, 8 });

		glwl::_uniform::block materials(prog.id(), "MaterialBlock");
		glwl::material box_material(materials, glwl::texture());

		box_material.ambient = { 1, 0, 0, 1 };
		box_material.diffuse = { 1, 0, 0, 1 };
		box_material.specular = { 1, 1, 1, 1 };

		glwl::input input;
		input.event({ glwlQuit, GLWL_IK_ESC });

		glwl::_uniform::block lights(prog.id(), "LightBlock");
		glwl::light light(cam.pos(), lights);
		light.ambient = glm::vec4(1, 1, 1, 0.05);
		light.point[0] = { true, { 1, 1, 1, 0.3 }, { 2, 0, 10 }, glm::vec3(1, 0.1, 0.01) };
		light.point[1] = { true, { 0, 0, 1, 0.3 }, { 3, 3, 3 }, glm::vec3(1, 0.1, 0.01) };

		lamp lamp1 = lamp(light.point[0], glwl::material(materials, glwl::texture()),
			glwl::location(ubuf, world.offset));
		lamp1.location.scale({ 0.1, 0.1, 0.1 });

		lamp lamp2 = lamp(light.point[1], glwl::material(materials, glwl::texture()),
			glwl::location(ubuf, world.offset));
		lamp2.location.scale({ 0.1, 0.1, 0.1 });

		int amb = 1, diff = 1, spec = 1, phong = 1;

		box.array.bind();
		box.indices.bind();

		glwlMainLoop([&]() {
			input.update();

			cam.look_up(float(input.mouse(GLWL_IM_Y)) / 300);
			cam.look_right(-float(input.mouse(GLWL_IM_X)) / 300);

			if (input.key(GLWL_IK_W)) cam.move_forward(0.25f);
			if (input.key(GLWL_IK_S)) cam.move_back(0.25f);
			if (input.key(GLWL_IK_A)) cam.move_left(0.25f);
			if (input.key(GLWL_IK_D)) cam.move_right(0.25f);
			if (input.key(GLWL_IK_R)) cam.reset();

			if (input.key(DIK_UPARROW)) lamp1.light.position += glm::vec3(0, 0, 0.1);
			if (input.key(DIK_DOWNARROW)) lamp1.light.position -= glm::vec3(0, 0, 0.1);
			if (input.key(DIK_LEFTARROW)) lamp1.light.position -= glm::vec3(0.1, 0, 0);
			if (input.key(DIK_RIGHTARROW)) lamp1.light.position += glm::vec3(0.1, 0, 0);

			if (input.key(DIK_NUMPAD8)) lamp2.light.position += glm::vec3(0, 0, 0.1);
			if (input.key(DIK_NUMPAD5)) lamp2.light.position -= glm::vec3(0, 0, 0.1);
			if (input.key(DIK_NUMPAD4)) lamp2.light.position -= glm::vec3(0.1, 0, 0);
			if (input.key(DIK_NUMPAD6)) lamp2.light.position += glm::vec3(0.1, 0, 0);
			if (input.key(DIK_NUMPAD3)) lamp2.light.position += glm::vec3(0, 0.1, 0);
			if (input.key(DIK_NUMPAD9)) lamp2.light.position -= glm::vec3(0, 0.1, 0);

			static clock_t time = clock();
			if (clock() - time > 100) {
				if (input.key(DIK_1)) lamp1.toggle();
				if (input.key(DIK_2)) lamp2.toggle();

				//uf_light.bind();
				//if (input.key(DIK_F1)) { uf_light["ambient"] = amb; amb = !amb; }
				//if (input.key(DIK_F2)) { uf_light["diffuse"] = diff; diff = !diff; }
				//if (input.key(DIK_F3)) { uf_light["specular"] = spec; spec = !spec; }
				//if (input.key(DIK_F4)) { uf_light["phong"] = phong; phong = !phong; }

				time = clock();
			}

			light.update();

			cam.update();
			boxpos.update();
			box_material.update();
			box.indices.draw();

			lamp1.update();
			box.indices.draw();

			lamp2.update();
			box.indices.draw();
		});
	}
	catch (_STD exception& error) {
		MessageBoxA(NULL, error.what(), "ERROR", MB_OK | MB_ICONEXCLAMATION);
	}
	return 0;
}