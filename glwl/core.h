#ifndef __GLWL_CORE_H
#define __GLWL_CORE_H

#include <cstdio>

#include <utility>
#include <new>

#include <GL\glew.h>
#include <GL\wglew.h>
#include <GL\freeglut.h>

#include <glm\common.hpp>

#pragma comment(lib,"OpenGL32.lib")
#pragma comment(lib,"freeglut.lib")
#pragma comment(lib,"glew32.lib")

//!!!
#include <io.h>

#define _GLWL ::glwl::

#define GLWL_MEM_ZERO(value) _STD memset(&value, NULL, sizeof(value));
#define GLWL_CTOR_RV_STD(myclass) \
	myclass(myclass&& src) { _STD memcpy(this, &src, sizeof(myclass)); \
	_STD memset(&src, NULL, sizeof(myclass)); }
#define GLWL_ASSIG_RV_STD(myclass) \
	myclass& operator=(myclass&& src) { \
	if (this == &src) return *this; \
	this->~myclass(); \
	new(this) myclass(_STD move(src)); \
	return *this; }
#define GLWL_ASSIG_LV_STD(myclass) \
	myclass& operator=(const myclass& src) {\
	if (this == &src) return *this; \
	this->~myclass(); \
	new(this) myclass(src); \
	return *this; }
#define GLWL_CTOR_RV_DELETE(myclass) \
	myclass(myclass&& src) = delete; \
	myclass& operator=(myclass&& src) = delete;
#define GLWL_CTOR_LV_DELETE(myclass) \
	myclass(const myclass& src) = delete; \
	myclass& operator=(const myclass& src) = delete;


namespace glwl {
	namespace macro {
#define GLWL_CASE_MACRO(def) case def: return #def
		const char* str(GLenum val) {
			switch (val) {
				GLWL_CASE_MACRO(GL_NO_ERROR);
				GLWL_CASE_MACRO(GL_INVALID_ENUM);
				GLWL_CASE_MACRO(GL_INVALID_VALUE);
				GLWL_CASE_MACRO(GL_INVALID_OPERATION);
				GLWL_CASE_MACRO(GL_STACK_OVERFLOW);
				GLWL_CASE_MACRO(GL_STACK_UNDERFLOW);
				GLWL_CASE_MACRO(GL_OUT_OF_MEMORY);
				GLWL_CASE_MACRO(GL_INVALID_FRAMEBUFFER_OPERATION);

				GLWL_CASE_MACRO(GL_STREAM_DRAW);
				GLWL_CASE_MACRO(GL_STREAM_READ);
				GLWL_CASE_MACRO(GL_STREAM_COPY);
				GLWL_CASE_MACRO(GL_STATIC_DRAW);
				GLWL_CASE_MACRO(GL_STATIC_READ);
				GLWL_CASE_MACRO(GL_STATIC_COPY);
				GLWL_CASE_MACRO(GL_DYNAMIC_DRAW);
				GLWL_CASE_MACRO(GL_DYNAMIC_READ);
				GLWL_CASE_MACRO(GL_DYNAMIC_COPY);

				GLWL_CASE_MACRO(GL_ARRAY_BUFFER);
				GLWL_CASE_MACRO(GL_ELEMENT_ARRAY_BUFFER);
				GLWL_CASE_MACRO(GL_UNIFORM_BUFFER);

				GLWL_CASE_MACRO(GL_TEXTURE_2D);
				GLWL_CASE_MACRO(GL_TEXTURE_3D);
				GLWL_CASE_MACRO(GL_LINEAR);
				GLWL_CASE_MACRO(GL_LINEAR_MIPMAP_LINEAR);
				//GLWL_CASE_MACRO(GL_DYNAMIC_COPY);

				GLWL_CASE_MACRO(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);
				GLWL_CASE_MACRO(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT);
				GLWL_CASE_MACRO(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);

			default: return "<unknown>";
			}
		}
		const char* str(const void* val) { return str(reinterpret_cast<GLenum>(val)); }
#undef GLWL_CASE_MACRO
	};

#undef GLWL_CASE_MACRO
	class exception : public _STD exception {
	public:
		exception() {}
		exception(const char* msg) : _msg(_msgbuf) {
			::strncpy_s(_msgbuf, msg, 512);
		}
		exception(const char* msg, int) : _msg(msg) {}
		template <typename... ArgsTy>
		exception(const char* format, ArgsTy... args) : _msg(_msgbuf) {
			::sprintf_s(_msgbuf, format, args...);
		}

