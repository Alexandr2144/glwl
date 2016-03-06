#pragma once

#include "buffer.h"

namespace glwl {
	template <class ExceptionPolicy>
	class unbindable : public ExceptionPolicy {
	public:
		void bind(GLenum type) const {}
		GLuint id() const { return _id; }
	protected:
		unbindable(_STD _Uninitialized) {}
		unbindable() { glGenTextures(1, &_id); }
		~unbindable() { glDeleteTextures(1, &_id); }
	private:
		GLuint _id;
	};

	template <class ExceptionPolicy>
	class bindable : public unbindable<ExceptionPolicy> {
	public:
		void bind(GLenum type) const { ExceptionPolicy::call(glBindTexture, type, id()); }
	protected:
		bindable(_STD _Uninitialized) : unbindable(_STD _Noinit) {}
		bindable() {}
	};

	template <class ExceptionPolicy, template <class> class BindPolicy = bindable>
	class basic_texture : public BindPolicy<ExceptionPolicy> {
	private:
		typedef ExceptionPolicy checker;
	public:
		basic_texture(_STD _Uninitialized) {}

		void set_filter(GLenum type, GLenum min_filter, GLenum mag_filter) const {
			checker::call(glTexParameteri, type, GL_TEXTURE_MIN_FILTER, min_filter);
			checker::call(glTexParameteri, type, GL_TEXTURE_MAG_FILTER, mag_filter);
		}

		void reserve_tex2d(GLenum type, GLuint level, GLuint width, GLuint height,
				GLenum internal_format, GLenum pixel_format, GLenum pixel_type,
				const void* data = nullptr, GLint border = NULL) {
			checker::call(glTexImage2D, type, level, internal_format, 
				width, height, border, pixel_format, pixel_type, data);
		}

		void reserve_compressed2d(GLenum type, GLuint level, GLuint width, GLuint height, 
				GLenum internal_format, GLuint img_size,
				const void* data = nullptr, GLint border = NULL) {
			checker::call(glCompressedTexImage2D, type, level, internal_format,
				width, height, border, img_size, data);
		}
	};

	template <class ExceptionPolicy>
	class texture : public basic_texture<ExceptionPolicy> {
	private:
		typedef basic_texture<ExceptionPolicy> base;
	public:
		class white_1x1 : public texture<ExceptionPolicy> {
		public:
			white_1x1(GLenum minmag_filter = GL_LINEAR) 
				: white_1x1(minmag_filter, minmag_filter) {}
			white_1x1(GLenum min_filter, GLenum mag_filter) 
				: texture(GL_TEXTURE_2D, min_filter, mag_filter) {
				reserve_tex2d(GL_TEXTURE_2D, 0, 1, 1, GL_RGBA, GL_RGBA, GL_FLOAT, &glm::vec4(1, 1, 1, 1));
			}
		};

		texture(_STD _Uninitialized, GLenum type) : basic_texture(_STD _Noinit), _type(type) {}
		texture(GLenum type, GLenum minmag_filter = GL_LINEAR) 
			: texture(type, minmag_filter, minmag_filter) {}
		texture(GLenum type, GLenum min_filter, GLenum mag_filter) 
			: texture(_STD _Noinit, type) 
		{
			base::bind(_type);
			base::set_filter(_type, min_filter, mag_filter);
		}
		texture(GLenum type, GLuint width, GLuint height, GLenum min_filter, GLenum mag_filter)
			: texture(type, min_filter, mag_filter)
		{
			_width = width;
			_height = height;
			bind();
		}

		void bind() const { base::bind(_type); }
		void set_filter(GLenum min_filter, GLenum mag_filter) const {
			base::set_filter(_type, min_filter, mag_filter); }
	private:
		GLenum _type;
		GLenum _ptype;
		GLenum _pform;
		GLuint _width;
		GLuint _height;
	};

	class material {
	public:
		struct properties {
			glm::vec4 ambient;
			glm::vec4 diffuse;
			glm::vec4 specular;
			glm::vec4 emission;
			float shininess;
			float roughness;
			float solid;

			properties(const glm::vec4& _ambient = { 1, 1, 1, 1 },
				const glm::vec4& _diffuse = { 1, 1, 1, 1 },
				const glm::vec4& _specular = { 1, 1, 1, 1 },
				const glm::vec4& _emission = { 0, 0, 0, 0 },
				float _shininess = 0.0f,
				float _roughness = 0.5f,
				float _solid = 1.0f)
					: ambient(_ambient), diffuse(_diffuse),
					specular(_specular), emission(_emission), 
					shininess(_shininess), roughness(_roughness),
					solid(_solid) {}
		} prop;

		struct offsets {
			GLuint ambient;
			GLuint diffuse;
			GLuint specular;
			GLuint emission;
			GLuint shininess;
			GLuint roughness;
			GLuint solid;
		} offset;

		material(properties&& p, offsets&& off)
			: prop(p), offset(off) {}
		material(properties&& p, const uniform& ufrm)
			: prop(p) {
			GLuint indices[7];
			ufrm.indices(indices, {
				"material.ambient", "material.diffuse",
				"material.specular", "material.emission",
				"material.shininess", "material.roughness",
				"material.solid" });
			ufrm.offsets(7, indices, (GLint*)&offset);
		}
		material(properties&& p, const uniform& ufrm, const char* name)
			: prop(p) {
			GLuint indices[7];
			char sz1[128]; strcpy_s(sz1, 128, name); strcat_s(sz1, ".ambient");
			char sz2[128]; strcpy_s(sz2, 128, name); strcat_s(sz2, ".diffuse");
			char sz3[128]; strcpy_s(sz3, 128, name); strcat_s(sz3, ".specular");
			char sz4[128]; strcpy_s(sz4, 128, name); strcat_s(sz4, ".emission");
			char sz5[128]; strcpy_s(sz5, 128, name); strcat_s(sz5, ".shininess");
			char sz6[128]; strcpy_s(sz6, 128, name); strcat_s(sz6, ".roughness");
			char sz7[128]; strcpy_s(sz7, 128, name); strcat_s(sz7, ".solid");
			ufrm.indices(indices, { sz1, sz2, sz3, sz4, sz5, sz6, sz7 });
			ufrm.offsets(7, indices, (GLint*)&offset);
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
			os.unsafe::write(out.offset.solid, sizeof(out.prop.solid), (char*)&out.prop.solid);
			os.unsafe::save();
		} else {
			GLuint pos = os.tell();
			os.shift(out.offset.ambient); os.write(1, &out.prop.ambient);
			os.seek(pos); os.shift(out.offset.diffuse); os.write(1, (char*)&out.prop.diffuse);
			os.seek(pos); os.shift(out.offset.specular); os.write(1, (char*)&out.prop.specular);
			os.seek(pos); os.shift(out.offset.emission); os.write(1, (char*)&out.prop.emission);
			os.seek(pos); os.shift(out.offset.shininess); os.write(1, (char*)&out.prop.shininess);
			os.seek(pos); os.shift(out.offset.roughness); os.write(1, (char*)&out.prop.roughness);
			os.seek(pos); os.shift(out.offset.solid); os.write(1, (char*)&out.prop.solid);
		} return os;
	}
}