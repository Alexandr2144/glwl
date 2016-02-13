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
		_GLWL texture& texture;

		struct properties {
			glm::vec4 ambient;
			glm::vec4 diffuse;
			glm::vec4 specular;
			glm::vec4 emission;
			float shininess;
			float roughness;

			properties(glm::vec4&& _ambient = { 1, 1, 1, 1 }, 
				glm::vec4&& _diffuse = { 1, 1, 1, 1 },
				glm::vec4&& _specular = { 1, 1, 1, 1 },
				glm::vec4&& _emission = { 0, 0, 0, 0 },
				float _shininess = 0.0f,
				float _roughness = 0.5f)
					: ambient(_ambient), diffuse(_diffuse),
					specular(_specular), emission(_emission), 
					shininess(_shininess), roughness(_roughness) {}
		} prop;

		struct offsets {
			GLuint ambient;
			GLuint diffuse;
			GLuint specular;
			GLuint emission;
			GLuint shininess;
			GLuint roughness;

			void shift(GLuint offset) {
				ambient += offset, diffuse += offset,
				specular += offset, emission += offset,
				shininess += offset, roughness += offset;
			}
			static offsets default(_uniform ufrm) {
				offsets out;
				GLuint indices[6];
				ufrm.indices(indices, {
					"material.ambient", "material.diffuse",
					"material.specular", "material.emission",
					"material.shininess", "material.roughness" });
				ufrm.offsets(6, indices, (GLint*)&out);
				return out;
			}
		} offset;

		material(properties&& p, offsets&& off, _GLWL texture&& tex) 
			: prop(p), offset(off), texture(tex) {}

		void load(buf::ubo<>& ubuf, offsets off) {
			ubuf.write(off.ambient, prop.ambient);
			ubuf.write(off.diffuse, prop.diffuse);
			ubuf.write(off.specular, prop.specular);
			ubuf.write(off.emission, prop.emission);
			ubuf.write(off.shininess, prop.shininess);
			ubuf.write(off.roughness, prop.roughness);
		}
	};
}