		virtual const char* what() const { return _msg; }
	private:
		char _msgbuf[512];
		const char* _msg;
	};


#include "auto\exception.h"
	namespace error {
		class none {
		public:
			enum code { sucess };

			GLenum glerr() { return glGetError(); }
			code error() { return sucess; }
			code exception(code mask) { return sucess; }
			code exception() { return sucess; }
		protected:
			template <typename FTy, typename... ArgsTy>
			inline void call(FTy f, ArgsTy... args) const { f(args...); }
			template <typename... MsgTypes>
			inline void check(bool err, MsgTypes... msg) const {}

			none() {}
			~none() {}
		};
	};

	namespace buf {
		enum type {
			static_draw = GL_STATIC_DRAW,
			dynamic_draw = GL_DYNAMIC_DRAW,
			stream_draw = GL_STREAM_DRAW,

			vertex = GL_ARRAY_BUFFER,
			index = GL_ELEMENT_ARRAY_BUFFER,
			uniform = GL_UNIFORM_BUFFER
		};

		template <class ExceptionPolicy>
		class immutable : public ExceptionPolicy {
		public:
			inline void init(GLenum type, GLbitfield flags, GLuint size, const void* data) {
				ExceptionPolicy::call(glBufferStorage, type, size, data, flags); }
		protected:
			immutable() {}
			~immutable() {}
		};
		template <class ExceptionPolicy>
		class dynamic : public ExceptionPolicy {
		public:
			inline void init(GLenum type, GLenum usage, GLuint size, const void* data) {
				ExceptionPolicy::call(glBufferData, type, size, data, usage); }
		protected:
			dynamic() {}
			~dynamic() {}
		};

		template <class ExceptionPolicy, template<class> class MutablePolicy>
		struct map_none : MutablePolicy<ExceptionPolicy> {
		protected:
			typedef void mapptr;
			map_none() {}
			~map_none() {}
		};

		template <class ExceptionPolicy, template<class> class MutablePolicy>
		struct map_fast : MutablePolicy<ExceptionPolicy> {
		private:
			typedef ExceptionPolicy checker;
		public:
			typedef void* mapptr;

			inline void unmap(GLenum type) { glUnmapBuffer(type); }
			inline mapptr map(GLenum type, GLenum acess, GLsizeiptr length, GLintptr offset) {
				void* ptr = glMapBufferRange(type, offset, length, acess);
				checker::check(ptr == nullptr, "can't map this buffer");
				return ptr;
			}
			inline mapptr map(GLenum type, GLenum acess) {
				void* ptr = glMapBuffer(type, acess);
				checker::check(ptr == nullptr, "can't map this buffer");
				return ptr;
			}
		protected:
			map_fast() {}
			~map_fast() {}
		};

		template <class ExceptionPolicy, template<class> class MutablePolicy>
		struct map_safe : MutablePolicy<ExceptionPolicy> {
		private:
			typedef ExceptionPolicy checker;
		public:
			typedef class mapptr {
			public:
				mapptr(void* ptr, GLenum type, map_safe& ref)
					: _ptr(ptr), _type(type), _ref(ref) {}
				~mapptr() { _ref.unmap(_type); }
				void* operator*() { return _ptr; }
				void unmap() { _ref.unmap(_type); }
			private:
				void* _ptr;
				GLenum _type;
				map_safe& _ref;
			};

