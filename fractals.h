#pragma once

#include "glwl\util.h"

namespace fractal {
	class cube {
	public:
		enum cull_mode {
			front = 0x01,
			back = 0x02,
			left = 0x04,
			right = 0x08,
			up = 0x10,
			down = 0x20,
			all = 0xff,
			none = 0x00,
		};

		typedef glm::vec2 vec2;
		typedef glm::vec3 vec3;
		typedef glwl::ut::box::vertex vertex;

		_STD vector<glm::mat4> primitives;
		glwl::vbo<vertex> vertices;
		glwl::ibo<GLuint> indices;
		glwl::vao array;

		cube(glwl::uniform& cbuf, const char* uf_matname, 
			glm::vec3 pos = glm::vec3(0, 0, 0), float len = 1.0f) : _cbuf(cbuf), _lvl(0) {
			_push(primitives, pos, len);
			cbuf.bind();
			_offset = cbuf.require(uf_matname).offset;

			vertices.reserve(10000);
			indices.reserve(10000);

			indices.mode = GL_TRIANGLES;
			
			_construct({ 0, 0, 0 }, len, none, 0);

			vertices.flush();
			indices.flush();

			array.bind();
			array[0] << vertices;
			array(0)[0][0] << glwl::format<vec3>();
			array(1)[0][sizeof(vec3)] << glwl::format<vec3>();
			array(2)[0][2 * sizeof(vec3)] << glwl::format<vec2>();
		}

		void update(GLuint idx) { _cbuf.write(_offset, primitives[idx]); }

