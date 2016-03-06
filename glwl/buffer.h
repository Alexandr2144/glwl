#ifndef __GLWL_BUFFER_H
#define __GLWL_BUFFER_H

#include "core.h"

namespace glwl {
	namespace buf {
		template <
			typename IndexTy,
			typename BufferTy = buf::raw <
			error::buf::std,
			buf::dynamic,
			buf::map_none >
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
			error::buf::std,
			buf::dynamic,
			buf::map_none >
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

		/*template <
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
		};*/

		namespace c {
			template <class CachePolicy>
			class no_check { bool full(size_t size) { return false; } };

			template <class CachePolicy>
			class check {
				bool full(size_t size) {
					if (size < CachePolicy::cache_capacity())
						return true;
					return false;
				}
			};
		};

		template <
			class BufferTy,
			class CachePolicy,
			template <class> class BindPolicy,
			template <class> class CheckPolicy>
		class ubo_stream :
			public basic_stream_impl<BufferTy, CachePolicy::impl, BindPolicy>,
			private CheckPolicy<typename CachePolicy::impl> {
		private:
			typedef basic_stream_impl<BufferTy, CachePolicy::impl, BindPolicy> base;
			typedef ubo_stream<BufferTy, CachePolicy, BindPolicy, CheckPolicy> mytype;
			typedef CheckPolicy<typename CachePolicy::impl> checker;
		public:
			ubo_stream(BufferTy* buf, GLuint pos = NULL) : stream(buf, pos), _off(NULL) {}
			GLWL_CTOR_LV_DELETE(ubo_stream)

			template <typename ElemTy>
			inline void read(size_t size, ElemTy* data) {
				base::read(size*sizeof(ElemTy), (char*)data);
			}
			template <typename ElemTy>
			inline void write(size_t size, const ElemTy* data) {
				if (checker::full(size)) {
					base::shift(out.offset);
					base::write(1, &out.value);
					base::shift(-out.offset);
				} else base::unsafe::write(_off, size*sizeof(ElemTy), (const char*)data);
			}

			template <typename ElemTy>
			inline mytype& operator<<(const ElemTy& out) { write(1, &out); return *this; }
			template <typename ElemTy>
			inline mytype& operator<<(_STD initializer_list<ElemTy> out) { write(out.size(), out.begin()); return *this; }

			template <typename ElemTy, unsigned Size>
			inline mytype& operator>>(ElemTy out[Size]) { read(Size*sizeof(ElemTy), out); return *this; }
			template <typename ElemTy>
			inline mytype& operator>>(ElemTy& out) { read(sizeof(ElemTy), &out); return *this; }

			inline base& operator<<(base&(*manip)(base&)) { manip(*this); return *this; }
			inline const base& operator>>(const base&(*manip)(const base&)) const { manip(*this); return *this; }

			mytype& operator[](GLintptr offset) { _off = offset; return *this; }
		private:
			GLintptr _off;
		};
	};

	class uniform {
	public:
		uniform(GLuint program) : _prog(program) {}

		struct pos { GLint offset; };
		struct value { GLint offset; GLint size; };
		struct array { GLint offset; GLint size; GLint stride; };

		template <typename ValTy, typename InfoTy = pos>
		struct var : InfoTy { ValTy value; };

		class block {
		public:
			block(GLuint program, GLuint block_index) : _prog(program), _id(block_index) {}
			block(GLuint program, const char* block_name) : _prog(program) {
				_id = glGetUniformBlockIndex(_prog, block_name);
				if (_id == GL_INVALID_INDEX) throw _GLWL exception("Uniform block %s not found!", block_name);
			}

			enum {
				size = GL_UNIFORM_BLOCK_DATA_SIZE,
			};

			void bind(GLuint index) {
				glUniformBlockBinding(_prog, _id, index); }

			GLuint id() const { return _id; }
			GLint property(GLenum prop) const {
				GLint out; glGetActiveUniformBlockiv(_prog, _id, prop, &out);
				return out; }
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

		GLint distance(const char* begin, const char* end) {
			GLint off[2];
			GLuint idx[2];
			indices(idx, { begin, end });
			offsets(2, idx, off);
			return off[2] - off[1];
		}

		template <typename ValTy> void require(ValTy& out, const char* var_name) const {
			return require<ValTy>(out, index(var_name)); }
		template <typename ValTy> void require(ValTy& out, GLuint id) const;
	
		template <> void require(array& out, GLuint id) const {
			if (id == GL_INVALID_INDEX) throw _GLWL exception("Variable %d not found!", id);
			glGetActiveUniformsiv(_prog, 1, &id, GL_UNIFORM_OFFSET, &out.offset);
			glGetActiveUniformsiv(_prog, 1, &id, GL_UNIFORM_SIZE, &out.size);
			glGetActiveUniformsiv(_prog, 1, &id, GL_UNIFORM_ARRAY_STRIDE, &out.stride);
		}
		template <> void require(value& out, GLuint id) const {
			if (id == GL_INVALID_INDEX) throw _GLWL exception("Variable %d not found!", id);
			glGetActiveUniformsiv(_prog, 1, &id, GL_UNIFORM_OFFSET, &out.offset);
			glGetActiveUniformsiv(_prog, 1, &id, GL_UNIFORM_SIZE, &out.size);
		}
		template <> void require(pos& out, GLuint id) const {
			if (id == GL_INVALID_INDEX) throw _GLWL exception("Variable %d not found!", id);
			glGetActiveUniformsiv(_prog, 1, &id, GL_UNIFORM_OFFSET, &out.offset);
		}
	private:
		inline GLuint _getidx(const char* block) {
			GLuint id = glGetUniformBlockIndex(_prog, block);
			if (id == GL_INVALID_INDEX) throw _GLWL exception("Uniform block %s not found!", block);
			return id;
		}
		GLuint _prog;
	};

	template <typename ValTy, typename InfoTy, 
		class BufferTy,
		class CachePolicy,
		template <class> class BindPolicy>
	buf::stream<BufferTy, CachePolicy, BindPolicy>& operator<<(
		buf::stream<BufferTy, CachePolicy, BindPolicy>& os, const uniform::var<ValTy, InfoTy>& out) {
		if (os.cache_capacity() >= sizeof(out.value)) {
			os.unsafe::write(out.offset, sizeof(out.value), (char*)&out.value);
			os.unsafe::save();
		} else {
			os.shift(out.offset); 
			os.write(1, &out.value);
			os.shift(-out.offset);
		} return os;
	}
};

#endif //__GLWL_BUFFER_H
