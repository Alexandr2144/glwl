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
	glwl::material& material;
	glwl::location& location;

	lamp(glwl::light::point& prop, glwl::material& m, 
		glwl::location& loc)
		: light(prop), material(m), location(loc) {
		material.prop.emission = light.prop.color;
		material.prop.ambient = light.prop.color;
		material.prop.emission.w = 1;
		material.prop.ambient.w = 1;
	}

	void move(glm::vec3 offset) {
		light.prop.position += offset;
		location.spawn(light.prop.position);
	}

	void toggle() {
		light.toggle();
		if (light.prop.enable) material.prop.emission.w = 1;
		else material.prop.emission.w = 0;
	}

	template <class BufferTy, class CachePolicy,
		template <class> class BindPolicy>
	void load(glwl::buf::stream<BufferTy, CachePolicy, BindPolicy>& ubuf) {
		ubuf << location;
		ubuf << material;
		ubuf << light;
	}
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	try {
		glwl::app app;
		app.setWindowSize(1366, 768);
		app.setWindowName(L"OpenGL 4.4");
		app.setContextProfile(GLWL_CORE_PROFILE);
		app.setContextVersion(4, 4);
		app.launch();
		app.showWindow();

		app.context.cull(GL_BACK);
		app.context.enable(GL_MULTISAMPLE);

		glEnable(GL_BLEND);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // здесь надо то что тебе подойдёт 
		glShadeModel(GL_SMOOTH);

		glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
		glEnable(GL_POLYGON_SMOOTH);

		glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
		glEnable(GL_POINT_SMOOTH);

		glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
		glEnable(GL_LINE_SMOOTH);


		//glDisable(GL_DEPTH_TEST);
		glEnable(GL_ALPHA_TEST);

		glwl::buf::raw<glwl::error::buf::std> ubuf(GL_UNIFORM_BUFFER, GL_STATIC_DRAW, 2048);

		glwl::shader<_GLWL a::std_check>::program prog;
		prog << glwl::shader<_GLWL a::std_check>(GL_VERTEX_SHADER, (char*)glwl::file(L"vs.glsl").data());
		prog << glwl::shader<_GLWL a::std_check>(GL_FRAGMENT_SHADER, (char*)glwl::file(L"ps.glsl").data());
		prog.link();
		prog.use();


		glwl::uniform uf(prog.id());
		auto material_block = uf.get_block("MaterialBlock");
		auto matrix_block = uf.get_block("MatrixBlock");
		auto light_block = uf.get_block("LightBlock");

		glwl::ut::dds<glwl::error::texture::std> metal(glwl::file(L"Texture.dds"));
		glwl::texture<glwl::error::texture::std> mono(GL_TEXTURE_2D, GL_LINEAR, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, &glm::vec4(1, 1, 1, 1));

		glwl::buf::stream<decltype(ubuf), 
			glwl::buf::a::no_cached, glwl::buf::b::manual> smbase(&ubuf);
		glwl::uniform::var<glm::mat4> proj; 
		uf.require<glwl::uniform::pos>(proj, "mProjection");
		proj.value = glm::perspectiveFov(75.0f, 1366.f, 768.f, 0.001f, 100.0f);
		smbase << proj;

		glwl::camera cam(uf, "mView");
		smbase << cam;

		glwl::ut::box box;
		glwl::location boxpos(uf, "mWorld");
		boxpos.move({ 0, 0, 8 });
		smbase << boxpos;

		glwl::buf::stream<decltype(ubuf),
			glwl::buf::a::cached<256>, glwl::buf::b::manual> sbm(&ubuf, 256);
		glwl::material box_material({ { 1, 1, 1, 1 },
			{ 1, 1, 1, 1 }, { 1, 1, 1, 0.7 } }, uf, "material");

		glwl::input input;
		input.event({ glwlQuit, GLWL_IK_ESC });

		glwl::buf::stream<decltype(ubuf),
			glwl::buf::a::cached<256>, glwl::buf::b::manual> sl(&ubuf, 512);
		glwl::light::global light_global({ &cam.pos(), 
			glm::vec4{ 1, 1, 1, 0.05 }, 2 }, uf, "light");

		glwl::light::point lamp1_light({ true, { 1, 1, 1, 0.3 }, { 2, 0, 10 },
			glm::vec3(1, 0.1, 0.01) }, uf, "light.point[0]");
		glwl::material lamp1_material({ { 1, 1, 1, 1 },
			{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 1, 1, 1, 1 } }, uf, "material");
		glwl::location lamp1_location(boxpos);
		lamp1_location.spawn(lamp1_light.prop.position);
		lamp1_location.scale({ 0.1, 0.1, 0.1 });
		lamp lamp1(lamp1_light, lamp1_material, lamp1_location);


		glwl::light::point lamp2_light({ true, { 0, 0, 1, 0.3 }, { 1, 1, 4 },
			glm::vec3(1, 0.1, 0.01) }, uf, "light.point[1]");
		glwl::material lamp2_material({ { 0, 0, 1, 1 },
			{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 1, 1 } }, uf, "material");
		glwl::location lamp2_location(boxpos);
		lamp2_location.spawn(lamp2_light.prop.position);
		lamp2_location.scale({ 0.1, 0.1, 0.1 });
		lamp lamp2(lamp2_light, lamp2_material, lamp2_location);


		//int amb = 1, diff = 1, spec = 1, phong = 1;

		fractal::cube_optimize cube({ 2, -0.5, 3 }, 1.0f);
		glwl::material cube_material({ { 0, 1, 0, 1 },
			{ 0, 1, 0, 1 }, { 0, 1, 0, 1 }, { 1, 1, 1, 0 } }, uf, "material");

		box.array.bind();
		box.indices.bind();

		ubuf.bind(0, 0, 256);
		matrix_block.bind(0);
		ubuf.bind(1, 256, 256);
		material_block.bind(1);
		ubuf.bind(2, 512, 256);
		light_block.bind(2);

		app.loop([&]() {
			input.update();

			cam.look_up(float(input.mouse(GLWL_IM_Y)) / 300);
			cam.look_right(-float(input.mouse(GLWL_IM_X)) / 300);

			if (input.key(GLWL_IK_W)) cam.move_forward(0.25f);
			if (input.key(GLWL_IK_S)) cam.move_back(0.25f);
			if (input.key(GLWL_IK_A)) cam.move_left(0.25f);
			if (input.key(GLWL_IK_D)) cam.move_right(0.25f);
			if (input.key(GLWL_IK_R)) cam.reset();
			smbase << cam;
			sl << light_global;

			if (input.key(DIK_UPARROW)) lamp1.move(glm::vec3(0, 0, 0.1));
			if (input.key(DIK_DOWNARROW)) lamp1.move(-glm::vec3(0, 0, 0.1));
			if (input.key(DIK_LEFTARROW)) lamp1.move(-glm::vec3(0.1, 0, 0));
			if (input.key(DIK_RIGHTARROW)) lamp1.move(glm::vec3(0.1, 0, 0));

			if (input.key(DIK_NUMPAD8)) lamp2.move(glm::vec3(0, 0, 0.1));
			if (input.key(DIK_NUMPAD5)) lamp2.move(-glm::vec3(0, 0, 0.1));
			if (input.key(DIK_NUMPAD4)) lamp2.move(-glm::vec3(0.1, 0, 0));
			if (input.key(DIK_NUMPAD6)) lamp2.move(glm::vec3(0.1, 0, 0));
			if (input.key(DIK_NUMPAD3)) lamp2.move(glm::vec3(0, 0.1, 0));
			if (input.key(DIK_NUMPAD9)) lamp2.move(-glm::vec3(0, 0.1, 0));

			static clock_t time = clock();
			if (clock() - time > 100) {
				if (input.key(DIK_1)) { lamp1.toggle(); }
				if (input.key(DIK_2)) { lamp2.toggle(); }

				if (input.key(DIK_ADD)) { cube.increase(); }
				if (input.key(DIK_SUBTRACT)) { cube.decrease(); }

				//uf_light.bind();
				//if (input.key(DIK_F1)) { uf_light["ambient"] = amb; amb = !amb; }
				//if (input.key(DIK_F2)) { uf_light["diffuse"] = diff; diff = !diff; }
				//if (input.key(DIK_F3)) { uf_light["specular"] = spec; spec = !spec; }
				//if (input.key(DIK_F4)) { uf_light["phong"] = phong; phong = !phong; }

				time = clock();
			}

			sbm << cube_material;
			cube.load(boxpos.offset(), smbase);

			box.array.bind();
			box.indices.bind();

			metal.bind();
			smbase << boxpos;
			sbm << box_material;
			box.indices.draw();

			
			mono.bind();
			smbase << lamp1_location;
			sbm << lamp1_material;
			sl << lamp1_light;
			box.indices.draw();

			smbase << lamp2_location;
			sbm << lamp2_material;
			sl << lamp2_light;
			box.indices.draw();
		});
	}
	catch (_STD exception& error) {
		MessageBoxA(NULL, error.what(), "ERROR", MB_OK | MB_ICONEXCLAMATION);
	}
	return 0;
}