		void increase() {
			float len, half;
			glm::vec3 pos;
			_STD vector<glm::mat4> newvec;
			const GLuint size = primitives.size();
			for (GLuint i = 0; i < size; i++) {
				pos = *((glm::vec3*)(&primitives[i][3]));
				half = primitives[i][0][0] / 3;
				len = half * 2;

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
		}
		void decrease() {}
	private:
		void _push(_STD vector<glm::mat4>& vec, glm::vec3 pos, float len) {
			glm::mat4 newcube;
			newcube = glm::translate(newcube, pos);
			newcube = glm::scale(newcube, glm::vec3(len, len, len));
			vec.push_back(newcube);
		}

		void _construct(glm::vec3 pos, float len, int cull, int lvl) {
			if (!lvl) { _box(pos, len, cull); return; }

			float half = len / 3;
			len = half * 2;
			lvl--;

			_construct(pos + glm::vec3(len, len, 0), half, front | back, lvl);
			_construct(pos + glm::vec3(len, -len, 0), half, front | back, lvl);
			_construct(pos + glm::vec3(-len, len, 0), half, front | back, lvl);
			_construct(pos + glm::vec3(-len, -len, 0), half, front | back, lvl);

			_construct(pos + glm::vec3(len, len, len), half, front | left | down, lvl);
			_construct(pos + glm::vec3(len, -len, len), half, front | left | up, lvl);
			_construct(pos + glm::vec3(-len, len, len), half, front | right | down, lvl);
			_construct(pos + glm::vec3(-len, -len, len), half, front | right | up, lvl);

			_construct(pos + glm::vec3(len, 0, len), half, up | down, lvl);
			_construct(pos + glm::vec3(0, len, len), half, left | right, lvl);
			_construct(pos + glm::vec3(-len, 0, len), half, up | down, lvl);
			_construct(pos + glm::vec3(0, -len, len), half, left | right, lvl);

			_construct(pos + glm::vec3(len, len, -len), half, back | left | down, lvl);
			_construct(pos + glm::vec3(len, -len, -len), half, back | left | up, lvl);
			_construct(pos + glm::vec3(-len, len, -len), half, back | right | down, lvl);
			_construct(pos + glm::vec3(-len, -len, -len), half, back | right | up, lvl);

			_construct(pos + glm::vec3(len, 0, -len), half, up | down, lvl);
			_construct(pos + glm::vec3(0, len, -len), half, left | right, lvl);
			_construct(pos + glm::vec3(-len, 0, -len), half, up | down, lvl);
			_construct(pos + glm::vec3(0, -len, -len), half, left | right, lvl);
		}

		void _box(glm::vec3 pos, float len, int cull) {
			GLuint baseidx = vertices.count();
			vec3 v[] = {
				vec3{ -len, -len, -len }+pos, vec3{ -len, len, -len }+pos,
				vec3{ len, -len, -len }+pos, vec3{ len, len, -len }+pos,
				vec3{ -len, -len, len }+pos, vec3{ -len, len, len }+pos,
				vec3{ len, -len, len }+pos, vec3{ len, len, len }+pos
			};
			vec3 n[] = {
				vec3{ 0, 0, -1 }, vec3{ -1, 0, 0 }, vec3{ 0, -1, 0 },
				vec3{ 0, 0, 1 }, vec3{ 1, 0, 0 }, vec3{ 0, 1, 0 }
			};
			vec2 t[] = { vec2{ 0, 0 }, vec2{ 0, 1 }, vec2{ 1, 0 }, vec2{ 1, 1 } };
			if (!(cull&front)) { vertices << _STD initializer_list<vertex>
				{ { v[0], n[0], t[0] }, { v[1], n[0], t[1] }, { v[2], n[0], t[2] }, { v[3], n[0], t[3] } }, 
				indices << _STD initializer_list<GLuint> { 0 + baseidx, 1 + baseidx, 
					2 + baseidx, 2 + baseidx, 1 + baseidx, 3 + baseidx }; baseidx += 4;}
			if (!(cull&left)) { vertices << _STD initializer_list<vertex>
				{ { v[4], n[1], t[0] }, { v[5], n[1], t[1] }, { v[0], n[1], t[2] }, { v[1], n[1], t[3] } }, 
				indices << _STD initializer_list<GLuint> { 0 + baseidx, 1 + baseidx, 
					2 + baseidx, 2 + baseidx, 1 + baseidx, 3 + baseidx }; baseidx += 4;}
			if (!(cull&down)) { vertices << _STD initializer_list<vertex>
				{ { v[4], n[2], t[0] }, { v[0], n[2], t[1] }, { v[6], n[2], t[2] }, { v[2], n[2], t[3] } }, 
				indices << _STD initializer_list<GLuint> { 0 + baseidx, 1 + baseidx, 
					2 + baseidx, 2 + baseidx, 1 + baseidx, 3 + baseidx }; baseidx += 4;}
			if (!(cull&back)) { vertices << _STD initializer_list<vertex>
				{ { v[6], n[3], t[0] }, { v[7], n[3], t[1] }, { v[4], n[3], t[2] }, { v[5], n[3], t[3] } }, 
				indices << _STD initializer_list<GLuint> { 0 + baseidx, 1 + baseidx, 
					2 + baseidx, 2 + baseidx, 1 + baseidx, 3 + baseidx }; baseidx += 4;}
			if (!(cull&right)) { vertices << _STD initializer_list<vertex>
				{ { v[2], n[4], t[0] }, { v[3], n[4], t[1] }, { v[6], n[4], t[2] }, { v[7], n[4], t[3] } }, 
				indices << _STD initializer_list<GLuint> { 0 + baseidx, 1 + baseidx, 
					2 + baseidx, 2 + baseidx, 1 + baseidx, 3 + baseidx }; baseidx += 4;}
			if (!(cull&up)) { vertices << _STD initializer_list<vertex>
				{ { v[1], n[5], t[0] }, { v[5], n[5], t[1] }, { v[3], n[5], t[2] }, { v[7], n[5], t[3] } }, 
				indices << _STD initializer_list<GLuint> { 0 + baseidx, 1 + baseidx, 
					2 + baseidx, 2 + baseidx, 1 + baseidx, 3 + baseidx }; baseidx += 4;}
			//vertices.flush();
			//indices.flush();
		}
		GLint _lvl;

		glwl::uniform& _cbuf;
		GLuint _offset;
	};
};