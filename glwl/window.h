#include "core.h"

namespace glwl { class window; }
glwl::window& glwlGetWindow();


#ifdef _WIN32
#include "win32\window.h"
#include "win32\input.h"
#include "win32\file.h"

//void glwlInitSetWindowName(LPWSTR name) { glwl::window::init.wnd_name = name; }
//void glwlInitSetWindowClassName(LPWSTR class_name) { glwl::window::init.wnd_class_name = class_name; }

#endif //_WIN32
/*
void glwlInitSetWindowSize(int width, int height) {
	glwl::window::init.width = width;
	glwl::window::init.height = height;
}
void glwlInitSetWindowPosition(int x, int y) {
	glwl::window::init.x = x;
	glwl::window::init.y = y;
}
void glwlInitContextVersion(int major, int minor) {
	glwl::window::init.glrc.version.major = major;
	glwl::window::init.glrc.version.minor = minor;
}
void glwlInitFullscreen(bool fullscreen = true) { glwl::window::init.fullscreen = fullscreen; }
void glwlInitContextProfile(GLWL_PROFILE profile) { glwl::window::init.glrc.profile = profile; }
void glwlCreateWindow() { glwl::window::instance().create(); }
void glwlShowWindow() { glwl::window::instance().show(); }

template <typename LambdaTy>
void glwlMainLoop(LambdaTy display) { glwl::window::instance().loop(display); }

glwl::window& glwlGetWindow() { return glwl::window::instance(); }*/
