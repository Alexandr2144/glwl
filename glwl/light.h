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
			} prop;
			struct offsets {
				GLuint camera;
				GLuint ambient;
				void shift(GLuint offset) {
					camera += offset, ambient += offset; }
				static offsets default(_uniform ufrm) {
					offsets out;
					GLuint indices[2];
					ufrm.indices(indices, { "light.camera", "light.ambient" });
					ufrm.offsets(2, indices, (GLint*)&out);
					return out;
				}
			} offset;

			global(properties&& p, offsets&& off)
				: prop(p), offset(off) {}

			void load(buf::ubo<>& ub) const {
				ub.write(offset.camera, prop.camera);
				ub.write(offset.ambient, prop.ambient);
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
				void shift(GLuint offset) {
					enable += offset, attenuation += offset,
						position += offset, color += offset;
				}
				static offsets default(_uniform& ufrm) {
					offsets out;
					GLuint indices[4];
					ufrm.indices(indices, {
						"light.point[0].color", "light.point[0].enable",
						"light.point[0].position", "light.point[0].attenuation" });
					ufrm.offsets(4, indices, (GLint*)&out);
					return out;
				}
			} offset;

			point(properties&& p, offsets&& off) 
				: prop(p), offset(off) {}

			void toggle() { prop.enable = !prop.enable; }
			void load(buf::ubo<>& ub) const {
				ub.write(offset.color, prop.color);
				ub.write(offset.enable, prop.enable);
				ub.write(offset.position, prop.position);
				ub.write(offset.attenuation, prop.attenuation);
			}
		};
	};
};