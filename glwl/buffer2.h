#ifndef __GLWL_BUFFER_H
#define __GLWL_BUFFER_H

#include "core.h"

namespace glwl {
	namespace buf {
		template <
			typename IndexTy,
			typename BufferTy = buf::raw <
			buf::a::std_check,
			buf::b::dynamic,
			buf::c::map_none >
		>
		class ibo : public BufferTy {
		public:
			ibo() : BufferTy(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW) {}
			ibo(GLenum usage) : BufferTy(GL_ELEMENT_ARRAY_BUFFER, usage) {}
			ibo(GLenum usage, GLuint stride, GLuint count, const void* data = nullptr)
				: BufferTy(GL_ELEMENT_ARRAY_BUFFER, usage, stride*count, data) {}

			ibo(ibo&& src) : BufferTy(_STD forward<vbo>(src)), count(src.count), mode(src.mode) {}
			ibo(const ibo& src) : BufferTy(src), count(src.count), mode(src.mode) {}
			GLWL_ASSIG_RV_STD(ibo)
			GLWL_ASSIG_LV_STD(ibo)

			inline void draw() const { draw(count, NULL); }
			inline void draw(GLuint count, GLuint start_index = NULL) const {
				glDrawElements(mode, count, glwl::type<IndexTy>::id, (void*)start_index); }

			GLuint count = 0;
			GLenum mode = GL_TRIANGLES;
		};

		template <
			typename BufferTy = buf::raw <
			buf::a::std_check,
			buf::b::dynamic,
			buf::c::map_none >
		>
		class vbo : public BufferTy {
		public:
			vbo() : BufferTy(GL_ARRAY_BUFFER, GL_STATIC_DRAW) {}
			vbo(GLenum usage) : BufferTy(GL_ARRAY_BUFFER, usage) {}
			vbo(GLenum usage, GLuint size, const void* data = nullptr)
				: BufferTy(GL_ARRAY_BUFFER, usage, size, data) {}

			vbo(vbo&& src) : BufferTy(_STD forward<vbo>(src)) {}
			vbo(const vbo& src) : BufferTy(src) {}
			GLWL_ASSIG_RV_STD(vbo)
			GLWL_ASSIG_LV_STD(vbo)
		};

		template <
			typename BufferTy = buf::raw <
			buf::a::std_check,
			buf::b::dynamic,
			buf::c::map_none >
		>
		class ubo : public BufferTy {
		public:
			ubo() : BufferTy(GL_UNIFORM_BUFFER, GL_STATIC_DRAW) {}
			ubo(GLenum usage) : BufferTy(GL_UNIFORM_BUFFER, usage) {}
			ubo(GLenum usage, GLuint size, const void* data = nullptr)
				: BufferTy(GL_UNIFORM_BUFFER, usage, size, data) {}

			ubo(ubo&& src) : BufferTy(_STD forward<ubo>(src)) {}
			ubo(const ubo& src) : BufferTy(src) {}
			GLWL_ASSIG_RV_STD(ubo)
			GLWL_ASSIG_LV_STD(ubo)

			struct part {
				part(ubo* parent, GLuint offset) : ptr(parent), pos(offset) {}
				GLuint pos;
				ubo* ptr;

				template <typename ValTy>
				void write(GLuint offset, ValTy value) { 
					ptr->write(pos + offset, sizeof(value), &value); }
				void bind() const { ptr->bind(); }
			};

			part get(GLuint offset = NULL) { return part(this, offset); }
		};
	};

	class _uniform {
	public:
		struct mark { GLint offset; };
		struct value {
			GLint offset;
			GLint size;
		};
		struct array {
			GLint offset;
			GLint size;
			GLint stride;
		};

		class block {
		public:
			block(GLuint program, GLuint block_index) : _prog(program), _id(block_index) {}

