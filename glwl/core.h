#ifndef __GLWL_CORE_H
#define __GLWL_CORE_H

#include <exception>
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
	this->~myclass(); \
	new(this) myclass(_STD move(src)); \
	return *this; }
#define GLWL_ASSIG_LV_STD(myclass) \
	myclass& operator=(const myclass& src) {\
	this->~myclass(); \
	new(this) myclass(src); \
	return *this; }
#define GLWL_CTOR_RV_DELETE(myclass) \
	myclass(myclass&& src) = delete; \
	myclass& operator=(myclass&& src) = delete;
#define GLWL_CTOR_LV_DELETE(myclass) \
	myclass(const myclass& src) = delete; \
	myclass& operator=(const myclass& src) = delete;


#define GLWL_CASE_MACRO(def) case def: return #def;

namespace glwl {
	namespace macro {
		const char* str(GLenum val) {
			switch (val) {
				GLWL_CASE_MACRO(GL_NO_ERROR)
					GLWL_CASE_MACRO(GL_INVALID_ENUM)
					GLWL_CASE_MACRO(GL_INVALID_VALUE)
					GLWL_CASE_MACRO(GL_INVALID_OPERATION)
					GLWL_CASE_MACRO(GL_STACK_OVERFLOW)
					GLWL_CASE_MACRO(GL_STACK_UNDERFLOW)
					GLWL_CASE_MACRO(GL_OUT_OF_MEMORY)
					GLWL_CASE_MACRO(GL_INVALID_FRAMEBUFFER_OPERATION)

					GLWL_CASE_MACRO(GL_STREAM_DRAW)
					GLWL_CASE_MACRO(GL_STREAM_READ)
					GLWL_CASE_MACRO(GL_STREAM_COPY)
					GLWL_CASE_MACRO(GL_STATIC_DRAW)
					GLWL_CASE_MACRO(GL_STATIC_READ)
					GLWL_CASE_MACRO(GL_STATIC_COPY)
					GLWL_CASE_MACRO(GL_DYNAMIC_DRAW)
					GLWL_CASE_MACRO(GL_DYNAMIC_READ)
					GLWL_CASE_MACRO(GL_DYNAMIC_COPY)

					GLWL_CASE_MACRO(GL_ARRAY_BUFFER)
					GLWL_CASE_MACRO(GL_ELEMENT_ARRAY_BUFFER)
					GLWL_CASE_MACRO(GL_UNIFORM_BUFFER)

					/*GLWL_CASE_MACRO(GL_STATIC_DRAW)
					GLWL_CASE_MACRO(GL_STATIC_READ)
					GLWL_CASE_MACRO(GL_STATIC_COPY)
					GLWL_CASE_MACRO(GL_DYNAMIC_DRAW)
					GLWL_CASE_MACRO(GL_DYNAMIC_READ)
					GLWL_CASE_MACRO(GL_DYNAMIC_COPY)*/

			default: return "<unknown>";
			}
		}
	};

#undef GLWL_CASE_MACRO

	class exception : public _XSTD exception {
	public:
		exception() {}
		template <typename... ArgsTy>
		exception(const char* format, ArgsTy... args) : _XSTD exception(_msg, 0) {
			sprintf_s(_msg, format, args...);
		}
	private:
		char _msg[512];
	};

	template <class _Ty>
	using list = _STD initializer_list<_Ty>;

	namespace buf {
		enum type {
			static_draw = GL_STATIC_DRAW,
			dynamic_draw = GL_DYNAMIC_DRAW,
			stream_draw = GL_STREAM_DRAW,

			vertex = GL_ARRAY_BUFFER,
			index = GL_ELEMENT_ARRAY_BUFFER,
			uniform = GL_UNIFORM_BUFFER
		};
		namespace a {
			struct no_check {
			public:
				enum code { sucess };

				template <typename FTy, typename... ArgsTy>
				inline void construct(FTy f, ArgsTy... args) { f(args...); }
				template <typename FTy, typename... ArgsTy>
				inline void destruct(FTy f, ArgsTy... args) { f(args...); }
				template <typename FTy, typename... ArgsTy>
				inline void call(FTy f, ArgsTy... args) const { f(args...); }

				template <typename... MsgTypes>
				inline void check(bool err, MsgTypes... msg) const {}

