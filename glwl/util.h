#pragma once

#include "mesh.h"
#include "buffer.h"

namespace glwl {
	namespace ut {
		template <class ExceptionPolicy>
		class dds : public texture<ExceptionPolicy> {
		public:
			enum format_list {
				DXT_1 = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
				DXT_3 = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
				DXT_5 = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
			};

			dds(const _GLWL file& src) : texture(GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR) {
				static const int DDS_HEADER_SIZE = 128;
				static const int DDS_SIGNATURE = 0x20534444;
				static const int FORMAT_DXT1   = 0x31545844;
				static const int FORMAT_DXT3   = 0x33545844;
				static const int FORMAT_DXT5   = 0x35545844;

				ExceptionPolicy::check(src.size() < DDS_HEADER_SIZE,
					"DDS parser failure. Arguments: {\n"\
					"    file_handle: 0x%08X,\n"\
					"    file_size: %d,\n"\
					"}\n"
					"File size less than DDS_HEADER_SIZE = %d.",
					src.handle(), src.size(), DDS_HEADER_SIZE);

				GLuint* data = reinterpret_cast<GLuint*>(src.data());
				GLuint signature = *(data);
				GLuint height = *(data + 3);
				GLuint width = *(data + 4);
				GLuint mipmaps = *(data + 7);
				GLuint formatCode = *(data + 21);

				ExceptionPolicy::check(signature != DDS_SIGNATURE,
					"DDS parser failure. Arguments: {\n"\
					"    file_handle: 0x%08X,\n"\
					"}\n"
					"Invalid signature: 0x%08X.",
					src.handle(), signature);

				GLuint blocksize;
				switch (formatCode) {
				case FORMAT_DXT1:
					_fmt = DXT_1;
					blocksize = 8;
					break;
				case FORMAT_DXT3:
					_fmt = DXT_3;
					blocksize = 16;
					break;
				case FORMAT_DXT5:
					_fmt = DXT_5;
					blocksize = 16;
					break;
				case 0:
					_fmt = (format_list)GL_RGBA;
					blocksize = 16;
					break;
				default:
					ExceptionPolicy::check(true,
						"DDS parser failure. Arguments: {\n"\
						"    file_handle: 0x%08X,\n"\
						"}\n"
						"Unknown format: 0x%08X.",
						src.handle(), formatCode);
				}

				char* end = reinterpret_cast<char*>(src.data()) + src.size();
				char* offset = reinterpret_cast<char*>(src.data()) + DDS_HEADER_SIZE;
				if (mipmaps == 0) mipmaps = 1;
				for (GLuint level = 0; level < mipmaps; level++) {
					GLuint size = ((width + 3) / 4)*((height + 3) / 4)*blocksize;
					ExceptionPolicy::check(offset + size > end, 
						"DDS parser failure. Arguments: {\n"\
						"    file_handle: 0x%08X,\n"\
						"    file_size: %d,\n"\
						"    level: %d,\n"\
						"    offset: %d,\n"\
						"    size: %d\n"\
						"}\n"
						"Can't load mipmap fragment.", 
						src.handle(), src.size(), level, offset, size);
					reserve_compressed2d(GL_TEXTURE_2D, level, width, height, _fmt, size, offset);

					width = width > 1 ? width >> 1 : 1;
					height = height > 1 ? height >> 1 : 1;
					offset += size;
				}
			}

			format_list format() const { return _fmt; }
			GLuint block_size() const {}
		private:
			format_list _fmt;
			GLuint _blocksize;
		};

		class plane {
		public:
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

			plane(vec3 pos, vec2 size, vec2 texsize) {
				_GLWL buf::elem_stream<vertex, glwl::buf::vbo<>,
					glwl::buf::a::cached<1024>, glwl::buf::b::manual> vs(&vertices);
				_GLWL buf::elem_stream<GLuint, glwl::buf::ibo<GLuint>,
					glwl::buf::a::cached<1024>, glwl::buf::b::manual> is(&indices);

				vec3 v[] = {
					vec3{ -size.x, 0, -size.y } +pos, vec3{ -size.x, 0, size.y } +pos,
					vec3{  size.x, 0, -size.y } +pos, vec3{  size.x, 0, size.y } +pos,
				};

				vec3 n = vec3{ 0,  1, 0 };

				vec2 t[] = {
					vec2{ 0, 0 }, vec2{ 0, 1 * texsize.y },
					vec2{ 1 * texsize.x, 0 }, vec2{ 1 * texsize.x, 1 * texsize.y },
				};

				vs.bind();
				vs << _STD initializer_list<vertex> {
					{ v[0], n, t[0] }, { v[1], n, t[1] }, 
					{ v[2], n, t[2] }, { v[3], n, t[3] },
				}; 	vs.flush();

				is.bind();
				is << _STD initializer_list<GLuint> { 0, 2, 1, 2, 3, 1 };
				indices.mode = GL_TRIANGLES;
				indices.count = 6;
				is.flush();

				array.bind();
				array[0] << vs;
				array(0)[0][0] << format<vec3>();
				array(1)[0][sizeof(vec3)] << format<vec3>();
				array(2)[0][2 * sizeof(vec3)] << format<vec2>();
			}
		};