			template <class BufferTy>
			void attach(const BufferTy& buf, GLuint index) const {
				glBindBufferBase(GL_UNIFORM_BUFFER, index, buf.id());
				glUniformBlockBinding(_prog, _id, buf.id());
			}
			template <class BufferTy>
			void attach(const BufferTy& buf, GLuint index, GLintptr offset, GLsizeiptr size) const {
				glBindBufferRange(GL_UNIFORM_BUFFER, index, buf.id(), offset, size);
				glUniformBlockBinding(_prog, _id, buf.id());
			}
			template <class BufferTy>
			void attach(const BufferTy& buf) const { attach(buf, _id); }
			template <class BufferTy>
			void attach(const BufferTy& buf, GLintptr offset, GLsizeiptr size) const {
				attach(buf, _id, offset, size);
			}

			void bind(GLuint buf) const { glUniformBlockBinding(_prog, _id, buf); }
			template <class BufferTy>
			void bind(const BufferTy& buf) const {
				glUniformBlockBinding(_prog, _id, buf.id());
			}

			GLuint id() const { return _id; }

			GLuint size() const { return property(GL_UNIFORM_BLOCK_DATA_SIZE); }
			GLint property(GLenum idx) const {
				GLint out;
				glGetActiveUniformBlockiv(_prog, _id, idx, &out);
				return out;
			}
		private:
			GLuint _prog;
			GLuint _id;
		};


		block get_block(const char* block_name) { return block(_prog, _getidx(block_name)); }
		block get_block(GLuint block_index) { return block(_prog, block_index); }

		GLint index(const char* var_name) const {
			GLuint id;
			glGetUniformIndices(_prog, 1, &var_name, &id);
			if (id == GL_INVALID_INDEX) throw _GLWL exception("Variable %s not found!", var_name);
			return id;
		}
		void indices(GLuint* indices, _STD initializer_list<const char*> var_names) const {
			glGetUniformIndices(_prog, var_names.size(), var_names.begin(), indices);
			//if (id == GL_INVALID_INDEX) throw _GLWL exception("Variable %s not found!", var_name);
		}

		void offsets(GLuint count, GLuint* indices, GLint* offset) const {
			glGetActiveUniformsiv(_prog, count, indices, GL_UNIFORM_OFFSET, offset); }
		GLint offset(GLuint index) const {
			GLint off; glGetActiveUniformsiv(_prog, 1, &index, GL_UNIFORM_OFFSET, &off);
			return off; }

		template <typename ValTy> ValTy require(const char* var_name) const;
		template <> array require(const char* var_name) const {
			return require<array>(index(var_name)); }
		template <> value require(const char* var_name) const {
			return require<value>(index(var_name)); }
		template <> GLint require(const char* var_name) const {
			return require<GLint>(index(var_name)); }

		template <typename ValTy> ValTy require(GLuint id) const;
		template <> array require(GLuint id) const {
			if (id == GL_INVALID_INDEX) throw _GLWL exception("Variable %d not found!", id);
			array out;
			glGetActiveUniformsiv(_prog, 1, &id, GL_UNIFORM_OFFSET, &out.offset);
			glGetActiveUniformsiv(_prog, 1, &id, GL_UNIFORM_SIZE, &out.size);
			glGetActiveUniformsiv(_prog, 1, &id, GL_UNIFORM_ARRAY_STRIDE, &out.stride);
			return out;
		}
		template <> value require(GLuint id) const {
			if (id == GL_INVALID_INDEX) throw _GLWL exception("Variable %d not found!", id);
			value out;
			glGetActiveUniformsiv(_prog, 1, &id, GL_UNIFORM_OFFSET, &out.offset);
			glGetActiveUniformsiv(_prog, 1, &id, GL_UNIFORM_SIZE, &out.size);
			return out;
		}
		template <> GLint require(GLuint id) const {
			if (id == GL_INVALID_INDEX) throw _GLWL exception("Variable %d not found!", id);
			GLint offset; glGetActiveUniformsiv(_prog, 1, &id, GL_UNIFORM_OFFSET, &offset);
			return offset;
		}
	private:
		inline GLuint _getidx(const char* block) {
			GLuint id = glGetUniformBlockIndex(_prog, block);
			if (id == GL_INVALID_INDEX) throw _GLWL exception("Uniform block %s not found!", block);
			return id;
		}
		GLuint _prog;
	};
};

#endif //__GLWL_BUFFER_H