				//inline void check_ptr(const void* ptr, const char* msg) {}
				//inline void check_size(GLuint size, GLuint expect, const char* msg) {}
				//inline void check_type(GLenum type, GLenum expect, const char* msg) {}

				GLenum glerr() { return glGetError(); }
				code error() { return sucess; }
				code exception(code mask) { return sucess; }
				code exception() { return sucess; }
			protected:
				no_check() {}
				~no_check() {}
			private:
			};


			struct std_check {
			public:
				enum code {
					sucess = 0x0000,
					fail = 0xffff,

					bad_glBindBuffer = 0x0001,
					bad_glBufferData = 0x0002,
					bad_glUnmapBuffer = 0x0004,
					bad_glBufferStorage = 0x0008,
					bad_glBufferSubData = 0x0010,
					bad_glBindBufferBase = 0x0020,
					bad_glBindBufferRange = 0x0040,
					bad_glCopyBufferSubData = 0x0080,

					none = 0x00000000,
					all = 0xffffffff
				};

				template <typename FTy, typename... ArgsTy>
				inline void construct(FTy f, ArgsTy... args) { f(args...); }
				template <typename FTy, typename... ArgsTy>
				inline void destruct(FTy f, ArgsTy... args) { f(args...); }
				
#define GLWL_BUF_THROW(proc, format) \
	case bad_##proc: throw _GLWL exception(#proc##" function failure. Arguments: {\n"##format##"} \nError message : %s"
#define GLWL_BUF_EXCEPT_CALL_BEG \
	_glerr = glGetError(); \
				if (_glerr) {\
				setf(reinterpret_cast<const void*>(f)); \
				if (!(_mask & _error)) return; \
				switch (_error) {
#define GLWL_BUF_EXCEPT_CALL_END }}

				template <typename FTy, typename A1Ty>
				inline void call(FTy f, A1Ty a1) const {
					f(a1);
					GLWL_BUF_EXCEPT_CALL_BEG
					GLWL_BUF_THROW(glUnmapBuffer, "\tType: %s\n"),
						macro::str(a1), macro::str(_glerr));
					GLWL_BUF_EXCEPT_CALL_END
				}
				template <typename FTy, typename A1Ty, typename A2Ty>
				inline void call(FTy f, A1Ty a1, A2Ty a2) const {
					f(a1, a2);
					GLWL_BUF_EXCEPT_CALL_BEG
					GLWL_BUF_THROW(glBindBuffer, "\tType: %s,\n\tID: %d\n"),
						macro::str(a1), a2, macro::str(_glerr));
					GLWL_BUF_EXCEPT_CALL_END
				}
				template <typename FTy, typename A1Ty, typename A2Ty, typename A3Ty>
				inline void call(FTy f, A1Ty a1, A2Ty a2, A3Ty a3) const {
					f(a1, a2, a3);
					GLWL_BUF_EXCEPT_CALL_BEG
					GLWL_BUF_THROW(glBindBufferBase, "\tType: %s,\n\tBindIdx: %d,\n\tID: %d\n"),
						macro::str(a1), a2, a3, macro::str(_glerr));
					GLWL_BUF_EXCEPT_CALL_END
				}
				template <typename FTy, typename A1Ty, typename A2Ty, typename A3Ty, typename A4Ty>
				inline void call(FTy f, A1Ty a1, A2Ty a2, A3Ty a3, A4Ty a4) const {
					f(a1, a2, a3, a4);
					GLWL_BUF_EXCEPT_CALL_BEG
					GLWL_BUF_THROW(glBufferData, "\tType: %s,\n\tSize: %d,\n\tData: %x,\n\tUsage: %s\n"),
						macro::str(a1), a2, a3, macro::str((GLenum)a4), macro::str(_glerr));
					GLWL_BUF_THROW(glBufferStorage, "\tType: %s,\n\tSize: %d,\n\tData: %x,\n\tFlags: %x\n"),
						macro::str(a1), a2, a3, a4, macro::str(_glerr));
					GLWL_BUF_THROW(glBufferSubData, "\tType: %s,\n\tOffset: %d,\n\tSize: %d,\n\tData: %x\n"),
						macro::str(a1), a2, a3, a4, macro::str(_glerr));
					GLWL_BUF_EXCEPT_CALL_END
				}

				template <typename FTy, typename A1Ty, typename A2Ty, typename A3Ty, typename A4Ty, typename A5Ty>
				inline void call(FTy f, A1Ty a1, A2Ty a2, A3Ty a3, A4Ty a4, A5Ty a5) const {
					f(a1, a2, a3, a4, a5);
					GLWL_BUF_EXCEPT_CALL_BEG
					GLWL_BUF_THROW(glBindBufferRange, "\tType: %s,\n\tBindIdx: %d,\n\tID: %x,\n\tOffset: %d,\n\tSize: %d\n"),
						macro::str(a1), a2, a3, a4, a5, macro::str(_glerr));
					GLWL_BUF_THROW(glCopyBufferSubData, "\tTarget1: %s,\n\tTarget2: %s,\n\tOffset1: %d,\n\tOffset2: %d,\n\tSize: %d\n"),
						macro::str(a1), macro::str(a2), a3, a4, a5, macro::str(_glerr));
					GLWL_BUF_EXCEPT_CALL_END
				}