		class skybox {
		public:
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

			skybox(vec3 pos = { 0, 0, 0 }, vec3 size = { 40, 40, 40 }) {
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

				float coef = 1.0f;
				vec2 t[] = { 
					coef*vec2{ 0.250, 0.334 }, coef*vec2{ 0.250, 0.666 }, coef * vec2{ 0.500, 0.334 }, coef * vec2{ 0.500, 0.666 },
					coef*vec2{ 0.000, 0.334 }, coef*vec2{ 0.001, 0.666 }, coef * vec2{ 0.250, 0.334 }, coef * vec2{ 0.250, 0.666 },
					coef*vec2{ 0.250, 0.000 }, coef*vec2{ 0.250, 0.333 }, coef * vec2{ 0.500, 0.000 }, coef * vec2{ 0.500, 0.333 },
					coef*vec2{ 0.750, 0.334 }, coef*vec2{ 0.750, 0.666 }, coef * vec2{ 1.000, 0.334 }, coef * vec2{ 1.000, 0.666 },
					coef*vec2{ 0.500, 0.334 }, coef*vec2{ 0.500, 0.666 }, coef * vec2{ 0.750, 0.334 }, coef * vec2{ 0.750, 0.666 },
					coef*vec2{ 0.250, 0.667 }, coef*vec2{ 0.250, 1.000 }, coef * vec2{ 0.500, 0.667 }, coef * vec2{ 0.500, 1.000 },
				};

				vs.bind();
				vs << _STD initializer_list<vertex> {
					{ v[0], n[0], t[ 0] }, { v[1], n[0], t[ 1] }, { v[2], n[0], t[ 2] }, { v[3], n[0], t[ 3] },
					{ v[4], n[1], t[ 4] }, { v[5], n[1], t[ 5] }, { v[0], n[1], t[ 6] }, { v[1], n[1], t[ 7] },
					{ v[4], n[2], t[ 8] }, { v[0], n[2], t[ 9] }, { v[6], n[2], t[10] }, { v[2], n[2], t[11] },
					{ v[6], n[3], t[12] }, { v[7], n[3], t[13] }, { v[4], n[3], t[14] }, { v[5], n[3], t[15] },
					{ v[2], n[4], t[16] }, { v[3], n[4], t[17] }, { v[6], n[4], t[18] }, { v[7], n[4], t[19] },
					{ v[1], n[5], t[20] }, { v[5], n[5], t[21] }, { v[3], n[5], t[22] }, { v[7], n[5], t[23] },
				}; 	vs.flush();

				is.bind();
				is << _STD initializer_list<GLuint> { 0, 2, 1, 2, 3, 1, 4, 6, 5, 6, 7, 5, 8,
					10, 9, 10, 11, 9, 12, 14, 13, 14, 15, 13, 16, 18, 17, 18, 19, 17, 20, 22, 21, 22, 23, 21 };
				indices.mode = GL_TRIANGLES;
				indices.count = 36;
				is.flush();

				array.bind();
				array[0] << vs;
				array(0)[0][0] << format<vec3>();
				array(1)[0][sizeof(vec3)] << format<vec3>();
				array(2)[0][2 * sizeof(vec3)] << format<vec2>();
			}
		};

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
					glwl::buf::a::cached<128>, glwl::buf::b::manual> vs(&vertices);
				_GLWL buf::elem_stream<GLuint, glwl::buf::ibo<GLuint>,
					glwl::buf::a::cached<128>, glwl::buf::b::manual> is(&indices);
				write(vs, is, pos, size, cull);

				array.bind();
				array[0] << vs;
				array(0)[0][0] << format<vec3>();
				array(1)[0][sizeof(vec3)] << format<vec3>();
				array(2)[0][2 * sizeof(vec3)] << format<vec2>();
			}

			template <GLuint N1, GLuint N2>
			void write(_GLWL buf::elem_stream<vertex, glwl::buf::vbo<>,
					glwl::buf::a::cached<N1>, glwl::buf::b::manual>& vs,
				_GLWL buf::elem_stream<GLuint, glwl::buf::ibo<GLuint>,
					glwl::buf::a::cached<N2>, glwl::buf::b::manual>& is, 
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