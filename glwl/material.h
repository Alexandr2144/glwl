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
		} offset;

		material(_GLWL texture&& tex, properties&& p, offsets&& off)
			: prop(p), offset(off), texture(tex) {}
		material(_GLWL texture&& tex, properties&& p, uniform ufrm)
			: prop(p), texture(tex)  {
			GLuint indices[6];
			ufrm.indices(indices, {
				"material.ambient", "material.diffuse",
				"material.specular", "material.emission",
				"material.shininess", "material.roughness" });
			ufrm.offsets(6, indices, (GLint*)&offset);
		}
		material(_GLWL texture&& tex, properties&& p, uniform ufrm, const char* name)
			: prop(p), texture(tex) {
			GLuint indices[6];
			char sz1[128]; strcpy_s(sz1, 128, name); strcat_s(sz1, ".ambient");
			char sz2[128]; strcpy_s(sz2, 128, name); strcat_s(sz2, ".diffuse");
			char sz3[128]; strcpy_s(sz3, 128, name); strcat_s(sz3, ".specular");
			char sz4[128]; strcpy_s(sz4, 128, name); strcat_s(sz4, ".emission");
			char sz5[128]; strcpy_s(sz5, 128, name); strcat_s(sz5, ".shininess");
			char sz6[128]; strcpy_s(sz6, 128, name); strcat_s(sz6, ".roughness");
			ufrm.indices(indices, { sz1, sz2, sz3, sz4, sz5, sz6 });
			ufrm.offsets(6, indices, (GLint*)&offset);
		}
	};

	template <class BufferTy, class CachePolicy,
		template <class> class BindPolicy>
	buf::stream<BufferTy, CachePolicy, BindPolicy>& operator<<(
		buf::stream<BufferTy, CachePolicy, BindPolicy>& os, const material& out) {
		if (os.cache_capacity() >= sizeof(out.prop)) {
			os.unsafe::write(out.offset.ambient, sizeof(out.prop.ambient), (char*)&out.prop.ambient);
			os.unsafe::write(out.offset.diffuse, sizeof(out.prop.diffuse), (char*)&out.prop.diffuse);
			os.unsafe::write(out.offset.specular, sizeof(out.prop.specular), (char*)&out.prop.specular);
			os.unsafe::write(out.offset.emission, sizeof(out.prop.emission), (char*)&out.prop.emission);
			os.unsafe::write(out.offset.shininess, sizeof(out.prop.shininess), (char*)&out.prop.shininess);
			os.unsafe::write(out.offset.roughness, sizeof(out.prop.roughness), (char*)&out.prop.roughness);
			os.unsafe::save();
		} else {
			GLuint pos = os.tell();
			os.shift(out.offset.ambient); os.write(1, &out.prop.ambient);
			os.seek(pos); os.shift(out.offset.diffuse); os.write(1, (char*)&out.prop.diffuse);
			os.seek(pos); os.shift(out.offset.specular); os.write(1, (char*)&out.prop.specular);
			os.seek(pos); os.shift(out.offset.emission); os.write(1, (char*)&out.prop.emission);
			os.seek(pos); os.shift(out.offset.shininess); os.write(1, (char*)&out.prop.shininess);
			os.seek(pos); os.shift(out.offset.roughness); os.write(1, (char*)&out.prop.roughness);
		} return os;
	}
}