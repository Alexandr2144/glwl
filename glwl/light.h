//#include "buffer.h"

#include <glm\vec3.hpp>

namespace glwl {
	namespace light {
		//light_point point[max];

		/*struct spot {
			bool enable;
			glm::vec4 position;
			glm::vec4 direction;
			glm::vec4 color;
			float brightness;
			float distance;
			float angle;
		} spot[max];*/

		struct global {
			struct properties {
				const glm::vec3* camera;
				glm::vec4 ambient;
				GLuint point_cnt;
			} prop;
			struct offsets {
				GLuint camera;
				GLuint ambient;
				GLuint point_cnt;
			} offset;

			global(properties&& p, offsets&& off) : prop(p), offset(off) {}
			global(properties&& p, const uniform& ufrm, const char* name)
				: prop(p) {
				GLuint indices[3];
				char sz1[128]; strcpy_s(sz1, 128, name); strcat_s(sz1, ".camera");
				char sz2[128]; strcpy_s(sz2, 128, name); strcat_s(sz2, ".ambient");
				char sz3[128]; strcpy_s(sz3, 128, name); strcat_s(sz3, ".point_cnt");
				ufrm.indices(indices, { sz1, sz2, sz3 });
				ufrm.offsets(3, indices, (GLint*)&offset);
			}
		};

		struct point {
			struct properties {
				int enable;
				glm::vec4 color;
				glm::vec3 position;
				glm::vec3 attenuation;
			} prop;
			struct offsets {
				GLuint color;
				GLuint enable;
				GLuint position;
				GLuint attenuation;
			} offset;

			point(properties&& p, offsets&& off) 
				: prop(p), offset(off) {}
			point(properties&& p, const uniform& ufrm, const char* name)
				: prop(p) {
				GLuint indices[4];
				char sz1[128]; strcpy_s(sz1, 128, name); strcat_s(sz1, ".color");
				char sz2[128]; strcpy_s(sz2, 128, name); strcat_s(sz2, ".enable");
				char sz3[128]; strcpy_s(sz3, 128, name); strcat_s(sz3, ".position");
				char sz4[128]; strcpy_s(sz4, 128, name); strcat_s(sz4, ".attenuation");
				ufrm.indices(indices, { sz1, sz2, sz3, sz4 });
				ufrm.offsets(4, indices, (GLint*)&offset);
			}

			void toggle() { prop.enable = !prop.enable; }
		};

		template <class BufferTy, class CachePolicy, template <class> class BindPolicy>
		buf::stream<BufferTy, CachePolicy, BindPolicy>& operator<<(
			buf::stream<BufferTy, CachePolicy, BindPolicy>& os, const global& out) {
			if (os.cache_capacity() >= sizeof(out.prop)) {
				os.unsafe::write(out.offset.camera, sizeof(*out.prop.camera), (char*)out.prop.camera);
				os.unsafe::write(out.offset.ambient, sizeof(out.prop.ambient), (char*)&out.prop.ambient);
				os.unsafe::write(out.offset.point_cnt, sizeof(out.prop.point_cnt), (char*)&out.prop.point_cnt);
				os.unsafe::save();
			}
			else {
				GLuint pos = os.tell();
				os.shift(out.offset.camera); os.write(1, &out.prop.camera);
				os.seek(pos); os.shift(out.offset.ambient); os.write(1, (char*)&out.prop.ambient);
				os.seek(pos); os.shift(out.offset.point_cnt); os.write(1, (char*)&out.prop.point_cnt);
			} return os;
		}

		template <class BufferTy, class CachePolicy, template <class> class BindPolicy>
		buf::stream<BufferTy, CachePolicy, BindPolicy>& operator<<(
			buf::stream<BufferTy, CachePolicy, BindPolicy>& os, const point& out) {
			if (os.cache_capacity() >= sizeof(out.prop)) {
				os.unsafe::write(out.offset.color, sizeof(out.prop.color), (char*)&out.prop.color);
				os.unsafe::write(out.offset.enable, sizeof(out.prop.enable), (char*)&out.prop.enable);
				os.unsafe::write(out.offset.position, sizeof(out.prop.position), (char*)&out.prop.position);
				os.unsafe::write(out.offset.attenuation, sizeof(out.prop.attenuation), (char*)&out.prop.attenuation);
				os.unsafe::save();
			}
			else {
				GLuint pos = os.tell();
				os.shift(out.offset.color); os.write(1, &out.prop.color);
				os.seek(pos); os.shift(out.offset.enable); os.write(1, (char*)&out.prop.enable);
				os.seek(pos); os.shift(out.offset.position); os.write(1, (char*)&out.prop.position);
				os.seek(pos); os.shift(out.offset.attenuation); os.write(1, (char*)&out.prop.attenuation);
			} return os;
		}
	};
};