			inline void unmap(GLenum type) { if (_mapped) checker::call(glUnmapBuffer, type); _mapped = false; }
			inline mapptr map(GLenum type, GLenum acess, GLsizeiptr length, GLintptr offset) {
				_mapped = true;
				void* ptr = glMapBufferRange(type, offset, length, acess);
				checker::check(ptr == nullptr, "can't map this buffer");
				return mapptr(ptr, type, *this);
			}
			inline mapptr map(GLenum type, GLenum acess) {
				_mapped = true;
				void* ptr = glMapBuffer(type, acess);
				checker::check(ptr == nullptr, "can't map this buffer");
				return mapptr(ptr, type, *this);
			}

			inline bool mapped() { return _mapped; }
		protected:
			~map_safe() {}
			map_safe() : _mapped(false) {}
		private:
			bool _mapped;
		};

		template <class ExceptionPolicy,
			template<class> class MutablePolicy,
			template<class, template<class> class> class MapPolicy>
		class basic_raw : public MapPolicy<ExceptionPolicy, MutablePolicy> {
		private:
			typedef ExceptionPolicy checker;
			typedef const basic_raw& cref;
		public:
			typedef typename checker::code errcode;

			basic_raw(_STD _Uninitialized) : _id(0) {}
			basic_raw() { glGenBuffers(1, &_id); }
			~basic_raw() { glDeleteBuffers(1, &_id); }

			basic_raw(const basic_raw& src) = delete;
			basic_raw(basic_raw&& src) : _id(src._id) { src._id = 0; }

			inline void bind(GLenum type) const { checker::call(glBindBuffer, type, _id); }
			inline void bind(GLenum type, GLuint index) const {
				checker::call(glBindBufferBase, type, index, _id); }
			inline void bind(GLenum type, GLuint index, GLintptr offset, GLsizeiptr length) const {
				checker::call(glBindBufferRange, type, index, _id, offset, length); }

			inline void read(GLenum type, GLuint offset, GLuint size, void* data) {
				checker::call(glGetBufferSubData, type, offset, size, data); }
			inline void write(GLenum type, GLuint offset, GLuint size, const void* data) {
				checker::call(glBufferSubData, type, offset, size, data); }
			inline void copy(GLuint offset, cref src, GLuint src_offset, GLuint write_len) {
				bind(GL_COPY_WRITE_BUFFER);
				src.bind(GL_COPY_READ_BUFFER);
				checker::call(glCopyBufferSubData, GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
					offset, src_offset, write_len);
			}

			GLuint id() const { return _id; }

			inline GLenum error() const { return checker::error(); }
			inline errcode exception(errcode mask) { return checker::exception(mask); }
			inline errcode exception() const { return checker::exception(); }
		private:
			GLuint _id;
		};

