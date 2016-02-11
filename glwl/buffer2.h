#include <cstdio>

#include <utility>
#include <new>

#include <GL\glew.h>
#include <GL\freeglut.h>

#include <glm\common.hpp>

#pragma comment(lib,"OpenGL32.lib")
#pragma comment(lib,"freeglut.lib")
#pragma comment(lib,"glew32.lib")

#include "macro.h"

namespace glwl {
	/*class exception : public _XSTD exception {
	public:
	exception() {}
	template <typename... ArgsTy>
	exception(const char* format, ArgsTy... args) : _XSTD exception(_msg, 0) {
	sprintf_s(_msg, format, args...); }
	private:
	char _msg[512];
	};*/

	enum glbuf_type {
		static_draw = GL_STATIC_DRAW,
		dynamic_draw = GL_DYNAMIC_DRAW,
		stream_draw = GL_STREAM_DRAW,

		vertex = GL_ARRAY_BUFFER,
		bindex = GL_ELEMENT_ARRAY_BUFFER,
		buniform = GL_UNIFORM_BUFFER
	};

	struct glbuf_no_check {
	public:
		enum code { sucess };

		template <typename FTy, typename... ArgsTy>
		inline void construct(FTy f, ArgsTy... args) { f(args...); }
		template <typename FTy, typename... ArgsTy>
		inline void destruct(FTy f, ArgsTy... args) { f(args...); }
		template <typename FTy, typename... ArgsTy>
		inline void call(FTy f, ArgsTy... args) const { f(args...); }

		inline void check_ptr(const void* ptr, const char* msg) {}
		inline void check_size(GLuint size, GLuint expect, const char* msg) {}
		inline void check_type(GLenum type, GLenum expect, const char* msg) {}

		GLenum glerr() { return glGetError(); }
		code error() { return sucess; }
		code exception(code mask) { return sucess; }
		code exception() { return sucess; }
	protected:
		glbuf_no_check() {}
		~glbuf_no_check() {}
	private:
	};

	struct glbuf_std_check {
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
				GLWL_BUF_THROW(glBindBufferRange, "\tType: %s,\n\tBindIdx: %d,\n\tID: %x,\n\tOffset: %x,\n\tSize: %d\n"),
				macro::str(a1), a2, a3, a4, a5, macro::str(_glerr));
			GLWL_BUF_THROW(glCopyBufferSubData, "\tTarget1: %s,\n\tTarget2: %s,\n\tOffset1: %d,\n\tOffset2: %d,\n\tSize: %d\n"),
				macro::str(a1), macro::str(a2), a3, a4, a5, macro::str(_glerr));
			GLWL_BUF_EXCEPT_CALL_END
		}

