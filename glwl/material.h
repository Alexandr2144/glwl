#pragma once

#include "buffer.h"

namespace glwl {
	struct pixel {
		GLuint n;
		pixel(GLuint format) {
			switch (format) {
			case GL_RGB: n = 3; break;
			}
		}
	};

	class texture : public unknown {
	public:
		texture() {}
		texture(GLenum type) : _type(type) { glGenTextures(1, &_id); }
		texture(GLenum type, GLenum min_filter = GL_LINEAR, GLenum mag_filter = GL_LINEAR) 
			: texture(type, 0, 0, min_filter, mag_filter) {}
		texture(GLenum type, GLuint width, GLuint height, GLenum min_filter = GL_LINEAR, GLenum mag_filter = GL_LINEAR) 
			: _type(type), _width(width), _height(height) {
			glGenTextures(1, &_id);
			glBindTexture(type, _id);
			glTexParameteri(type, GL_TEXTURE_MAG_FILTER, mag_filter);
			glTexParameteri(type, GL_TEXTURE_MIN_FILTER, min_filter);
		}

		void filter(GLenum min_filter, GLenum mag_filter) {
			glTexParameteri(_type, GL_TEXTURE_MIN_FILTER, min_filter);
			glTexParameteri(_type, GL_TEXTURE_MAG_FILTER, mag_filter);
		}

		virtual void reserve(GLenum type, GLuint width, GLuint height, GLenum pixel_type,
			GLenum pixel_format = GL_RGB, const void* data = nullptr) {
			glTexImage2D(type, 0, pixel(_pform).n, width, height, 0, pixel_format, pixel_type, data);
			_type = type, _ptype = pixel_type, _pform = pixel_format, _width = width, _height = height;
		}
		virtual void reserve(GLuint width, GLuint height, const void* data = nullptr) {
			glTexImage2D(_type, 0, pixel(_pform).n, width, height, 0, _pform, _ptype, data);
			_width = width, height = _height;
		}
	private:
		GLenum _type;
		GLenum _ptype;
		GLenum _pform;
		GLuint _width;
		GLuint _height;
	};

	class texture2D : public texture {
	public:
		texture2D() {}
		texture2D(int) {}
	};

	class material {
	public:
		glwl::texture& texture;
		glm::vec4 ambient = { 1, 1, 1, 1 };
		glm::vec4 diffuse = { 1, 1, 1, 1 };
		glm::vec4 specular = { 1, 1, 1, 1 };
		glm::vec4 emission = { 0, 0, 0, 0 };
		float shininess = 0.0f;
		float roughness = 0.5f;

		material(uniform& cbuf, glwl::texture& tex) : texture(tex), _cbuf(cbuf) { _init(); }

		void update() {
			_cbuf.bind();
			_cbuf.write(_offset.ambient, ambient);
			_cbuf.write(_offset.diffuse, diffuse);
			_cbuf.write(_offset.specular, specular);
			_cbuf.write(_offset.emission, emission);
			_cbuf.write(_offset.shininess, shininess);
			_cbuf.write(_offset.roughness, roughness);
		}
	private:
		void _init() {
			_cbuf.bind();
			_offset.ambient = _cbuf.require("material.ambient").offset;
			_offset.diffuse = _cbuf.require("material.diffuse").offset;
			_offset.specular = _cbuf.require("material.specular").offset;
			_offset.emission = _cbuf.require("material.emission").offset;
			_offset.shininess = _cbuf.require("material.shininess").offset;
			_offset.roughness = _cbuf.require("material.roughness").offset;
		}

		uniform& _cbuf;
		struct {
			GLuint ambient;
			GLuint diffuse;
			GLuint specular;
			GLuint emission;
			GLuint shininess;
			GLuint roughness;
		} _offset;
	};
}