		template <class ExceptionPolicy = std_check,
			template<class> class MutablePolicy = dynamic,
			template<class, template<class> class> class MapPolicy = map_safe>
		class raw :
			protected basic_raw<ExceptionPolicy, MutablePolicy, MapPolicy> {
		private:
			typedef basic_raw<ExceptionPolicy, MutablePolicy, MapPolicy> base;
			typedef MutablePolicy<ExceptionPolicy> initp;
			typedef ExceptionPolicy checker;
			typedef const raw& cref;
		public:
			typedef typename base::errcode errcode;
			typedef typename base::mapptr mapptr;

			~raw() {}

			raw() : raw(0, 0) {}
			raw(_STD _Uninitialized) : basic_raw(_STD _Noinit), raw(0, 0) {}
			raw(GLenum type, GLenum usage) : _len(0), _type(type), _usage(usage) {}
			raw(GLenum type, GLenum usage, GLuint size, const void* data = nullptr)
					: _len(size), _type(type), _usage(usage) {
				bind();
				reserve(size, data);
			}

			raw(raw&& src) : basic_raw(_STD move(src)), _len(src._len),
				_type(src._type), _usage(src._usage) {}
			raw(const raw& src) : raw(src._type, src._usage, src._len) {
				copy(0, src, 0, src._len); }

			raw& operator=(raw&& src) {
				this->~raw();
				new(this) raw(_STD move(src));
				return *this;
			}
			raw& operator=(const raw& src) {
				this->~raw();
				new(this) raw(src);
				return *this;
			}

			inline void bind() const { base::bind(_type); }
			inline void bind(GLuint index) const { base::bind(_type, index); }
			inline void bind(GLuint index, GLintptr offset) const {
				base::bind(_type, index, offset, _len); }
			inline void bind(GLuint index, GLintptr offset, GLsizeiptr length) const {
				checker::check((GLsizeiptr)_len < length, 
					"Bind function failure. Buffer too small [buflen: %d, len: %d]", _len, length);
				base::bind(_type, index, offset, length); }

			inline void unmap() { base::unmap(_type); }
			inline typename base::mapptr map(GLenum acess = GL_READ_WRITE) {
				return base::map(_type, acess); }
			inline typename base::mapptr map(GLintptr offset,
				GLenum acess_ext = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT) {
				return base::map(_type, acess_ext, _len, offset);
			}
			inline typename base::mapptr map(GLintptr offset, GLsizeiptr length,
				GLenum acess_ext = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT) {
				return base::map(_type, acess_ext, length, offset);
			}

			inline void read(GLuint offset, GLuint size, void* data) {
				base::read(_type, offset, size, data); }
			inline void write(GLuint offset, GLuint size, const void* data) {
				base::write(_type, offset, size, data); }
			inline void copy(GLuint offset, cref src, GLuint src_offset, GLuint write_len) {
				checker::check(offset + write_len == _len, "Copy error: dest buffer too small");
				checker::check(src_offset + write_len == src._len, "Copy error: src buffer too small");
				base::copy(offset, src, src_offset, write_len);
			}

			inline void reserve(GLuint size, const void* data = nullptr) {
				initp::init(_type, _usage, size, data);
				_len = size;
			}

			inline void resize(GLuint size) {
				GLuint write_len = size < _len ? size : _len;
				raw buf(_type, _usage, size);
				buf.base::copy(0, *this, 0, write_len);
				*this = _STD move(buf);
			}

			void resize(GLuint size, const void* data) {
				GLuint offset = _len;
				resize(size);
				if (!data) return;
				write(offset, size - offset, data);
			}

			inline GLuint id() const { return base::id(); }

			inline GLenum error() { return checker::error(); }
			inline errcode exception(errcode mask) { return checker::exception(mask); }
			inline errcode exception() { return checker::exception(); }

			inline GLenum type() { return _type; }
			inline void type(GLenum new_type) { _type = new_type; }

			inline GLenum usage() { return _type; }
			inline void usage(GLenum new_usage) { _usage = new_usage;}

			inline GLuint capacity() { return _len; }
		private:
			GLuint _len;
			GLenum _type;
			GLenum _usage;
		};


		namespace a {
			struct no_cached_unsafe {
				template <class BufferTy>
				class impl {
				public:
					inline static GLuint cache_capacity() { return NULL; }
					inline GLuint cache_size() const { return NULL; }

					inline GLintptr tell() const { return _offset; }
					inline void seek(GLintptr pos) { _offset = pos; }
					inline void shift(GLintptr offset) { _offset += offset; }

					inline void save() const {}
					inline void flush() const {}
					inline void clear() { _offset = 0; }
					inline void rdbuf(BufferTy* bufptr) { _buf = bufptr; }
					inline const BufferTy* rdbuf() const { return _buf; }
					inline BufferTy* rdbuf() { return _buf; }

					inline void read(GLuint size, char* data) const {
						_buf->read(_offset, size, (void*)data); }
					inline void write(GLuint size, const char* data) {
						_buf->write(_offset, size, data); }
					inline void write(GLuint offset, GLuint size, const char* data) {
						_buf->write(offset, size, data); }
				protected:
					impl(BufferTy* buf, GLuint pos) : _buf(buf), _offset(pos) {}
					~impl() {}

					GLintptr _offset;
					BufferTy* _buf;
				};
			};

			struct no_cached {
				template <class BufferTy>
				class impl : public no_cached_unsafe::impl<BufferTy> {
				public:
					typedef no_cached_unsafe::impl<BufferTy> unsafe;

