#pragma once

#include "core.h"

namespace glwl {
	class unknown {
	public:
		~unknown() {}
		unknown() : unknown(NULL) {}
		unknown(GLuint id) : _id(id), _owner(true), _errmask(-1),
			_errfl(0) {}

		unknown(unknown&& src) : _id(src._id) { src._id = NULL; }
		unknown(const unknown& src) : _id(src._id), _owner(false) {}

		inline void exception(GLuint mask) {
			_errmask = mask; }
		inline GLuint exception() const { return _errmask; }
		inline GLuint error() const { return _errfl; }

		inline bool isref() const { return !_owner; }
		inline bool good() const { return _errfl == 0; }
		inline bool bad() const { return _errfl != 0; }

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
		GLuint _errmask;
		mutable GLuint _errfl;
	};

	template <typename ValueTy>
	struct format {
		GLboolean norm;
		format(GLboolean normalize = default) : norm(normalize) {}
		enum {
			normalized = GL_TRUE,
			default = GL_FALSE
		};
	};

	//Prototype
	template <typename ValueTy>
	struct type {};

	//Float
	template <>
	struct type<GLfloat> {
		static const GLenum count = 1;
		static const GLenum id = GL_FLOAT;
	};
	template <>
	struct type<GLdouble> {
		static const GLenum count = 1;
		static const GLenum id = GL_DOUBLE;
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
	};
	template <>
	struct type<GLushort> {
		static const GLenum count = 1;
		static const GLenum id = GL_UNSIGNED_SHORT;
	};
	template <>
	struct type<GLint> {
		static const GLenum count = 1;
		static const GLenum id = GL_INT;
	};
	template <>
	struct type<GLuint> {
		static const GLenum count = 1;
		static const GLenum id = GL_UNSIGNED_INT;
	};

	//GLM Vector
	template <>
	struct type<glm::vec2> {
		static const GLenum count = 2;
		static const GLenum id = GL_FLOAT;
	};

	template <>
	struct type<glm::vec3> {
		static const GLenum count = 3;
		static const GLenum id = GL_FLOAT;
	};

	template <>
	struct type<glm::vec4> {
		static const GLenum count = 4;
		static const GLenum id = GL_FLOAT;
	};


	//GLM Matrix
	template <>
	struct type<glm::mat4x4> {
		static const GLenum count = 16;
		static const GLenum id = GL_FLOAT;
	};
}