#undef GLWL_BUF_THROW
#undef GLWL_BUF_EXCEPT_CALL_BEG
#undef GLWL_BUF_EXCEPT_CALL_END

				template <typename... MsgTypes>
				inline void check(bool err, MsgTypes... msg) const {
					if (err) throw _GLWL exception(msg...); }

				/*inline void check_ptr(const void* ptr, const char* msg) const {
					if (ptr) return;
					throw _GLWL exception("Invalid pointer: %s", msg);
				}
				inline void check_size(GLuint size, GLuint expect, const char* msg) const {
					if (size == expect) return;
					throw _GLWL exception("Invalid size: %s", msg);
				}
				inline void check_type(GLenum type, GLenum expect, const char* msg) const {
					if (type == expect) return;
					throw _GLWL exception("Types mismath: %s", msg);
				}*/

				GLenum glerr() const { return _glerr; }
				code error() const { return _error; }
				code exception() const { return _mask; }
				code exception(code mask) const { code old = _mask; _mask = mask; return old; }
			protected:
				std_check() : _mask(all), _error(sucess), _glerr(GL_NO_ERROR) {}
				~std_check() {}
			private:
				inline void setf(const void* f) const {
					if (f == glBindBuffer)			_error = bad_glBindBuffer;
					else if (f == glBufferData)			_error = bad_glBufferData;
					else if (f == glUnmapBuffer)		_error = bad_glUnmapBuffer;
					else if (f == glBufferStorage)		_error = bad_glBufferStorage;
					else if (f == glBufferSubData)		_error = bad_glBufferSubData;
					else if (f == glBindBufferBase)		_error = bad_glBindBufferBase;
					else if (f == glBindBufferRange)	_error = bad_glBindBufferRange;
					else if (f == glCopyBufferSubData)	_error = bad_glCopyBufferSubData;
					else _error = fail;
				}

				mutable GLenum _glerr;
				mutable code _mask;
				mutable code _error;
			};
		};

		namespace b {
			template <class ExceptionPolicy>
			class immutable : public ExceptionPolicy {
			public:
				inline void init(GLenum type, GLbitfield flags, GLuint size, const void* data) {
					ExceptionPolicy::call(glBufferStorage, type, size, data, flags);
				}
			protected:
				immutable() {}
				~immutable() {}
			};
			template <class ExceptionPolicy>
			class dynamic : public ExceptionPolicy {
			public:
				inline void init(GLenum type, GLenum usage, GLuint size, const void* data) {
					ExceptionPolicy::call(glBufferData, type, size, data, usage);
				}
			protected:
				dynamic() {}
				~dynamic() {}
			};
		};

		namespace c {
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
			basic_raw() { checker::construct(glGenBuffers, 1, &_id); }
			~basic_raw() { checker::destruct(glDeleteBuffers, 1, &_id); }

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

		template <class ExceptionPolicy = a::std_check,
			template<class> class MutablePolicy = b::dynamic,
			template<class, template<class> class> class MapPolicy = c::map_safe>
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
					inline GLuint cache_capacity() const { return NULL; }
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
					inline GLuint cache_capacity() const { return CacheSize; }
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
						_STD memcpy(_last + offset, data, size); }
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
							return;
						}
						GLuint capacity = _end - _last;
						GLint tail = size - capacity;
						if (tail < 0) { unsafe::write(size, data); return; }
						unsafe::write(capacity, data);
						data += capacity;
						_bufwrite(CacheSize, _base);
						_last = _base;
						unsafe::write(tail, data);
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
