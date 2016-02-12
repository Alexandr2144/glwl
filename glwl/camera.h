#include <glm\mat4x4.hpp>
#include <glm\vec4.hpp>

#include <glm\gtc\matrix_transform.hpp>

#include "buffer.h"

namespace glwl {
	class camera {
	public:
		camera(buf::ubo<>::part cbuf, GLint offset)
			: _cbuf(cbuf), _offset(offset) { reset(); }

		void reset() {
			_pos = glm::vec3(0, 0, 0);
			_forward = glm::vec3(0, 0, 1);
			_right = glm::vec3(1, 0, 0);
			update();
		}

		void update() const { _cbuf.bind(); _cbuf.write(_offset, _view); }

		void spawn(glm::vec3 pos) { _pos = pos; update(); }

		void move_forward(float speed)  { _pos += speed*_forward; _update(); }
		void move_back(float speed)		{ _pos -= speed*_forward; _update(); }
		void move_right(float speed)	{ _pos += speed*_right; _update(); }
		void move_left(float speed)		{ _pos -= speed*_right; _update(); }
		//void move_down(float speed)		{ _pos = glm::translate(_mov, glm::vec3(0, 0, 0)); }
		//void move_up(float speed)		{ _pos = glm::translate(_mov, glm::vec3(0, 0, 0)); }

		void look_up(float angle) { _rotate(angle, _right); }
		void look_right(float angle) { _rotate(angle, glm::vec3(0, 1, 0)); }

		const glm::vec3& pos() const { return _pos; }
	private:
		inline void _rotate(float angle, const glm::vec3& axis) {
			_rot = glm::rotate(_rot, angle, axis);
			_STD memcpy(&_forward, &(glm::vec4(0, 0, 1, 0)*_rot), sizeof(_forward));
			_right = glm::cross(glm::vec3(0, 1, 0), _forward);
			_view = glm::lookAt(_pos, _pos + _forward, glm::vec3(0, 1, 0));
		}
		inline void _update() { _view = glm::lookAt(_pos, _pos + _forward, glm::vec3(0, 1, 0)); }

		glm::mat4 _view;
		glm::mat4 _rot;

		glm::vec3 _forward;
		glm::vec3 _right;
		glm::vec3 _pos;

		mutable buf::ubo<>::part _cbuf;
		GLuint _offset;
	};
}