#pragma once

#include <exception>
#include <cstdio>

#include <utility>
#include <new>

#include <GL\glew.h>
#include <GL\wglew.h>
#include <GL\freeglut.h>

#include <glm\common.hpp>

#pragma comment(lib,"OpenGL32.lib")
#pragma comment(lib,"freeglut.lib")
#pragma comment(lib,"glew32.lib")

#define _GLWL ::glwl::

#define GLWL_MEM_ZERO(value) _STD memset(&value, NULL, sizeof(value));
#define GLWL_CTOR_RV_STD(myclass) \
	myclass(myclass&& src) { _STD memcpy(this, &src, sizeof(myclass)); \
	_STD memset(&src, NULL, sizeof(myclass)); }
#define GLWL_ASSIG_RV_STD(myclass) \
	myclass& operator=(myclass&& src) { \
		this->~myclass(); \
		new(this) myclass(_STD move(src)); \
		return *this; }
#define GLWL_ASSIG_LV_STD(myclass) \
	myclass& operator=(const myclass& src) {\
	this->~myclass(); \
	new(this) myclass(src); \
	return *this; }
#define GLWL_CTOR_RV_DELETE(myclass) \
	myclass(myclass&& src) = delete; \
	myclass& operator=(myclass&& src) = delete;
#define GLWL_CTOR_LV_DELETE(myclass) \
	myclass(const myclass& src) = delete; \
	myclass& operator=(const myclass& src) = delete;

namespace glwl {
	class exception : public _XSTD exception {
	public:
		enum code {
			ok							= 0x00,
			bad_glGen					= 0x01,
			bad_glBind					= 0x02,
			bad_glDelete				= 0x04,
			bad_glBufferData			= 0x08,
			bad_glBufferSubData			= 0x10,
			bad_glCopyBufferSubData		= 0x20,

			bad_glBindBufferBase			= 0x0040,
			bad_glBindBufferRange			= 0x0080,
			bad_glUniformBlockBinding		= 0x0100,
			bad_glGetUniformIndices			= 0x0200,
			bad_glGetActiveUniformsiv		= 0x0400,

			bad_glVertexAttribFormat = 0x140,
			bad_glVertexAttribBinding = 0x140,
			bad_glEnableVertexAttribArray = 0x140,
			bad_glDisableVertexAttribArray = 0x140,

			bad_glUnknown = 0x0fffffff,

			buffer_OutOfRange = 0x1fffffff,

			all = 0xffffffff,
			none = 0x0,
		};

		exception() {}
		template <typename... ArgsTy>
		exception(const char* format, ArgsTy... args) : _XSTD exception(_msg, 0) {
			sprintf_s(_msg, format, args...);
		}

		template <typename... ArgsTy>
		exception(GLenum gl_error, code proc_code, const char* proc_postfix, ArgsTy... proc_args)
			: exception(format(proc_code), proc_postfix, proc_args..., recognize(gl_error)) {}

