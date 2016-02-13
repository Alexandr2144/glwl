#pragma once

#include "glwl\util.h"

namespace fractal {
	class cube {
	public:
		typedef glm::vec2 vec2;
		typedef glm::vec3 vec3;
		typedef glwl::ut::box::vertex vertex;

		_STD vector<glm::mat4> primitives;

		cube(glm::vec3 pos = glm::vec3(0, 0, 0), float len = 1.0f) 
			: _lvl(0), _pos(pos), _len(len) { _push(primitives, pos, len); }

		void increase() {
			float len, half;
			glm::vec3 pos;
			_STD vector<glm::mat4> newvec;
			const GLuint size = primitives.size();
			for (GLuint i = 0; i < size; i++) {
				pos = *((glm::vec3*)(&primitives[i][3]));
				half = primitives[i][0][0] / 3;
				len = half * 3/2.15f;

				_push(newvec, pos + glm::vec3( len,  len, 0), half);
				_push(newvec, pos + glm::vec3( len, -len, 0), half);
				_push(newvec, pos + glm::vec3(-len,  len, 0), half);
				_push(newvec, pos + glm::vec3(-len, -len, 0), half);

				_push(newvec, pos + glm::vec3( len,  len, len), half);
				_push(newvec, pos + glm::vec3( len, -len, len), half);
				_push(newvec, pos + glm::vec3(-len,  len, len), half);
				_push(newvec, pos + glm::vec3(-len, -len, len), half);

				_push(newvec, pos + glm::vec3( len, 0, len), half);
				_push(newvec, pos + glm::vec3(0,  len, len), half);
				_push(newvec, pos + glm::vec3(-len, 0, len), half);
				_push(newvec, pos + glm::vec3(0, -len, len), half);

				_push(newvec, pos + glm::vec3( len,  len, -len), half);
				_push(newvec, pos + glm::vec3( len, -len, -len), half);
				_push(newvec, pos + glm::vec3(-len,  len, -len), half);
				_push(newvec, pos + glm::vec3(-len, -len, -len), half);

				_push(newvec, pos + glm::vec3( len, 0, -len), half);
				_push(newvec, pos + glm::vec3(0,  len, -len), half);
				_push(newvec, pos + glm::vec3(-len, 0, -len), half);
				_push(newvec, pos + glm::vec3(0, -len, -len), half);
			}
			primitives = _STD move(newvec);
			_lvl++;
		}
		void decrease() {
			if (!_lvl) return;
			GLint newlvl = _lvl - 1;
			primitives.clear();
			_push(primitives, _pos, _len);
			for (GLint i = newlvl; i != 0; i--) increase();
			_lvl = newlvl;
		}

		template <class BufferTy, class CachePolicy,
			template <class> class BindPolicy>
		void load(const glwl::ut::box& box, GLuint off, glwl::buf::stream<BufferTy, CachePolicy, BindPolicy>& ubuf) {
			const GLuint cnt = primitives.size();
			for (GLuint i = 0; i < cnt; i++) {
				ubuf.unsafe::write(off, sizeof(glm::mat4), (char*)&primitives[i]);
				box.indices.draw();
			}
		}
	private:
		void _push(_STD vector<glm::mat4>& vec, glm::vec3 pos, float len) {
			glm::mat4 newcube;
			newcube = glm::translate(newcube, pos);
			newcube = glm::scale(newcube, glm::vec3(len, len, len));
			vec.push_back(newcube);
		}
		float _len;
		glm::vec3 _pos;
		GLint _lvl;
	};

	class cube_optimize : public cube {
	public:
		cube_optimize(glm::vec3 pos = glm::vec3(0, 0, 0), 
			float len = 1.0f) : cube(pos, len), _box(_STD _Noinit),
			vs(&_box.vertices), is(&_box.indices) {
			_construct(glm::vec3(0, 0, 0), len, bx::none, 2);
			_box.array.bind();
			_box.array[0] << vs;
			_box.array(0)[0][0] << glwl::format<vec3>();
			_box.array(1)[0][sizeof(vec3)] << glwl::format<vec3>();
			_box.array(2)[0][2 * sizeof(vec3)] << glwl::format<vec2>();
		}

		template <class BufferTy, class CachePolicy,
			template <class> class BindPolicy>
		void load(GLuint off, glwl::buf::stream<BufferTy, CachePolicy, BindPolicy>& ubuf) {
			_box.array.bind();
			_box.indices.bind();
			const GLuint cnt = primitives.size();
			for (GLuint i = 0; i < cnt; i++) {
				ubuf.unsafe::write(off, sizeof(glm::mat4), (char*)&primitives[i]);
				_box.indices.draw();
			}
		}
	private:
		typedef glwl::ut::box bx;
		void _construct(glm::vec3 pos, float len, int cull, int lvl) {
			if (!lvl) { _box.write(vs, is, pos, 
				vec3(len, len, len), (bx::cull_mode)cull); return; }

			float half = len / 3;
			len = half * 2;
			lvl--;

			_construct(pos + glm::vec3(len, len, 0), half, bx::front | bx::back, lvl);
			_construct(pos + glm::vec3(len, -len, 0), half, bx::front | bx::back, lvl);
			_construct(pos + glm::vec3(-len, len, 0), half, bx::front | bx::back, lvl);
			_construct(pos + glm::vec3(-len, -len, 0), half, bx::front | bx::back, lvl);

			_construct(pos + glm::vec3(len, len, len), half, bx::front | bx::left | bx::down, lvl);
			_construct(pos + glm::vec3(len, -len, len), half, bx::front | bx::left | bx::up, lvl);
			_construct(pos + glm::vec3(-len, len, len), half, bx::front | bx::right | bx::down, lvl);
			_construct(pos + glm::vec3(-len, -len, len), half, bx::front | bx::right | bx::up, lvl);

			_construct(pos + glm::vec3(len, 0, len), half, bx::up | bx::down, lvl);
			_construct(pos + glm::vec3(0, len, len), half, bx::left | bx::right, lvl);
			_construct(pos + glm::vec3(-len, 0, len), half, bx::up | bx::down, lvl);
			_construct(pos + glm::vec3(0, -len, len), half, bx::left | bx::right, lvl);

			_construct(pos + glm::vec3(len, len, -len), half, bx::back | bx::left | bx::down, lvl);
			_construct(pos + glm::vec3(len, -len, -len), half, bx::back | bx::left | bx::up, lvl);
			_construct(pos + glm::vec3(-len, len, -len), half, bx::back | bx::right | bx::down, lvl);
			_construct(pos + glm::vec3(-len, -len, -len), half, bx::back | bx::right | bx::up, lvl);

			_construct(pos + glm::vec3(len, 0, -len), half, bx::up | bx::down, lvl);
			_construct(pos + glm::vec3(0, len, -len), half, bx::left | bx::right, lvl);
			_construct(pos + glm::vec3(-len, 0, -len), half, bx::up | bx::down, lvl);
			_construct(pos + glm::vec3(0, -len, -len), half, bx::left | bx::right, lvl);
		}
		glwl::ut::box _box;
		_GLWL buf::elem_stream<vertex, glwl::buf::vbo<>,
			glwl::buf::a::cached<10>, glwl::buf::b::manual> vs;
		_GLWL buf::elem_stream<GLuint, glwl::buf::ibo<GLuint>,
			glwl::buf::a::cached<10>, glwl::buf::b::manual> is;
	};
};