					inline void seek(GLintptr pos) {
						if (pos < 0) throw _GLWL exception(
							"bufstream shift: out of range [pos: %d < 0].", pos);
						unsafe::seek(pos);
					}
					inline void shift(GLintptr offset) {
						GLintptr pos = unsafe::tell() + offset;
						if (pos < 0) throw _GLWL exception(
							"bufstream shift: out of range [pos: %d < 0].", pos);
						unsafe::seek(pos);
					}

					inline void clear() { _endbuf = 0, unsafe::clear(); }

					inline void write(GLuint size, const char* data) {
						GLuint len = _offset + size;
						if (len > _buf->capacity()) _buf->resize(len);
						unsafe::write(size, data);
					}
					inline void write(GLuint offset, GLuint size, const char* data) {
						GLuint off = _offset + offset;
						GLuint len = off + size;
						if (len > _buf->capacity()) _buf->resize(len);
						unsafe::write(off, size, data);
					}
				protected:
					impl(BufferTy* buf, GLuint pos) : unsafe(buf, pos) {}
					~impl() {}
				};
			};

			template <unsigned CacheSize>
			struct cached_unsafe {
				template <class BufferTy>
				class impl {
				public:
					inline static GLuint cache_capacity() { return CacheSize; }
					inline GLuint cache_size() const { return _last - _base; }

					inline GLintptr tell() { return _offset + (_last - _base); }

					inline void seek_near(GLintptr pos) { _last = _base + pos; }
					inline void seek_far(GLintptr pos) { save(); _offset = pos; }

					inline void shift_near(GLintptr offset) { _last += offset; }
					inline void shift_far(GLintptr offset) { save(); _offset += offset; }

					inline BufferTy* rdbuf() const { return _buf; }
					inline void rdbuf(BufferTy* bufptr) { _buf = bufptr; }
					inline void clear() { _offset = 0, _last = _base; }
					inline void flush() { save(); _offset += CacheSize; }
					inline void save() {
						_buf->write(_offset, CacheSize, _base);
						_last = _base;
					}

					inline void read(GLuint size, char* data) {
						_buf->read(_offset, size, (void*)data); }
					inline void write(GLuint size, const char* data) {
						_STD memcpy(_last, data, size);
						_last += size; }
					inline void write(GLuint offset, GLuint size, const char* data) {
						_STD memcpy(_base + offset, data, size); }
				protected:
					impl(BufferTy* buf, GLuint pos) 
						: _buf(buf), _last(_base), _offset(pos) {
						static_assert(CacheSize != NULL, "Cache size must be not eq null. "\
							"Use \"no_cached\" for disabe cache.");
					}
					~impl() {}

					GLintptr _offset;
					BufferTy* _buf;

					char _base[CacheSize];
					char* _last;
				};
			};

			template <unsigned CacheSize>
			struct cached {
				template <class BufferTy>
				class impl :
					public cached_unsafe<CacheSize>::impl<BufferTy> {
				public:
					typedef typename cached_unsafe<CacheSize>::impl<BufferTy> unsafe;

					inline GLuint tell() { return unsafe::tell(); }
					inline void seek(GLintptr pos) {
						GLintptr delta = pos - _offset;
						if (delta >= 0) unsafe::seek_near(delta);
						else unsafe::seek_far(pos);
					}
					inline void shift(GLintptr offset) {
						char* pos = _last + offset;
						if (pos < _base || pos > _end)
							unsafe::shift_far(offset);
						else _last = pos;
					}
					inline void save() { _save(_last - _base); }
					inline void flush() {
						GLuint size = _last - _base;
						_save(size); _offset += size; 
					}

					inline BufferTy* rdbuf() const { return unsafe::rdbuf(); }
					inline void rdbuf(BufferTy* bufptr) { flush(); unsafe::rdbuf(bufptr); }
				protected:
					inline void read(GLuint size, char* data) {
						_buf->read(_offset, size, (void*)data); }
					inline void write(GLuint size, const char* data) {
						if (size > CacheSize) {
							flush();
							_bufwrite(size, data);
							return; }
						GLuint capacity = _end - _last;
						GLint tail = size - capacity;
						if (tail < 0) { unsafe::write(size, data); return; }
						unsafe::write(capacity, data);
						data += capacity;
						_bufwrite(CacheSize, _base);
						_last = _base;
						unsafe::write(tail, data);
					}

					inline void write(GLuint offset, GLuint size, const char* data) {
						if (_last != _base) throw 1;
						if (offset + size >= CacheSize) 
							{ unsafe::write(offset, size, data); }
						else {
							GLuint off = _offset + offset;
							GLuint len = off + size;
							if (len > _buf->capacity()) _buf->resize(len);
							_buf->write(off, size, data);	
						}
					}

					impl(BufferTy* buf, GLuint pos) : unsafe(buf, pos), _end(_base + CacheSize) {}
					~impl() {}
				private:
					inline void _save(GLuint size) {
						if (!size) return;
						GLuint len = _offset + size;
						if (len > _buf->capacity()) _buf->resize(len);
						_buf->write(_offset, size, _base);
						_last = _base;
					}
					inline void _bufwrite(GLuint size, const char* data) {
						GLuint len = _offset + size;
						if (len > _buf->capacity()) _buf->resize(len);
						_buf->write(_offset, size, data), _offset = len;
					}
					char* _end;
				};
			};
		};