#undef GLWL_BUF_THROW
#undef GLWL_BUF_EXCEPT_CALL_BEG
#undef GLWL_BUF_EXCEPT_CALL_END

		inline void check_ptr(const void* ptr, const char* msg) const {
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
		}

		GLenum glerr() const { return _glerr; }
		code error() const { return _error; }
		code exception() const { return _mask; }
		code exception(code mask) const { code old = _mask; _mask = mask; return old; }
	protected:
		glbuf_std_check() : _mask(all), _error(sucess), _glerr(GL_NO_ERROR) {}
		~glbuf_std_check() {}
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


	template <class ExceptionPolicy>
	class glbuf_immutable : public ExceptionPolicy {
	public:
		inline void init(GLenum type, GLbitfield flags, GLuint size, const void* data) {
			ExceptionPolicy::call(glBufferStorage, type, size, data, flags);
		}
	protected:
		glbuf_immutable() {}
		~glbuf_immutable() {}
	};
	template <class ExceptionPolicy>
	class glbuf_mutable : public ExceptionPolicy {
	public:
		inline void init(GLenum type, GLenum usage, GLuint size, const void* data) {
			ExceptionPolicy::call(glBufferData, type, size, data, usage);
		}
	protected:
		glbuf_mutable() {}
		~glbuf_mutable() {}
	};


	template <class ExceptionPolicy, template<class> class MutablePolicy>
	struct glbuf_map_none : MutablePolicy<ExceptionPolicy> {
	protected:
		glbuf_map_none() {}
		~glbuf_map_none() {}
	};

	template <class ExceptionPolicy, template<class> class MutablePolicy>
	struct glbuf_map_fast : MutablePolicy<ExceptionPolicy> {
	private:
		typedef ExceptionPolicy checker;
	public:
		typedef void* mapptr;

		inline void unmap(GLenum type) { glUnmapBuffer(type); }
		inline mapptr map(GLenum type, GLenum acess, GLsizeiptr length, GLintptr offset) {
			void* ptr = glMapBufferRange(type, offset, length, acess);
			checker::check_ptr(ptr, "can't map this buffer");
			return ptr;
		}
		inline mapptr map(GLenum type, GLenum acess) {
			void* ptr = glMapBuffer(type, acess);
			checker::check_ptr(ptr, "can't map this buffer");
			return ptr;
		}
	protected:
		glbuf_map_fast() {}
		~glbuf_map_fast() {}
	};

	template <class ExceptionPolicy, template<class> class MutablePolicy>
	struct glbuf_map_safe : MutablePolicy<ExceptionPolicy> {
	private:
		typedef ExceptionPolicy checker;
	public:
		typedef class mapptr {
		public:
			mapptr(void* ptr, GLenum type, glbuf_map_safe& ref)
				: _ptr(ptr), _type(type), _ref(ref) {}
			~mapptr() { _ref.unmap(_type); }
			operator void*() { return _ptr; }
			void unmap() { _ref.unmap(_type); }
		private:
			void* _ptr;
			GLenum _type;
			glbuf_map_safe& _ref;
		};

		inline void unmap(GLenum type) { if (_mapped) checker::call(glUnmapBuffer, type); _mapped = false; }
		inline mapptr map(GLenum type, GLenum acess, GLsizeiptr length, GLintptr offset) {
			_mapped = true;
			void* ptr = glMapBufferRange(type, offset, length, acess);
			checker::check_ptr(ptr, "can't map this buffer");
			return mapptr(ptr, type, *this);
		}
		inline mapptr map(GLenum type, GLenum acess) {
			void* ptr = glMapBuffer(type, acess);
			checker::check_ptr(ptr, "can't map this buffer");
			return mapptr(ptr, type, *this);
		}

		inline bool mapped() { return _mapped; }
	protected:
		~glbuf_map_safe() {}
		glbuf_map_safe() : _mapped(false) {}
	private:
		bool _mapped;
	};

	template <class ExceptionPolicy,
		template<class> class MutablePolicy,
		template<class, template<class> class> class MapPolicy>
	class basic_glbuf : public MapPolicy<ExceptionPolicy, MutablePolicy> {
	private:
		typedef ExceptionPolicy checker;
		typedef const basic_glbuf& cref;
	public:
		typedef typename checker::code errcode;

		basic_glbuf(_STD _Uninitialized) : _id(0) {}
		basic_glbuf() { checker::construct(glGenBuffers, 1, &_id); }
		~basic_glbuf() { checker::destruct(glDeleteBuffers, 1, &_id); }

		basic_glbuf(const basic_glbuf& src) = delete;
		basic_glbuf(basic_glbuf&& src) : _id(src._id) { src._id = 0; }

		inline void bind(GLenum type) const { checker::call(glBindBuffer, type, _id); }
		inline void bind(GLenum type, GLuint index) const {
			checker::call(glBindBufferBase, type, index, _id);
		}
		inline void bind(GLenum type, GLuint index, GLintptr offset, GLsizeiptr length) const {
			checker::call(glBindBufferRange, type, index, _id, offset, length);
		}

		inline void read(GLenum type, GLuint offset, GLuint size, void* data) {
			checker::call(glGetBufferSubData, type, offset, size, data);
		}
		inline void write(GLenum type, GLuint offset, GLuint size, const void* data) {
			checker::call(glBufferSubData, type, offset, size, data);
		}
		inline void copy(GLuint offset, cref src, GLuint src_offset, GLuint write_len) {
			bind(GL_COPY_WRITE_BUFFER);
			src.bind(GL_COPY_READ_BUFFER);
			checker::call(glCopyBufferSubData, GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
				offset, src_offset, write_len);
		}

		GLuint id() { return _id; }

		inline GLenum error() { return checker::error(); }
		inline errcode exception(errcode mask) { return checker::exception(mask); }
		inline errcode exception() { return checker::exception(); }
	private:
		GLuint _id;
	};

	template <class ExceptionPolicy = glbuf_std_check,
		template<class> class MutablePolicy = glbuf_mutable,
		template<class, template<class> class> class MapPolicy = glbuf_map_safe>
	class glbuf :
		private basic_glbuf<ExceptionPolicy, MutablePolicy, MapPolicy> {
	private:
		typedef basic_glbuf<ExceptionPolicy, MutablePolicy, MapPolicy> base;
		typedef MutablePolicy<ExceptionPolicy> initp;
		typedef ExceptionPolicy checker;
		typedef const glbuf& cref;
	public:
		typedef typename base::errcode errcode;

		~glbuf() {}

		glbuf() : glbuf(0, 0) {}
		glbuf(_STD _Uninitialized) : basic_glbuf(_STD _Noinit), glbuf(0, 0) {}
		glbuf(GLenum type, GLenum usage) : _len(0), _type(type), _usage(usage) {}
		glbuf(GLenum type, GLenum usage, GLuint size, const void* data = nullptr)
			: _len(size), _type(type), _usage(usage) {
			bind();
			reserve(size, data);
		}

		glbuf(glbuf&& src) : basic_glbuf(_STD move(src)), _len(src._len),
			_type(src._type), _usage(src._usage) {}
		glbuf(const glbuf& src) : glbuf(src._type, src._usage, src._len) {
			copy(0, src, 0, src._len);
		}

		glbuf& operator=(glbuf&& src) {
			this->~glbuf();
			new(this) glbuf(_STD move(src));
			return *this;
		}
		glbuf& operator=(const glbuf& src) {
			this->~glbuf();
			new(this) glbuf(src);
			return *this;
		}

		inline void bind() const { base::bind(_type); }
		inline void bind(GLuint index) const { base::bind(_type, index); }
		inline void bind(GLuint index, GLintptr offset) const {
			base::bind(_type, index, offset, _len);
		}
		inline void bind(GLuint index, GLintptr offset, GLsizeiptr length) const {
			checker::check_size(_len, length);
			base::bind(_type, index, offset, length);
		}

		inline void unmap() { base::unmap(_type); }
		inline typename base::mapptr map(GLenum acess = GL_READ_WRITE) {
			return base::map(_type, acess);
		}
		inline typename base::mapptr map(GLintptr offset,
			GLenum acess_ext = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT) {
			return base::map(_type, acess_ext, _len, offset);
		}
		inline typename base::mapptr map(GLintptr offset, GLsizeiptr length,
			GLenum acess_ext = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT) {
			return base::map(_type, acess_ext, length, offset);
		}

		inline void read(GLuint offset, GLuint size, void* data) {
			base::read(_type, offset, size, data);
		}
		inline void write(GLuint offset, GLuint size, const void* data) {
			base::write(_type, offset, size, data);
		}
		inline void copy(GLuint offset, cref src, GLuint src_offset, GLuint write_len) {
			checker::check_size(offset + write_len, _len);
			checker::check_size(src_offset + write_len, src._len);
			base::copy(offset, src, src_offset, write_len);
		}

		inline void reserve(GLuint size, const void* data = nullptr) {
			initp::init(_type, _usage, size, data);
			_len = size;
		}

		inline void resize(GLuint size) {
			GLuint write_len = size < _len ? size : _len;
			glbuf buf(_type, _usage, size);
			buf.base::copy(0, *this, 0, write_len);
			*this = _STD move(buf);
		}

		void resize(GLuint size, const void* data) {
			GLuint offset = _len;
			resize(size);
			if (!data) return;
			write(offset, size - offset, data);
		}

		inline GLuint id() { return base::id(); }

		inline GLenum error() { return checker::error(); }
		inline errcode exception(errcode mask) { return checker::exception(mask); }
		inline errcode exception() { return checker::exception(); }

		inline GLenum type() { return _type; }
		inline GLenum type(GLenum new_type) { GLenum old = _type; _type = new_type; return old; }

		inline GLenum usage() { return _type; }
		inline GLenum usage(GLenum new_usage) { GLenum old = _usage; _usage = new_usage; return old; }
	private:
		GLuint _len;
		GLenum _type;
		GLenum _usage;
	};

	class bufstream_no_cached_unsafe {
	public:
		inline GLintptr tell() { return _offset; }
		inline void seek(GLintptr pos) { _offset = pos; }
		inline void shift(GLintptr offset) { _offset += offset; }
	protected:
		template <class BufType>
		inline void write(BufType* buf, GLuint size, const char* data) {
			buf->write(_offset, size, data); }
		template <class BufType>
		inline void flush(BufType* buf) {}
		inline void clear() { _offset = 0; }

		bufstream_no_cached_unsafe() {}
		~bufstream_no_cached_unsafe() {}
	private:
		GLintptr _offset;
	};

	class bufstream_no_cached : public bufstream_no_cached_unsafe {
	private:
		typedef bufstream_no_cached_unsafe base;
	public:
		inline void seek(GLintptr pos) { 
			if (pos > _endbuf) throw _GLWL exception(
				"bufstream seek: out of range [pos: %d, bufsize %d].", pos, _endbuf);
			base::seek(pos); 
		}
		inline void shift(GLintptr offset) {
			GLintptr pos = base::tell() + offset;
			if (pos > _endbuf) throw _GLWL exception(
				"bufstream shift: out of range [pos: %d, bufsize %d].", pos, _endbuf);
			if (pos < 0) throw _GLWL exception(
				"bufstream shift: out of range [pos: %d < 0].", pos);
			base::seek(pos); 
		}
	protected:
		template <class BufType>
		inline void write(BufType* buf, GLuint size, const char* data) {
			_endbuf += size, base::write(buf, size, data); }
		template <class BufType>
		inline void flush(BufType* buf) {}
		inline void clear() { _endbuf = 0, base::clear(); }

		bufstream_no_cached() {}
		~bufstream_no_cached() {}
	private:
		GLintptr _endbuf;
	};

	template <GLuint CacheSize>
	class bufstream_cached_fast {
	public:
		inline GLintptr tell() { return _offset + (_last - _base); }

		inline void seek_near(GLintptr pos) { _last = _base + pos - _offset; }
		inline void seek_far(GLintptr pos) { _flush(); _offset = pos; }

		inline void shift_near(GLintptr offset) { _last += offset; }
		inline void shift_far(GLintptr offset) { _flush(); _offset += offset; }
	protected:
		template <class BufType>
		inline void write(BufType* buf, GLuint size, const char* data) {
			_STD memcpy(_last, data, size);
			_last += size;
		}
		template <class BufType>
		inline void flush(BufType* buf) {
			GLuint size = _last - _base;
			buf->write(_offset, size, _base);
			_offset += size;
			_last = _base;
		}
		inline void clear() { _offset = 0, _last = _base; }

		bufstream_cached_fast() : _last(_base), _end(_base + _CacheSize) {}
		~bufstream_cached_fast() {}
	private:
		template <class BufType>
		inline void _flush(BufType* buf) {
			GLuint size = _last - _base;
			buf->write(_offset, size, _base);
			_last = _base;
		}
		GLuint _offset;

		char _base[_CacheSize];
		char* _last;
		char* _end;
	};

	template <GLuint CacheSize>
	class bufstream_cached :
		private bufstream_cached_fast<CacheSize> {
	private:
		typedef bufstream_cached_fast<CacheSize> base;
	public:
		inline GLuint tell() { return base::tell(); }
		inline void seek(GLintptr pos) {
			GLint offset = pos - _offset;
			char* new_last = _last + offset;
			if (new_last < _base || new_last > _end) {
				base::seek_far(pos); return;
			}
			_last = new_last;
		}
		inline void shift(GLintptr offset) {
			char* new_last = _last + offset;
			if (new_last < _base || new_last > _end) {
				base::shift_far(offset); return;
			}
			_last = new_last;
		}
	protected:
		template <class BufType>
		inline void write(BufType* buf, GLuint size, const char* data) {
			if (size > _CacheSize) { _buf.write(_offset, size, data); return; }
			GLuint capacity = _end - _last;
			GLint tail = size - capacity;
			if (tail < 0) { base::write(buf, size, data); return; }
			base::write(buf, capacity, data);
			data += capacity;
			buf->write(_offset, _CacheSize, _base);
			_offset += _CacheSize;
			base::write(buf, tail, data);
		}
		inline void flush() { base::flush(); }

		bufstream_cached() {}
		~bufstream_cached() {}
	};

	template <class BufferTy, class CachePolicy>
	class basic_bufstream : public CachePolicy {
	public:
		~basic_bufstream() {}

		basic_bufstream(BufferTy& buf) : _data(buf) {}
		basic_bufstream(basic_stream&& src) : _data(src._data) { _STD memcpy(this, &src, sizeof(basic_stream)); }
		GLWL_CTOR_LV_DELETE(basic_stream)
		GLWL_ASSIG_RV_STD(basic_stream)

		inline void reserve(GLuint size, const char* data = nullptr) {
			_data.bind(); _data.reserve(size, data); }
		inline void resize(GLuint size, const char* data = nullptr) {
			_data.bind(); _data.resize(size, data); }
		inline void write(size_t size, const char* data) {
			CachePolicy::write(size, data); }
		inline void flush() {
			CachePolicy::flush(_buf); }

		inline void clear() {
			_data.bind();
			_data.reserve(0);
			CachePolicy::clear();
		}

		basic_stream& operator<<(basic_stream&(*manip)(basic_stream&)) { manip(*this); return *this; }
		const basic_stream& operator>>(const basic_stream&(*manip)(const basic_stream&)) const { manip(*this); return *this; }
	};
};
