#pragma once

#include "mesh.h"
#include "buffer.h"

namespace glwl {
	namespace ut {
		class box {
		public:
			enum cull_mode {
				none = 0x00000000, cull_all = 0x000000ff, inv_all = 0x0000ff00,

				front = 0x01, back  = 0x02,
				left  = 0x04, right = 0x08,
				up	  = 0x10, down  = 0x20,

				front_inv = 0x0100, back_inv  = 0x0200,
				left_inv  = 0x0400, right_inv = 0x0800,
				up_inv    = 0x1000, down_inv  = 0x2000,
			};

			typedef glm::vec2 vec2;
			typedef glm::vec3 vec3;
			struct vertex {
				vec3 position;
				vec3 normal;
				vec2 texcoord;
			};

			_GLWL buf::vbo <> vertices;
			_GLWL buf::ibo<GLuint> indices;
			_GLWL vao array;

			box(_STD _Uninitialized) {}

			box(vec3 pos, vec3 size, cull_mode cull) {
				_GLWL buf::elem_stream<vertex, glwl::buf::vbo<>,
					glwl::buf::a::cached<10>, glwl::buf::b::manual> vs(&vertices);
				_GLWL buf::elem_stream<GLuint, glwl::buf::ibo<GLuint>,
					glwl::buf::a::cached<10>, glwl::buf::b::manual> is(&indices);
				write(vs, is, pos, size, cull);

				array.bind();
				array[0] << vs;
				array(0)[0][0] << format<vec3>();
				array(1)[0][sizeof(vec3)] << format<vec3>();
				array(2)[0][2 * sizeof(vec3)] << format<vec2>();
			}