		template <typename FTy>
		static const char* recognize(FTy proc, code& proc_code) {
			if ((void*)proc == glGenBuffers) proc_code = bad_glGen; return "Buffers";
			if ((void*)proc == glBindBuffer) proc_code = bad_glBind; return "Buffer";
			if ((void*)proc == glDeleteBuffers) proc_code = bad_glDelete; return "Buffers";
			if ((void*)proc == glBufferData) proc_code = bad_glBufferData; return "";
			if ((void*)proc == glBufferSubData) proc_code = bad_glBufferSubData; return "";
			if ((void*)proc == glCopyBufferSubData) proc_code = bad_glCopyBufferSubData; return "";

			if ((void*)proc == glBindBufferBase) proc_code = bad_glBindBufferBase; return "";
			if ((void*)proc == glBindBufferRange) proc_code = bad_glBindBufferRange; return "";
			if ((void*)proc == glUniformBlockBinding) proc_code = bad_glUniformBlockBinding; return "";
			if ((void*)proc == glGetUniformIndices) proc_code = bad_glGetUniformIndices; return "";
			if ((void*)proc == glGetActiveUniformsiv) proc_code = bad_glGetActiveUniformsiv; return "";

			if ((void*)proc == glBindVertexArray) proc_code = bad_glBind; return "VertexArray";
			if ((void*)proc == glBindVertexBuffer) proc_code = bad_glBind; return "VertexBuffer";

			proc_code = bad_glUnknown; return "";
		}
		static const char* recognize(GLenum gl_error) {
			switch (gl_error) {
			case GL_NO_ERROR: return "GL_NO_ERROR";
			case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
			case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
			case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
			case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
			case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
			case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
			case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
			default: return "<unknown>";
			}
		}
	private:
		static const char* format(code proc_code) {
			switch (proc_code) {
			case bad_glGen: return "glGen%s function failure. Arguments: \
				{ ID_ArraySize: %d,  ID_Array: %d }. Error message: %s";
			case bad_glBind: return "glBind%s function failure. Arguments: \
				{ Type: %x, ID: %d }. Error message: %s";
			case bad_glDelete: return "glDelete%s function failure. Arguments: \
				{ IdArraySize: %d, ID_Array: %d }. Error message: %s";
			case bad_glBufferData: return "glBufferData%s function failure. Arguments: \
				{ Type: %x, Size: %d, Data: %x, Usage: %x }. Error message: %s";
			case bad_glBufferSubData: return "glBufferSubData%s function failure. Arguments: \
				{ Type: %x, Offset: %d, Size: %d, Data: %x,}. Error message: %s";
			case bad_glCopyBufferSubData: return "glCopyBufferSubData%s function failure. Arguments: \
				{ Target1: %x, Target2: %x, Offset1: %d, Offset2: %d, Size: %d }. Error message: %s";

			case bad_glBindBufferBase: return "glBindBufferBase%s function failure. Arguments: \
				{ Type: %x, BindIdx: %d, ID: %d }. Error message: %s";
			case bad_glBindBufferRange: return "glBindBufferRange%s function failure. Arguments: \
				{ Type: %x, BindIdx: %d, ID: %d, Offset: %d, Size: %d }. Error message: %s";
			case bad_glUniformBlockBinding: return "glUniformBlockBinding%s function failure. Arguments: \
				{ Type: %x, BindIdx: %d, Block: %d, Offset: %d, Size: %d }. Error message: %s";
			case bad_glGetUniformIndices: return "_glGetUniformIndices%s function failure. Arguments: \
				{ Program: %d, Count: %d, Names: %x, Indices: %x }. Error message: %s";
			case bad_glGetActiveUniformsiv: return "glGetActiveUniformsiv%s function failure. Arguments: \
				{ Program: %x, Count: %d, Indices: %d, ParamType: %d, ParamPtr: %d }. Error message: %s";
			}
			return "<Unknown error>";
		}
		char _msg[512];
	};

	template <typename ValTy>
	class ptr {
	public:
		ptr() : _ptr(nullptr) {}
		ptr(ValTy& val) : _ptr(&val) {}
		ptr& operator=(ValTy& val) { _ptr = &val; return *this; }
		GLWL_CTOR_RV_DELETE(ptr)
		GLWL_CTOR_LV_DELETE(ptr)
		inline ValTy* operator->() const { return _ptr; }
	private:
		ValTy* _ptr;
	};

	class unknown {
	public:
		~unknown() {}
		unknown() : unknown(NULL) {}
		unknown(GLuint id) : _id(id), _owner(true), _errmask(_GLWL exception::all),
			_errfl(_GLWL exception::ok) {}

		unknown(unknown&& src) : _id(src._id) { src._id = NULL; }
		unknown(const unknown& src) : _id(src._id), _owner(false) {}

		inline _GLWL exception::code exception(_GLWL exception::code mask) { 
			_GLWL exception::code _oldmask = _errmask;
			_errmask = mask; 
			return _oldmask;
		}
		inline _GLWL exception::code exception() const { return _errmask; }
		inline _GLWL exception::code error() const { return _errfl; }

		inline bool isref() const { return !_owner; }
		inline bool good() const { return _errfl == _GLWL exception::ok; }
		inline bool bad() const { return _errfl != _GLWL exception::ok; }

		GLuint id() const { return _id; }
	protected:
		GLuint _id;

