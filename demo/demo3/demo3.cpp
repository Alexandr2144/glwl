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

		app.context.cull(GL_FRONT);
		//glEnable(GL_MULTISAMPLE);
		//glShadeModel(GL_FLAT);
		//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		//glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);


		auto err = glGetError();
		glwl::buf::raw<glwl::error::buf::std> ubuf(GL_UNIFORM_BUFFER, GL_STATIC_DRAW, 2048);

		glwl::shader<_GLWL a::std_check>::program sh;
		sh << glwl::shader<_GLWL a::std_check>(GL_VERTEX_SHADER, (char*)glwl::file(L"Bulb.vs").data());
		sh << glwl::shader<_GLWL a::std_check>(GL_FRAGMENT_SHADER, (char*)glwl::file(L"Bulb.ps").data());
		sh.link();

		glwl::shader<_GLWL a::std_check>::program fxaa;
		fxaa << glwl::shader<_GLWL a::std_check>(GL_VERTEX_SHADER, (char*)glwl::file(L"Bulb.vs").data());
		fxaa << glwl::shader<_GLWL a::std_check>(GL_FRAGMENT_SHADER, (char*)glwl::file(L"FXAA.ps").data());
		fxaa.link();

		struct point { glm::vec3 pos; glm::vec2 tex; } v[] = {
			{ { -1.f, -1.f, 0.f }, { 0, 0 } }, { { -1.f, +1.f, 0.f }, { 0, 1 } },
			{ { +1.f, -1.f, 0.f }, { 1, 0 } }, { { +1.f, +1.f, 0.f }, { 1, 1 } },
		};
		GLuint idx[] = { 0, 1, 2, 2, 1, 3 };

		_GLWL buf::vbo <> vbuf(GL_STATIC_DRAW, sizeof(v), v);
		_GLWL buf::ibo<GLuint> ibuf(GL_STATIC_DRAW, 4, 6, idx);
		_GLWL buf::elem_stream<point, glwl::buf::vbo<>,
			glwl::buf::a::cached<1024>, glwl::buf::b::manual> vs(&vbuf);
		ibuf.mode = GL_TRIANGLES;
		ibuf.count = 6;

		_GLWL vao array;
		array.bind();
		array[0] << vs;
		array(0)[0][0] << glwl::format<glm::vec3>();
		array(1)[0][sizeof(glm::vec3)] << glwl::format<glm::vec3>();

		glwl::uniform uf(sh.id());
		auto material_block = uf.get_block("MaterialBlock");
		auto matrix_block = uf.get_block("MatrixBlock");
		auto light_block = uf.get_block("LightBlock");

		glwl::buf::stream<decltype(ubuf), 
			glwl::buf::a::cached<256>, glwl::buf::b::manual> smbase(&ubuf);

		glwl::buf::stream<decltype(ubuf),
			glwl::buf::a::cached<256>, glwl::buf::b::manual> sbm(&ubuf, 256);
		glwl::material bulb_material({ { 1, 1, 1, 1 },
		{ 0.8, 0.75, 0.7, 0.9 }, { 1, 1, 1, 1 }, { 1, 1, 1, 0 }, 20.5f, 0.25f }, uf, "material");
		
		glwl::buf::stream<decltype(ubuf),
			glwl::buf::a::cached<256>, glwl::buf::b::manual> sl(&ubuf, 512);
		glwl::light::global light_global({ &v[0].pos,
			glm::vec4{ 1, 1, 1, 0.2 }, 1 }, uf, "light");
		glwl::light::point light({ true, { 1, 1, 1, 0.5 }, { 0, 0, 2 },
			glm::vec3(0.7, 0.1, 0.01) }, uf, "light.point");

		glwl::input input;
		input.event({ glwlQuit, GLWL_IK_ESC });

		ubuf.bind(0, 0, 256);
		matrix_block.bind(0);
		ubuf.bind(1, 256, 512);
		material_block.bind(1);
		ubuf.bind(2, 512, 512+256);
		light_block.bind(2);

		glwl::location pos(uf, "mWorld");
		glm::vec4 x_axis(1, 0, 0, 0);
		glm::vec4 y_axis(0, 1, 0, 0);
		
		/*GLuint fbo; //Framebuffer
		glwl::texture<glwl::error::texture::std> fbt(GL_TEXTURE_2D);
		fbt.reserve_tex2d(GL_TEXTURE_2D, 0, 1366, 768, GL_RGBA, GL_RGBA, GL_FLOAT);
		glGenFramebuffersEXT(1, &fbo);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
			GL_TEXTURE_2D, fbt.id(), 0);
			*/
		smbase << pos;
		sbm << bulb_material;
		sl << light_global;
		sl << light;

		sh.use();

		app.loop([&]() {
			input.update();

			pos.rotate(+float(input.mouse(GLWL_IM_X)) / 300, (glm::vec3)y_axis);
			//x_axis = x_axis*pos.getm();
			pos.rotate(+float(input.mouse(GLWL_IM_Y)) / 300, (glm::vec3)x_axis);
			//y_axis = y_axis*pos.getm();
			if (input.mouse(GLWL_IM_Z) > 0) pos.scale(0.9f);
			else if (input.mouse(GLWL_IM_Z) < 0) pos.scale(1.1f);

			smbase << pos;
			/*glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
			sh.use();
			//array.bind();
			ibuf.bind();
			ibuf.draw();
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);*/

			//fxaa.use();
			//smbase << pos;

			//glBindTexture(GL_TEXTURE_2D, fbt.id());
			//array.bind();
			ibuf.bind();
			ibuf.draw();
		});
	}
	catch (_STD exception& error) {
		MessageBoxA(NULL, error.what(), "ERROR", MB_OK | MB_ICONEXCLAMATION);
	}
	return 0;
}