			void write(_GLWL buf::elem_stream<vertex, glwl::buf::vbo<>,
					glwl::buf::a::cached<10>, glwl::buf::b::manual>& vs,
				_GLWL buf::elem_stream<GLuint, glwl::buf::ibo<GLuint>,
					glwl::buf::a::cached<10>, glwl::buf::b::manual>& is, 
				vec3 pos, vec3 size, cull_mode cull) 
			{
				vec3 v[] = {
					vec3{ -size.x, -size.y, -size.z } +pos, vec3{ -size.x, size.y, -size.z } +pos,
					vec3{  size.x, -size.y, -size.z } +pos, vec3{  size.x, size.y, -size.z } +pos,
					vec3{ -size.x, -size.y,  size.z } +pos, vec3{ -size.x, size.y,  size.z } +pos,
					vec3{  size.x, -size.y,  size.z } +pos, vec3{  size.x, size.y,  size.z } +pos
				};
				vec3 n[] = {
					vec3{ 0, 0, -1 }, vec3{ -1, 0, 0 }, vec3{ 0, -1, 0 },
					vec3{ 0, 0, 1 },  vec3{ 1, 0, 0 },  vec3{ 0, 1, 0 }
				};
				vec2 t[] = { vec2{ 0, 0 }, vec2{ 0, 1 }, vec2{ 1, 0 }, vec2{ 1, 1 } };
								
				GLuint baseidx = vertices.capacity()/32;
				indices.count += 36;
				indices.mode = GL_TRIANGLES;
				vs.bind();
				is.bind();

				if (!(cull&front)) { if (cull&front_inv) vs << _STD initializer_list<vertex>
						{ { v[0], n[3], t[0] }, { v[2], n[3], t[1] }, { v[1], n[3], t[2] }, { v[3], n[3], t[3] } };
				else vs << _STD initializer_list<vertex>
						{ { v[0], n[0], t[0] }, { v[1], n[0], t[1] }, { v[2], n[0], t[2] }, { v[3], n[0], t[3] } };
					is << _STD initializer_list<GLuint> { GLuint(0 + baseidx), GLuint(1 + baseidx),
						GLuint(2 + baseidx), GLuint(2 + baseidx), GLuint(1 + baseidx), GLuint(3 + baseidx) }; baseidx += 4;}
				if (!(cull&left)) { if (cull&left_inv) vs << _STD initializer_list<vertex>
						{ { v[4], n[4], t[0] }, { v[0], n[4], t[1] }, { v[5], n[4], t[2] }, { v[1], n[4], t[3] } };
					else vs << _STD initializer_list<vertex>
						{ { v[4], n[1], t[0] }, { v[5], n[1], t[1] }, { v[0], n[1], t[2] }, { v[1], n[1], t[3] } };
					is << _STD initializer_list<GLuint> { GLuint(0 + baseidx), GLuint(1 + baseidx),
						GLuint(2 + baseidx), GLuint(2 + baseidx), GLuint(1 + baseidx), GLuint(3 + baseidx) }; baseidx += 4;}
				if (!(cull&down)) { if (cull&down_inv) vs << _STD initializer_list<vertex>
						{ { v[4], n[5], t[0] }, { v[6], n[5], t[1] }, { v[0], n[5], t[2] }, { v[2], n[5], t[3] } };
					else vs << _STD initializer_list<vertex>
						{ { v[4], n[2], t[0] }, { v[0], n[2], t[1] }, { v[6], n[2], t[2] }, { v[2], n[2], t[3] } };
					is << _STD initializer_list<GLuint> { GLuint(0 + baseidx), GLuint(1 + baseidx),
						GLuint(2 + baseidx), GLuint(2 + baseidx), GLuint(1 + baseidx), GLuint(3 + baseidx) }; baseidx += 4;}
				if (!(cull&back)) { if (cull&back_inv) vs << _STD initializer_list<vertex>
						{ { v[6], n[0], t[0] }, { v[4], n[0], t[1] }, { v[7], n[0], t[2] }, { v[5], n[0], t[3] } };
					else vs << _STD initializer_list<vertex>
						{ { v[6], n[3], t[0] }, { v[7], n[3], t[1] }, { v[4], n[3], t[2] }, { v[5], n[3], t[3] } };
					is << _STD initializer_list<GLuint> { GLuint(0 + baseidx), GLuint(1 + baseidx),
						GLuint(2 + baseidx), GLuint(2 + baseidx), GLuint(1 + baseidx), GLuint(3 + baseidx) }; baseidx += 4;}
				if (!(cull&right)) { if (cull&right_inv) vs << _STD initializer_list<vertex>
						{ { v[2], n[1], t[0] }, { v[6], n[1], t[1] }, { v[3], n[1], t[2] }, { v[7], n[1], t[3] } };
					else vs << _STD initializer_list<vertex>
						{ { v[2], n[4], t[0] }, { v[3], n[4], t[1] }, { v[6], n[4], t[2] }, { v[7], n[4], t[3] } };
					is << _STD initializer_list<GLuint> { GLuint(0 + baseidx), GLuint(1 + baseidx),
						GLuint(2 + baseidx), GLuint(2 + baseidx), GLuint(1 + baseidx), GLuint(3 + baseidx) }; baseidx += 4;}
				if (!(cull&up)) { if (cull&up_inv) vs << _STD initializer_list<vertex>
						{ { v[1], n[2], t[0] }, { v[3], n[2], t[1] }, { v[5], n[2], t[2] }, { v[7], n[2], t[3] } };
					else vs << _STD initializer_list<vertex>
						{ { v[1], n[5], t[0] }, { v[5], n[5], t[1] }, { v[3], n[5], t[2] }, { v[7], n[5], t[3] } };
					is << _STD initializer_list<GLuint> { GLuint(0 + baseidx), GLuint(1 + baseidx),
						GLuint(2 + baseidx), GLuint(2 + baseidx), GLuint(1 + baseidx), GLuint(3 + baseidx) }; baseidx += 4;}

				vs.flush();
				is.flush();
			}

