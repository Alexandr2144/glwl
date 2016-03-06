#pragma once

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <glm\glm.hpp>

#include "traits.h"
#include "buffer.h"

namespace glwl {
	namespace a {
		struct no_check {
		public:
			enum code { sucess };

			GLenum glerr() { return glGetError(); }
			code error() { return sucess; }
			code exception(code mask) { return sucess; }
			code exception() { return sucess; }
		protected:
			template <typename FTy, typename... ArgsTy>
			inline GLuint construct(FTy f, ArgsTy... args) { return f(args...); }
			template <typename FTy, typename... ArgsTy>
			inline void destruct(FTy f, ArgsTy... args) { f(args...); }

			template <typename FTy, typename... ArgsTy>
			inline void call(FTy f, ArgsTy... args) const { f(args...); }
			template <typename... MsgTypes>
			inline void check(bool err, MsgTypes... msg) const {}

			no_check() {}
			~no_check() {}
		};

		struct std_check {
		public:
			enum code {
				sucess = 0x0000,
				fail = 0xffff,

				bad_glShaderSource = 0x0001,
				bad_glCompileShader = 0x0002,
				bad_glGetShaderInfoLog = 0x0004,
				bad_glDeleteShader = 0x0008,
				bad_glGetShaderiv = 0x0010,
				bad_glCreateShader = 0x0020,
				bad_glCreateProgram = 0x0040,

				bad_glAttachShader = 0x0100,
				bad_glDetachShader = 0x0200,
				bad_glUseProgram = 0x0400,
				bad_glLinkProgram = 0x0800,
				bad_glGetProgramiv = 0x1000,
				bad_glGetProgramInfoLog = 0x2000,

				none = 0x00000000,
				all = 0xffffffff
			};

			GLenum glerr() const { return _glerr; }
			code error() const { return _error; }
			code exception() const { return _mask; }
			code exception(code mask) const { code old = _mask; _mask = mask; return old; }

		protected:
			template <typename FTy>
			inline GLuint construct(FTy f) {
				GLuint id = f();
				_glerr = glGetError();
				if (_glerr) {
					_error = bad_glCreateProgram;
					if (_mask & _error) throw _GLWL exception("glCreateShader\
						function failure. \nError message : %s", macro::str(_glerr));
				} return id;
			}
			template <typename FTy, typename ATy>
			inline GLuint construct(FTy f, ATy a1) {
				GLuint id = f(a1);
				_glerr = glGetError();
				if (_glerr) {
					_error = bad_glCreateShader;
					if (_mask & _error) throw _GLWL exception("glCreateShader\
						function failure. Type: %s \nError message : %s",
						macro::str(a1), macro::str(_glerr));
				} return id;
			}
			template <typename FTy, typename... ArgsTy>
			inline void destruct(FTy f, ArgsTy... args) { f(args...); }

#define GLWL_BUF_THROW(proc, format) \
	case bad_##proc: throw _GLWL exception(#proc##" function failure. Arguments: {\n"##format##"\n} \nError message : %s"
#define GLWL_BUF_EXCEPT_CALL_BEG \
	_glerr = glGetError(); \
	if (_glerr) { \
	setf(reinterpret_cast<const void*>(f)); \
	if (!(_mask & _error)) return; \
	switch (_error) {
#define GLWL_BUF_EXCEPT_CALL_END }}

				template <typename FTy, typename A1Ty>
				inline void call(FTy f, A1Ty a1) const {
					f(a1);
					GLWL_BUF_EXCEPT_CALL_BEG
					GLWL_BUF_THROW(glCompileShader, "    ID: %d"),
						a1, macro::str(_glerr));
					GLWL_BUF_THROW(glDeleteShader, "    ID: %d"),
						a1, macro::str(_glerr));
					GLWL_BUF_EXCEPT_CALL_END
				}
				template <typename FTy, typename A1Ty, typename A2Ty>
				inline void call(FTy f, A1Ty a1, A2Ty a2) const {
					f(a1, a2);
					GLWL_BUF_EXCEPT_CALL_BEG
					GLWL_BUF_THROW(glAttachShader, "    ProgramID: %d,\n    ShaderID: %d"),
						a1, a2, macro::str(_glerr));
					GLWL_BUF_THROW(glDetachShader, "    ProgramID: %d,\n    ShaderID: %d"),
						a1, a2, macro::str(_glerr));
					GLWL_BUF_EXCEPT_CALL_END
				}
				template <typename FTy, typename A1Ty, typename A2Ty, typename A3Ty>
				inline void call(FTy f, A1Ty a1, A2Ty a2, A3Ty a3) const {
					f(a1, a2, a3);
					GLWL_BUF_EXCEPT_CALL_BEG
					GLWL_BUF_THROW(glGetShaderiv, "    ID: %d,\n    ParamName: %s,\n    Data: %x"),
						a1, macro::str(a2), a3, macro::str(_glerr));
					GLWL_BUF_EXCEPT_CALL_END
				}
				template <typename FTy, typename A1Ty, typename A2Ty, typename A3Ty, typename A4Ty>
				inline void call(FTy f, A1Ty a1, A2Ty a2, A3Ty a3, A4Ty a4) const {
					f(a1, a2, a3, a4);
					GLWL_BUF_EXCEPT_CALL_BEG
					GLWL_BUF_THROW(glShaderSource, "    ID: %d,\n    Count: %d,\n\tData: %x,\n    Length: %d"),
						a1, a2, a3, a4, macro::str(_glerr));
					GLWL_BUF_THROW(glGetShaderInfoLog, "    ID: %d,\n    BufSize: %d,\n    Length: %d,\n    Data: %x"),
						a1, a2, a3, a4, macro::str(_glerr));
					GLWL_BUF_EXCEPT_CALL_END
				}

