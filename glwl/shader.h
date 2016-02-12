#pragma once

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <glm\glm.hpp>

namespace glwl {
	class shader : public unknown {
	public:
		enum src_type { bin, text, file_bin, file_text };

		shader() : _log(nullptr) {}
		shader(GLenum type, const char* source, bool throw_except = true) {
			_id = glCreateShader(type);
			_check(throw_except, "Can't create shader");
			glShaderSource(_id, 1, &source, nullptr);
			_check(throw_except, "Can't create shader source object");

			GLint status;
			glCompileShader(_id);
			glGetShaderiv(_id, GL_COMPILE_STATUS, &status);
			if (status == GL_FALSE) {
				GLint length;
				glGetShaderiv(_id, GL_INFO_LOG_LENGTH, &length);
				_log = blob(length, nullptr);
				glGetShaderInfoLog(_id, length, &length, (char*)_log.data());
				if (throw_except) throw _GLWL exception((char*)_log.data());
			}
		}
		~shader() { if (_id) glDeleteShader(_id); }

		shader(const shader& src) = delete;
		shader& operator=(const shader& src) = delete;

		shader(shader&& src) : unknown(_STD forward<unknown>(src)) {}
		GLWL_ASSIG_RV_STD(shader)

		inline const char* log() const { return (const char*)_log.data(); }
	private:
		blob _log;
		inline void _check(bool throw_except, const char* msg) const {
			GLenum error = glGetError();
			if (error && throw_except) throw _GLWL exception("%s. [code: %d]", msg, error);
		}
	};

	class vshader : public shader {
	public:
		vshader() {}
		vshader(const char* source) : shader(GL_VERTEX_SHADER, source) {}
		vshader(vshader&& src) : shader(_STD forward<shader>(src)) {}
		GLWL_ASSIG_RV_STD(vshader)

		vshader(const vshader& src) = delete;
		vshader& operator=(const vshader& src) = delete;
	};

	class pshader : public shader {
	public:
		pshader() {}
		pshader(const char* source) : shader(GL_FRAGMENT_SHADER, source) {}
		pshader(pshader&& src) : shader(_STD forward<shader>(src)) {}
		GLWL_ASSIG_RV_STD(pshader)

		pshader(const pshader& src) = delete;
		pshader& operator=(const pshader& src) = delete;
	};

	class shader_program : public unknown {
	public:
		shader_program() { _id = glCreateProgram(); }
		~shader_program() { unknown::dtor(glDeleteProgram); }

		shader_program(shader_program&& src) : unknown(_STD forward<unknown>(src)) {}
		GLWL_ASSIG_RV_STD(shader_program)

		shader_program(const shader_program& src) : unknown(src) {}
		GLWL_ASSIG_LV_STD(shader_program)

		void attach(GLuint shader_id) const {
			glAttachShader(_id, shader_id);
			_check("Can't attach shader");
		}
		void detach(GLuint shader_id) const {
			glDetachShader(_id, shader_id);
			_check("Can't detach shader");
		}

		shader_program& operator<<(const shader& shader) { attach(shader.id()); return *this; }
		shader_program& operator>>(const shader& shader) { detach(shader.id()); return *this; }
		const shader_program& operator<<(const shader& shader) const { attach(shader.id()); return *this; }
		const shader_program& operator>>(const shader& shader) const { detach(shader.id()); return *this; }

		void attach(const shader& shader) const { attach(shader.id()); }
		void detach(const shader& shader) const { detach(shader.id()); }

		GLuint uniform_location(const GLchar* name) {
			GLuint loc = glGetUniformLocation(_id, name);
			_check("Can't get uniform location");
			return loc;
		}

		void uniform(GLuint location, const glm::mat4& value) const {
			glUniformMatrix4fv(location, 1, GL_FALSE, &value[0].x);
			_check("Can't set uniform object");
		}

		_GLWL uniform uniform(const GLchar* name) const { return _GLWL uniform(_id, name); }

		void bind() const { 
			glUseProgram(_id);
			_check("Can't using this program");
		}
		void link(bool throw_except = true) {
			GLint status;
			glLinkProgram(_id);
			glGetProgramiv(_id, GL_LINK_STATUS, &status);
			if (status == GL_FALSE) {
				GLint length;
				glGetProgramiv(_id, GL_INFO_LOG_LENGTH, &length);
				_log = blob(length, nullptr);
				glGetProgramInfoLog(_id, length, &length, (char*)_log.data());
				//if (throw_except) throw glwl::exception(log());
			}
		}
		inline const char* log() const { return (const char*)_log.data(); }
	private:
		blob _log;
		inline void _check(const char* msg) const {
			GLenum error = glGetError();
			//if (error) throw glwl::exception("%s. [id: %d, code: %d]", msg, _id, error);
		}
	};
}