			box(vec3 pos = { 0, 0, 0 }, vec3 size = { 1, 1, 1 }) {
				_GLWL buf::elem_stream<vertex, glwl::buf::vbo<>,
					glwl::buf::a::cached<1024>, glwl::buf::b::manual> vs(&vertices);
				_GLWL buf::elem_stream<GLuint, glwl::buf::ibo<GLuint>,
					glwl::buf::a::cached<1024>, glwl::buf::b::manual> is(&indices);
				vec3 v[] = {
					vec3{ -size.x, -size.y, -size.z } +pos, vec3{ -size.x, size.y, -size.z } +pos,
					vec3{ size.x, -size.y, -size.z } +pos, vec3{ size.x, size.y, -size.z } +pos,
					vec3{ -size.x, -size.y, size.z } +pos, vec3{ -size.x, size.y, size.z } +pos,
					vec3{ size.x, -size.y, size.z } +pos, vec3{ size.x, size.y, size.z } +pos
				};
				vec3 n[] = {
					vec3{ 0, 0, -1 }, vec3{ -1, 0, 0 }, vec3{ 0, -1, 0 },
					vec3{ 0, 0, 1 }, vec3{ 1, 0, 0 }, vec3{ 0, 1, 0 }
				};
				vec2 t[] = { vec2{ 0, 0 }, vec2{ 0, 1 }, vec2{ 1, 0 }, vec2{ 1, 1 } };
				vs.bind();
				vs << _STD initializer_list<vertex> {
					{ v[0], n[0], t[0] }, { v[1], n[0], t[1] }, { v[2], n[0], t[2] }, { v[3], n[0], t[3] },
					{ v[4], n[1], t[0] }, { v[5], n[1], t[1] }, { v[0], n[1], t[2] }, { v[1], n[1], t[3] },
					{ v[4], n[2], t[0] }, { v[0], n[2], t[1] }, { v[6], n[2], t[2] }, { v[2], n[2], t[3] },
					{ v[6], n[3], t[0] }, { v[7], n[3], t[1] }, { v[4], n[3], t[2] }, { v[5], n[3], t[3] },
					{ v[2], n[4], t[0] }, { v[3], n[4], t[1] }, { v[6], n[4], t[2] }, { v[7], n[4], t[3] },
					{ v[1], n[5], t[0] }, { v[5], n[5], t[1] }, { v[3], n[5], t[2] }, { v[7], n[5], t[3] },
				}; 	vs.flush();

				is.bind();
				is << _STD initializer_list<GLuint> { 0, 1, 2, 2, 1, 3, 4, 5, 6, 6, 5, 7, 8,
					9, 10, 10, 9, 11, 12, 13, 14, 14, 13, 15, 16, 17, 18, 18, 17, 19, 20, 21, 22, 22, 21, 23 };
				indices.mode = GL_TRIANGLES;
				indices.count = 36;
				is.flush();

				/*GLubyte baseidx = 0;
				if (!(cull&front)) { if (cull&front_inv) vertices << _STD initializer_list<vertex>
				{ { v[0], n[3], t[0] }, { v[2], n[3], t[1] }, { v[1], n[3], t[2] }, { v[3], n[3], t[3] } };
				else vertices << _STD initializer_list<vertex>
				{ { v[0], n[0], t[0] }, { v[1], n[0], t[1] }, { v[2], n[0], t[2] }, { v[3], n[0], t[3] } };
				indices << _STD initializer_list<GLubyte> { GLubyte(0 + baseidx), GLubyte(1 + baseidx),
				GLubyte(2 + baseidx), GLubyte(2 + baseidx), GLubyte(1 + baseidx), GLubyte(3 + baseidx) }; baseidx += 4;}
				if (!(cull&left)) { if (cull&left_inv) vertices << _STD initializer_list<vertex>
				{ { v[4], n[4], t[0] }, { v[0], n[4], t[1] }, { v[5], n[4], t[2] }, { v[1], n[4], t[3] } };
				else vertices << _STD initializer_list<vertex>
				{ { v[4], n[1], t[0] }, { v[5], n[1], t[1] }, { v[0], n[1], t[2] }, { v[1], n[1], t[3] } };
				indices << _STD initializer_list<GLubyte> { GLubyte(0 + baseidx), GLubyte(1 + baseidx),
				GLubyte(2 + baseidx), GLubyte(2 + baseidx), GLubyte(1 + baseidx), GLubyte(3 + baseidx) }; baseidx += 4;}
				if (!(cull&down)) { if (cull&down_inv) vertices << _STD initializer_list<vertex>
				{ { v[4], n[5], t[0] }, { v[6], n[5], t[1] }, { v[0], n[5], t[2] }, { v[2], n[5], t[3] } };
				else vertices << _STD initializer_list<vertex>
				{ { v[4], n[2], t[0] }, { v[0], n[2], t[1] }, { v[6], n[2], t[2] }, { v[2], n[2], t[3] } };
				indices << _STD initializer_list<GLubyte> { GLubyte(0 + baseidx), GLubyte(1 + baseidx),
				GLubyte(2 + baseidx), GLubyte(2 + baseidx), GLubyte(1 + baseidx), GLubyte(3 + baseidx) }; baseidx += 4;}
				if (!(cull&back)) { if (cull&back_inv) vertices << _STD initializer_list<vertex>
				{ { v[6], n[0], t[0] }, { v[4], n[0], t[1] }, { v[7], n[0], t[2] }, { v[5], n[0], t[3] } };
				else vertices << _STD initializer_list<vertex>
				{ { v[6], n[3], t[0] }, { v[7], n[3], t[1] }, { v[4], n[3], t[2] }, { v[5], n[3], t[3] } };
				indices << _STD initializer_list<GLubyte> { GLubyte(0 + baseidx), GLubyte(1 + baseidx),
				GLubyte(2 + baseidx), GLubyte(2 + baseidx), GLubyte(1 + baseidx), GLubyte(3 + baseidx) }; baseidx += 4;}
				if (!(cull&right)) { if (cull&right_inv) vertices << _STD initializer_list<vertex>
				{ { v[2], n[1], t[0] }, { v[6], n[1], t[1] }, { v[3], n[1], t[2] }, { v[7], n[1], t[3] } };
				else vertices << _STD initializer_list<vertex>
				{ { v[2], n[4], t[0] }, { v[3], n[4], t[1] }, { v[6], n[4], t[2] }, { v[7], n[4], t[3] } };
				indices << _STD initializer_list<GLubyte> { GLubyte(0 + baseidx), GLubyte(1 + baseidx),
				GLubyte(2 + baseidx), GLubyte(2 + baseidx), GLubyte(1 + baseidx), GLubyte(3 + baseidx) }; baseidx += 4;}
				if (!(cull&up)) { if (cull&up_inv) vertices << _STD initializer_list<vertex>
				{ { v[1], n[2], t[0] }, { v[3], n[2], t[1] }, { v[5], n[2], t[2] }, { v[7], n[2], t[3] } };
				else vertices << _STD initializer_list<vertex>
				{ { v[1], n[5], t[0] }, { v[5], n[5], t[1] }, { v[3], n[5], t[2] }, { v[7], n[5], t[3] } };
				indices << _STD initializer_list<GLubyte> { GLubyte(0 + baseidx), GLubyte(1 + baseidx),
				GLubyte(2 + baseidx), GLubyte(2 + baseidx), GLubyte(1 + baseidx), GLubyte(3 + baseidx) }; baseidx += 4;}*/

				array.bind();
				array[0] << vs;
				array(0)[0][0] << format<vec3>();
				array(1)[0][sizeof(vec3)] << format<vec3>();
				array(2)[0][2 * sizeof(vec3)] << format<vec2>();
			}
		};
	}
}