#undef GLWL_BUF_THROW
#undef GLWL_BUF_EXCEPT_CALL_BEG
#undef GLWL_BUF_EXCEPT_CALL_END

				template <typename... MsgTypes>
				inline void check(bool err, MsgTypes... msg) const {
					if (err) throw _GLWL exception(msg...); }

				std_check() : _mask(all), _error(sucess), _glerr(GL_NO_ERROR) {}
				~std_check() {}
		private:
			inline void setf(const void* f) const {
				if (f == glShaderSource)		_error = bad_glShaderSource;
				else if (f == glCompileShader)		_error = bad_glCompileShader;
				else if (f == glGetShaderInfoLog)	_error = bad_glGetShaderInfoLog;
				else if (f == glDeleteShader)		_error = bad_glDeleteShader;
				else if (f == glGetShaderiv)		_error = bad_glGetShaderiv;
				else if (f == glCreateShader)		_error = bad_glCreateShader;

				else if (f == glCreateProgram)		_error = bad_glCompileShader;
				else if (f == glDeleteProgram)		_error = bad_glGetShaderInfoLog;
				else if (f == glAttachShader)		_error = bad_glAttachShader;
				else if (f == glDetachShader)		_error = bad_glDetachShader;
				else if (f == glUseProgram)			_error = bad_glUseProgram;
				else if (f == glLinkProgram)		_error = bad_glLinkProgram;
				else if (f == glGetProgramiv)		_error = bad_glGetProgramiv;
				else if (f == glGetProgramInfoLog)	_error = bad_glGetProgramInfoLog;
				else _error = fail;
			}

			mutable GLenum _glerr;
			mutable code _mask;
			mutable code _error;
		};
	};

	struct log {
		char* msg;
		~log() { free(msg); }
		log() : msg(nullptr) {}
		void create(GLuint size) { msg = (char*)malloc(size); }
	};

	template <class ExceptionsPolicy = a::std_check >
	class shader : public ExceptionsPolicy {
	private:
		typedef ExceptionsPolicy err;
	public:
		class program : ExceptionsPolicy {
		public:
			program(_STD _Uninitialized) : _id(NULL) {}
			program() : program(_STD _Noinit) { _id = err::construct(glCreateProgram); }
			~program() { err::destruct(glDeleteProgram, _id); }

			program(program&& src) : unknown(_STD forward<unknown>(src)) {}
			GLWL_ASSIG_RV_STD(program)

			program(const program& src) = delete;
			program& operator=(const program& src) = delete;
			
			void attach(GLuint shader_id) const { err::call(glAttachShader, _id, shader_id); }
			void detach(GLuint shader_id) const { err::call(glDetachShader, _id, shader_id); }
			void attach(const shader& shader) const { attach(shader.id()); }
			void detach(const shader& shader) const { detach(shader.id()); }

			program& operator<<(const shader& shader) { attach(shader.id()); return *this; }
			program& operator>>(const shader& shader) { detach(shader.id()); return *this; }
			const program& operator<<(const shader& shader) const { attach(shader.id()); return *this; }
			const program& operator>>(const shader& shader) const { detach(shader.id()); return *this; }

			GLuint id() const { return _id; }

			void use() const { err::call(glUseProgram, _id); }
			void link(bool throw_except = true) {
				GLint status;
				err::call(glLinkProgram, _id);
				err::call(glGetProgramiv, _id, GL_LINK_STATUS, &status);
				if (status == GL_FALSE) {
					GLint length;
					err::call(glGetProgramiv, _id, GL_INFO_LOG_LENGTH, &length);
					_log.create(length);
					err::call(glGetProgramInfoLog, _id, length, &length, _log.msg);
					err::check(true, _log.msg);
				}
			}
			inline const char* log() const { return _log.msg; }
		private:
			_GLWL log _log;
			GLuint _id;
		};

		shader() {}
		shader(GLenum type, const char* source) : shader() {
			_id = err::construct(glCreateShader, type);
			err::call(glShaderSource, _id, 1, &source, nullptr);

			GLint status;
			err::call(glCompileShader, _id);
			err::call(glGetShaderiv, _id, GL_COMPILE_STATUS, &status);
			if (status == GL_FALSE) {
				GLint length;
				err::call(glGetShaderiv, _id, GL_INFO_LOG_LENGTH, &length);
				_log.create(length);
				err::call(glGetShaderInfoLog, _id, length, &length, _log.msg);
				err::check(true, _log.msg);
			}
		}
		~shader() { if (_id) glDeleteShader(_id); }

		shader(const shader& src) = delete;
		shader& operator=(const shader& src) = delete;

		shader(shader&& src) : _id(src._id) { src._id = NULL; }
		GLWL_ASSIG_RV_STD(shader)

		GLuint id() const { return _id; }

		inline const char* log() const { return _log.msg; }
	private:
		_GLWL log _log;
		GLuint _id;
	};
}