		template <typename FTy, typename... ArgsTy>
		inline void call(FTy glf, ArgsTy... proc_args) const {
			glf(proc_args...);
			GLenum error = glGetError();
			if (error) {
				const char* proc_postfix = _GLWL exception::recognize(glf, _errfl);
				if (_errmask & _errfl) throw glwl::exception(error, _errfl, proc_postfix, proc_args...);
			}
		}
		inline void dtor(void(GLAPIENTRY *dtrof)(GLuint)) { if (_owner) dtrof(_id); }
		inline void dtor(void(GLAPIENTRY *dtrof)(GLuint, GLuint*)) { if (_owner) dtrof(1, &_id); }
	private:
		bool _owner;
		_GLWL exception::code _errmask;
		mutable _GLWL exception::code _errfl;
	};

	template <typename _Ty>
	class object {
	public:
		object() : _ptr(&_default), _usedef(true) {}
		object(object&& src) : _ptr(src._ptr), _usedef(src._ptr),
			_default(_STD move(src._default)) { if (_usedef) _ptr = &_default; src._ptr = nullptr; }
		object(const object& src) : _ptr(src._ptr), _usedef(src._ptr) { 
			if (_usedef) _ptr = &_default; }
	private:
		bool _usedef;
		_Ty _default;
		_Ty* _ptr;
	};

	struct offset {
		GLuint val;
		offset(GLuint value) : val(value) {}
	};
	struct reserve {
		GLuint count;
		const void* data;
		reserve(GLuint _count, const void* _data = nullptr)
			: count(_count), data(_data) {}
	};
	template <typename ValueTy>
	struct format {
		GLboolean norm;
		format(GLboolean normalize = GL_FALSE) : norm(normalize) {}
		static const GLboolean normalized = GL_TRUE;
		static const GLboolean default = GL_FALSE;
	};

	//Prototype
	template <typename ValueTy>
	class stream;
	template <typename ValueTy>
	struct type {};

	//Float
	template <>
	struct type<GLfloat> {
		static const GLenum count = 1;
		static const GLenum id = GL_FLOAT;
		typedef stream<GLfloat> stream;
	};
	template <>
	struct type<GLdouble> {
		static const GLenum count = 1;
		static const GLenum id = GL_DOUBLE;
		typedef stream<GLdouble> stream;
	};


	//Integer
	template <>
	struct type<GLbyte> {
		static const GLenum count = 1;
		static const GLenum id = GL_BYTE;
	};
	template <>
	struct type<GLubyte> {
		static const GLenum count = 1;
		static const GLenum id = GL_UNSIGNED_BYTE;
	};
	template <>
	struct type<GLshort> {
		static const GLenum count = 1;
		static const GLenum id = GL_SHORT;
		typedef stream<GLshort> stream;
	};
	template <>
	struct type<GLushort> {
		static const GLenum count = 1;
		static const GLenum id = GL_UNSIGNED_SHORT;
		typedef stream<GLushort> stream;
	};
	template <>
	struct type<GLint> {
		static const GLenum count = 1;
		static const GLenum id = GL_INT;
		typedef stream<GLint> stream;
	};
	template <>
	struct type<GLuint> {
		static const GLenum count = 1;
		static const GLenum id = GL_UNSIGNED_INT;
		typedef stream<GLuint> stream;
	};

	//GLM Vector
	template <>
	struct type<glm::vec2> {
		static const GLenum count = 2;
		static const GLenum id = GL_FLOAT;
		typedef stream<glm::vec2> stream;
	};

	template <>
	struct type<glm::vec3> {
		static const GLenum count = 3;
		static const GLenum id = GL_FLOAT;
		typedef stream<glm::vec3> stream;
	};

	template <>
	struct type<glm::vec4> {
		static const GLenum count = 4;
		static const GLenum id = GL_FLOAT;
		typedef stream<glm::vec4> stream;
	};


	//GLM Matrix
	template <>
	struct type<glm::mat4x4> {
		static const GLenum count = 16;
		static const GLenum id = GL_FLOAT;
		typedef stream<glm::mat4x4> stream;
	};
}