		namespace b {
			template <class CachePolicy>
			class autobind : public CachePolicy {
			protected:
				template <typename BufferTy>
				autobind(BufferTy* buf, GLuint pos) : CachePolicy(buf, pos) {}
				~autobind() {}
				void _bind() { CachePolicy::rdbuf()->bind(); }
			};
			template <class CachePolicy>
			class manual : public CachePolicy {
			public:
				void bind() { CachePolicy::rdbuf()->bind(); }
			protected:
				template <typename BufferTy>
				manual(BufferTy* buf, GLuint pos) : CachePolicy(buf, pos) {}
				~manual() {}
				void _bind() {}
			};
		};

		template <class BufferTy, 
			template <class> class CachePolicyImpl, 
			template <class> class BindPolicy>
		class basic_stream_impl : public BindPolicy<CachePolicyImpl<BufferTy>> {
		private:
			typedef BindPolicy<CachePolicyImpl<BufferTy>> cache;
			typedef basic_stream_impl mytype;
		public:
			typedef typename CachePolicyImpl<BufferTy>::unsafe unsafe;

			~basic_stream_impl() { flush(); }

			basic_stream_impl(BufferTy* buf, GLuint pos = NULL) : cache(buf, pos) {}
			basic_stream_impl(basic_stream_impl&& src)
				: cache(_STD forward<BufferTy>(src)) {}
			GLWL_CTOR_LV_DELETE(basic_stream_impl)
			GLWL_ASSIG_RV_STD(basic_stream_impl)

			inline void reserve(GLuint size, const char* data = nullptr) {
				_bind(); cache::rdbuf()->reserve(size, data); }
			inline void resize(GLuint size, const char* data = nullptr) {
				_bind(); cache::rdbuf()->resize(size, data); }
			//inline void read(size_t size, char* data) { cache::read(size, data); }
			//inline void write(size_t size, const char* data) { cache::write(size, data); }

			inline void unmap() { _bind(); cache::rdbuf()->unmap(_type); }
			inline bool mapped() { return BufferTy::mapped(); }

			inline typename BufferTy::mapptr map(GLenum acess = GL_READ_WRITE) {
				_bind(); return cache::rdbuf()->map(acess); }

			inline typename BufferTy::mapptr map(GLintptr offset,
				GLenum acess_ext = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT) {
				_bind(); return cache::rdbuf()->map(offset, acess_ext); }

			inline typename BufferTy::mapptr map(GLintptr offset, GLsizeiptr length,
				GLenum acess_ext = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT) {
				_bind(); return cache::rdbuf()->map(offset, length, acess_ext); }

			mytype& operator<<(mytype&(*manip)(mytype&)) { manip(*this); return *this; }
			const mytype& operator>>(const mytype&(*manip)(const mytype&)) const { manip(*this); return *this; }
		};

