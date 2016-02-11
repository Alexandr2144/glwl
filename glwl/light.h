#include "buffer.h"

#include <glm\vec3.hpp>

namespace glwl {
	struct light_point {
		int enable;
		glm::vec4 color;
		glm::vec3 position;
		glm::vec3 attenuation;

		light_point(int on = 0, glm::vec4 colr = { 0, 0, 0, 0 }, glm::vec3 pos = { 0, 0, 0 }, glm::vec3 att = { 1, 0, 0 })
			: enable(on), color(colr), position(pos), attenuation(att) {}
		void toggle() { enable = !enable; }
	};

	class light {
	public:
		static const GLuint max = 10;

		light_point point[max];

		struct spot {
			bool enable;
			glm::vec4 position;
			glm::vec4 direction;
			glm::vec4 color;
			float brightness;
			float distance;
			float angle;
		} spot[max];

		light(const glm::vec3& cam_pos, uniform& cbuf) : _cbuf(cbuf), _cam(cam_pos) { _init(); }

		void update() {
			_cbuf.bind();
			_cbuf.write(_offset.camera, _cam);
			_cbuf.write(_offset.ambient, ambient);

			GLuint spot_offset = 0;
			GLuint point_offset = 0;
			for (GLuint i = 0; i < max; i++) {
				_cbuf.write(point_offset + _offset.point_color, point[i].color);
				_cbuf.write(point_offset + _offset.point_enable, point[i].enable);
				_cbuf.write(point_offset + _offset.point_position, point[i].position);
				_cbuf.write(point_offset + _offset.point_attenuation, point[i].attenuation);
				point_offset += _offset.point_stride;
			}
		}

		glm::vec4 ambient;
	private:
		void _init() {
			_cbuf.bind();
			_offset.camera = _cbuf["light.camera"].offset();
			_offset.ambient = _cbuf["light.ambient"].offset();

			_offset.point_color = _cbuf["light.point[0].color"].offset();
			_offset.point_enable = _cbuf["light.point[0].enable"].offset();
			_offset.point_position = _cbuf["light.point[0].position"].offset();
			_offset.point_attenuation = _cbuf["light.point[0].attenuation"].offset();
			_offset.point_stride = _cbuf["light.point[1].color"].offset() - _offset.point_color;
		}

		const glm::vec3& _cam;
		uniform& _cbuf;

		struct {
			GLuint ambient;
			GLuint camera;
			GLuint point_color;
			GLuint point_enable;
			GLuint point_position;
			GLuint point_attenuation;
			GLuint point_stride;
		} _offset;
	};
}