		template <
			class BufferTy, 
			class CachePolicy, 
			template <class> class BindPolicy>
		class basic_stream : public basic_stream_impl<BufferTy, CachePolicy::impl, BindPolicy> {
		public:
			basic_stream(BufferTy* buf, GLuint pos = NULL) : basic_stream_impl(buf, pos) {}
			GLWL_CTOR_LV_DELETE(basic_stream)
		};

		template <
			class BufferTy,
			class CachePolicy, 
			template <class> class BindPolicy>
		class stream : public basic_stream_impl<BufferTy, CachePolicy::impl, BindPolicy> {
		private:
			typedef basic_stream_impl<BufferTy, CachePolicy::impl, BindPolicy> base;
		public:
			//typedef typename base::unsafe unsafe;

			stream(BufferTy* buf, GLuint pos = NULL) : base(buf, pos) {}
			GLWL_CTOR_LV_DELETE(stream)

			template <typename ElemTy>
			inline void read(size_t size, ElemTy* data) { 
				base::read(size*sizeof(ElemTy), (char*)data); }
			template <typename ElemTy>
			inline void write(size_t size, const ElemTy* data) {
				base::write(size*sizeof(ElemTy), (const char*)data); }

			template <typename ElemTy>
			inline stream& operator<<(const ElemTy& out) { write(1, &out); return *this; }
			template <typename ElemTy>
			inline stream& operator<<(_STD initializer_list<ElemTy> out) { write(out.size(), out.begin()); return *this; }

			template <typename ElemTy, unsigned Size>
			inline stream& operator>>(ElemTy out[Size]) { read(Size*sizeof(ElemTy), out); return *this; }
			template <typename ElemTy>
			inline stream& operator>>(ElemTy& out) { read(sizeof(ElemTy), &out); return *this; }

			inline base& operator<<(base&(*manip)(base&)) { manip(*this); return *this; }
			inline const base& operator>>(const base&(*manip)(const base&)) const { manip(*this); return *this; }
		};

		template <
			class ElemTy,
			class BufferTy, 
			class CachePolicy, 
			template <class> class BindPolicy>
		class elem_stream : public basic_stream_impl<BufferTy, CachePolicy::impl, BindPolicy> {
		private:
			typedef basic_stream_impl<BufferTy, CachePolicy::impl, BindPolicy> base;
		public:
			elem_stream(BufferTy* buf, GLuint pos = NULL) : base(buf, pos) {}
			GLWL_CTOR_LV_DELETE(elem_stream)

			inline void reserve(GLuint size, const ElemTy* data = nullptr) {
				base::reserve(size*sizeof(ElemTy), (const char*)data); }
			inline void resize(GLuint size, const ElemTy* data = nullptr) {
				base::resize(size*sizeof(ElemTy), (const char*)data); }

			inline void read(size_t size, ElemTy* data) { 
				base::read(size*sizeof(ElemTy), (char*)data); }
			inline void write(size_t size, const ElemTy* data) {
				base::write(size*sizeof(ElemTy), (const char*)data); }

			inline GLuint tell() const { return base::tell() / sizeof(ElemTy); }
			inline GLuint count() const { return base::tell() / sizeof(ElemTy); }
			inline void shift(GLuint pos) { base::shift(pos*sizeof(ElemTy)); }
			inline void seek(GLuint pos) { base::seek(pos*sizeof(ElemTy)); }

			inline elem_stream& operator<<(const ElemTy& out) { write(1, &out); return *this; }
			inline elem_stream& operator<<(_STD initializer_list<ElemTy> out) { write(out.size(), out.begin()); return *this; }
			inline elem_stream& operator<<(int out) {
				ElemTy&& val = static_cast<ElemTy&&>(out);
				write(1, &val); return *this; }

			template <unsigned Size>
			inline elem_stream& operator>>(ElemTy out[Size]) { read(Size*sizeof(ElemTy), out); return *this; }
			inline elem_stream& operator>>(ElemTy& out) { read(sizeof(ElemTy), &out); return *this; }

			inline base& operator<<(base&(*manip)(base&)) { manip(*this); return *this; }
			inline const base& operator>>(const base&(*manip)(const base&)) const { manip(*this); return *this; }
		};
	}
};

#endif //__GLWL_CORE_H
