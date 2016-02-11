#pragma once

#include <stdio.h>
#include <io.h>

#include <map>

#include "basic.h"

namespace glwl {
	class blob {
	private:
		void* _data;
		size_t _size;
	public:
		~blob() { if (_data) free(_data); }
		blob() : _data(nullptr), _size(0) {}
		blob(const char* filename) {
			FILE* file;
			if (fopen_s(&file, filename, "rb")) throw 0;
			_size = _filelength(_fileno(file));
			_data = malloc(_size + 1);
			_size = fread(_data, 1, _size, file);
			((char*)_data)[_size] = 0;
		}
		blob(size_t size, const void* data) : _size(size) {
			_data = malloc(size);
			if (data) _STD memcpy(_data, data, size);
		}
		blob(size_t size, void* data, int) : _size(size), _data(data) {}

		blob(blob&& src) : _data(src._data), _size(src._size) {
			src._data = nullptr;
		}
		blob& operator=(blob&& Right) {
			this->~blob();
			new(this) blob(_STD forward<blob>(Right));
			return *this;
		}

		void* data() { return _data; }
		const void* data() const { return _data; }
		size_t size() { return _size; }
	};

	class cache {
	public:
		cache() : cache(0, nullptr) {}
		cache(size_t block_size) : cache(block_size, (char*)malloc(block_size)) {}
		cache(size_t block_size, char* mem) : _base(mem), _last(mem),
			_end(mem + block_size), _blocksize(block_size) {}
		~cache() { ::free(_base); }

		cache(const cache& src) : cache(src.capacity()) {
			_STD memcpy(_base, src._base, src.capacity());
		}

		GLWL_CTOR_RV_STD(cache)
		GLWL_ASSIG_RV_STD(cache)
		GLWL_ASSIG_LV_STD(cache)

		inline void reserve(size_t size) { *this = _STD move(cache(size)); }
		inline void resize(size_t size) { _base = (char*) ::realloc(_base, size); }
		inline void expand(size_t size) { resize(this->size() + size); }

		inline char* data() { return _base; }
		inline const char* data() const { return _base; }
		inline size_t size() const { return _last - _base; }
		inline size_t free() const { return _end - _last; }
		inline size_t capacity() const { return _end - _base; }

		size_t write(size_t size, const char* data) {
			if (size > capacity()) { return 0; }
			const size_t len = free();
			const int tail = size - len;
			if (tail < 0) { put(size, data); return size; }
			put(len, data); return len;
		}

		inline bool seek(int offset) {
			char* off = _last + offset;
			if (off >= _end) return true;
			if (off < _base) return true;
			_last = off;
			return false;
		}

		inline void put(size_t size, const char* data) {
			_STD memcpy(_last, data, size);
			_last += size;
		}

		inline void push(size_t size, const char* data) {
			const size_t len = free();
			if (len < size) expand(size + _blocksize);
			_STD memcpy(_last, data, size);
			_last += size;
		}

		inline void clear() { _last = _base; }
	private:
		size_t _blocksize;
		char* _base;
		char* _last;
		char* _end;
	};

	class basic_buffer : public unknown {
	public:
		~basic_buffer() { if (_id) glDeleteBuffers(1, &_id); }

		basic_buffer(_STD _Uninitialized) : _type(0), _usage(0), _len(0) {}

		basic_buffer() : basic_buffer(0, 0) {}
		basic_buffer(GLenum type, GLenum usage) : _type(type), _usage(usage), _len(0){
			glGenBuffers(1, &_id); }

		basic_buffer(const basic_buffer& src) : basic_buffer(src._type, src._usage) {
			if (!src._len) return;
			reserve(src._type, src._usage, src._len);
			src.bind(GL_COPY_READ_BUFFER);
			bind(GL_COPY_WRITE_BUFFER);
			unknown::call(glCopyBufferSubData, GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, _len);
		}

		GLWL_CTOR_RV_STD(basic_buffer)
		GLWL_ASSIG_RV_STD(basic_buffer)
		GLWL_ASSIG_LV_STD(basic_buffer)

		inline void bind() const { bind(_type); }
		inline void bind(GLenum type) const {
			unknown::call(glBindBuffer, type, _id);
		}

		inline void* map(GLenum acess) { return glMapBuffer(_type, acess); }
		inline void unmap() { glUnmapBuffer(_type); }

		void reserve(GLenum type, GLenum usage, GLuint size, const void* data = nullptr) {
			unknown::call(glBufferData, type, size, data, usage);
			_len = size, _type = type, _usage = usage;
		}

		void resize(GLuint size) {
			GLuint write_len = size < _len ? size : _len;
			basic_buffer buf;
			buf.bind(_type);
			buf.reserve(_type, _usage, size, nullptr);

			buf.bind(GL_COPY_WRITE_BUFFER);
			bind(GL_COPY_READ_BUFFER);

			unknown::call(glCopyBufferSubData, GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, write_len);
			*this = _STD move(buf);
		}

		void resize(GLuint size, const void* data) { 
			GLuint offset = _len;
			resize(size); 
			write(offset, size - offset, data);
		}

		void read(GLuint offset, GLuint size, const void* data) {}
		void write(GLuint offset, GLuint size, const void* data) { 
			unknown::call(glBufferSubData, _type, offset, size, data); }

		inline GLuint size() const { return _len; }
		inline GLenum type() const { return _type; }
		inline GLenum usage() const { return _usage; }
	private:
		GLuint _len;
		GLenum _type;
		GLenum _usage;
	};

	struct source {
		GLuint off;
		basic_buffer& buf;
		source(basic_buffer& buffer, GLuint offset) : buf(buffer), off(offset) {}
	};

	class buffer : public basic_buffer {
	public:
		buffer() : basic_buffer(_STD _Noinit) {}
		buffer(GLenum type, GLenum usage) : basic_buffer(type, usage) {}
		buffer(GLenum type, GLenum usage, GLuint size, const void* data = nullptr)
				: basic_buffer(type, usage) {
			basic_buffer::bind();
			basic_buffer::reserve(type, usage, size, data);
		}

		buffer(buffer&& src) : basic_buffer(_STD forward<basic_buffer>(src)) {}
		buffer& operator=(buffer&& src) {
			(basic_buffer&)*this = _STD forward<basic_buffer>(src);
			return *this;
		}
		buffer(const buffer& src) : basic_buffer(src) {}
		buffer& operator=(const buffer& src) {
			(basic_buffer&)*this = src;
			return *this;
		}


		void expand(int size) { basic_buffer::resize(basic_buffer::size() + size); }

		void reserve(GLuint size, const void* data = nullptr) { 
			basic_buffer::reserve(type(), usage(), size, data); }
		void flush(void* data) { 
			basic_buffer::read(0, basic_buffer::size(), data); }
		void update(const void* data) { 
			basic_buffer::write(0, basic_buffer::size(), data); }
	};

	class basic_stream {
	public:
		~basic_stream() {}

		basic_stream(buffer& buf, size_t cache_size = BUFSIZ) : _data(buf), _offset(0),
			_cache(cache_size) {}
		basic_stream& operator=(buffer& buf) { 
			this->~basic_stream();
			new(this) basic_stream(buf);
			return *this; 
		}

		basic_stream(basic_stream&& src) : _data(src._data) { _STD memcpy(this, &src, sizeof(basic_stream)); }
		GLWL_CTOR_LV_DELETE(basic_stream)
		GLWL_ASSIG_RV_STD(basic_stream)

		inline void reserve(GLuint size, const char* data = nullptr) {
			_data.bind(); _data.reserve(size, data); }
		inline void resize(GLuint size, const char* data = nullptr) {
			_data.bind(); _data.resize(size, data); }
		inline void reset(buffer& buf) { *this = buf; }

		void read(size_t size, char* data) const {
			_data.bind();
			_data.read(_offset, size, data);
			_offset += size;
		}

		void write(size_t size, const char* data) {
			size_t write_len = _cache.write(size, data);
			size_t tail = size - write_len;
			if (tail) {
				flush();
				if (tail > _cache.capacity()) _flush(tail, data + write_len);
				else write(tail, data + write_len);
			}
		}

		inline void clear() {
			_offset = 0;
			_data.bind();
			_data.reserve(0);
			_cache.clear();
		}

		inline void flush() {
			_flush(_cache.size(), _cache.data());
			_cache.clear();
		}

		inline buffer* rdbuf() { return &_data; }
		inline const buffer* rdbuf() const { return &_data; }

		inline GLuint tell() const { return _offset + _cache.size(); }
		inline void seek(GLuint pos) {
			if (!_cache.seek(pos - _offset - _cache.size())) return;
			flush(); _offset = pos;
		}

		basic_stream& operator<<(basic_stream&(*manip)(basic_stream&)) { manip(*this); return *this; }
		const basic_stream& operator>>(const basic_stream&(*manip)(const basic_stream&)) const { manip(*this); return *this; }
	private:
		inline void _flush(size_t size, const void* data) {
			if (!size) return;
			_data.bind();
			int len = size + _offset - _data.size();
			if (len > 0) _data.expand(len);
			_data.write(_offset, size, data);
			_offset += size;
		}

		cache _cache;
		buffer& _data;
		mutable GLuint _offset;
	};
	
	template <typename _Ty>
	class stream : public basic_stream {
	public:
		stream(buffer& buf, size_t cache_size = BUFSIZ) : basic_stream(buf, cache_size) {}

		stream(stream&& src) : basic_stream(_STD move(src)) {}
		GLWL_CTOR_LV_DELETE(stream)
		GLWL_ASSIG_RV_STD(stream)

		inline void reserve(GLuint size, const _Ty* data = nullptr) { basic_stream::reserve(size*sizeof(_Ty), (const char*)data); }
		inline void resize(GLuint size, const _Ty* data = nullptr) { basic_stream::resize(size*sizeof(_Ty), (const char*)data); }

		inline void read(size_t size, _Ty* data) { basic_stream::read(size*sizeof(_Ty), (char*)data); }
		inline void write(size_t size, const _Ty* data) { basic_stream::write(size*sizeof(_Ty), (const char*)data); }

		inline GLuint tell() const { return basic_stream::tell() / sizeof(_Ty); }
		inline GLuint count() const { return basic_stream::tell() / sizeof(_Ty); }
		inline void seek(GLuint pos) { basic_stream::seek(pos*sizeof(_Ty)); }

		inline stream& operator<<(const _Ty& out) { write(1, &out); return *this; }
		inline stream& operator<<(_STD initializer_list<_Ty> out) { write(out.size(), out.begin()); return *this; }
		inline stream& operator<<(int out) {
			_Ty&& val = static_cast<_Ty&&>(out);
			write(1, &val); return *this;
		}

		inline stream& operator>>(_Ty& out) { read(sizeof(_Ty), &out); return *this; }

		inline basic_stream& operator<<(basic_stream&(*manip)(basic_stream&)) { manip(*this); return *this; }
		inline const basic_stream& operator>>(const basic_stream&(*manip)(const basic_stream&)) const { manip(*this); return *this; }
	};

	basic_stream& endl(basic_stream& os) {
		os.flush();
		return os; }

	template <typename _Ty>
	basic_stream& operator<<(basic_stream& os, reserve&& manip) {
		os.reserve(manip.count, (const _Ty*) manip.data); 
		return os; }
	template <typename _Ty>
	basic_stream& operator<<(basic_stream& os, offset&& manip) {
		os.seek(manip.val);
		return os; }

	struct basic_ibo {
		virtual void bind() const = 0;
		virtual GLenum type() const = 0;
		virtual GLuint count() const = 0;
	};

	template <typename IndexTy>
	class ibo : public stream<IndexTy>, public basic_ibo {
	public:
		ibo() : stream(_data), _data(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW) {}
		ibo(GLenum usage) : stream(_data), _data(GL_ELEMENT_ARRAY_BUFFER, usage) {}
		ibo(GLenum usage, GLuint stride, GLuint count, const void* data = nullptr)
			: stream(_data), _data(GL_ELEMENT_ARRAY_BUFFER, usage, stride*count, data) {stream::seek(count);}
		ibo(buffer& vb, size_t bufsize) : stream(vb, bufsize) {}

		ibo(ibo&& src) : stream(_STD move(src)) {}
		GLWL_ASSIG_RV_STD(ibo)
		ibo(const ibo& src) : stream(_data), _data(src._data) {}
		GLWL_ASSIG_LV_STD(ibo)

		inline void bind() const override { _data.bind(); }
		inline GLenum type() const override { return glwl::type<IndexTy>::id; }
		inline GLuint count() const override { return stream::count(); }

		inline void draw() const { draw(stream::count(), NULL); }
		inline void draw(GLuint count, GLuint start_index = NULL) const {
			glDrawElements(mode, count, glwl::type<IndexTy>::id, (void*)start_index); }

		GLenum mode = GL_TRIANGLES;
	private:
		buffer _data;
	};

	template <typename AttribTy>
	class vbo : public stream<AttribTy> {
	public:
		vbo() : stream(_data), _data(GL_ARRAY_BUFFER, GL_STATIC_DRAW) {}
		vbo(GLenum usage) : stream(_data), _data(GL_ARRAY_BUFFER, usage) {}
		vbo(GLenum usage, GLuint size, const void* data = nullptr)
			: stream(_data), _data(GL_ARRAY_BUFFER, usage, size, data) {}
		vbo(buffer& vb, size_t bufsize) : stream(vb, bufsize) {}

		vbo(vbo&& src) : stream(_STD move(src)) {}
		GLWL_ASSIG_RV_STD(vbo)
		vbo(const vbo& src) : stream(_data), _data(src) {}
		GLWL_ASSIG_LV_STD(vbo)
	private:
		buffer _data;
	};

	class ubo : public buffer {
	public:
		ubo() : buffer(GL_UNIFORM_BUFFER, GL_STATIC_DRAW), _offset(0) {}
		ubo(GLenum usage) : buffer(GL_UNIFORM_BUFFER, usage), _offset(0) {}
		ubo(GLenum usage, GLuint size, const void* data = nullptr) 
			: buffer(GL_UNIFORM_BUFFER, usage, size, data), _offset(0) {}

		ubo(ubo&& src) : buffer(_STD move(src)), _offset(src._offset) {}
		ubo(const ubo& src) : buffer(src), _offset(0) {}
		GLWL_ASSIG_RV_STD(ubo)
		GLWL_ASSIG_LV_STD(ubo)

		void bind() const { _offset = 0; buffer::bind(); }

		template <typename _Ty>
		ubo& operator<<(const _Ty& value) {
			_offset += sizeof(_Ty);
			if (_offset <= buffer::size()) buffer::write(_offset, sizeof(_Ty), &value);
			_offset -= sizeof(_Ty);
			throw _GLWL exception("UBO exception. Out of range!");
		}
		ubo& operator<<(offset&& manip) {
			_offset = manip.val;
			return *this;
		}
		ubo& operator[](GLuint offset) {
			_offset = offset;
			return *this;
		}
	private:
		mutable GLuint _offset;
	};

	class uniform : public unknown {
	private:
		struct value {
			const char* name;
			GLuint id;
			GLint size;
			GLint stride;
			GLint offset;
		};

		class baseval {
		public:
			baseval() {}
			baseval(value* val, ubo* s) : _val(val), _buf(s) {}
			template <typename _Ty>
			baseval& operator=(_Ty&& val) {
				_buf->write(_val->offset, sizeof(_Ty), &val);
				return *this;
			}
			GLint stride() const { return _val->stride; }
			GLint offset() const { return _val->offset; }
			GLint size() const { return _val->size; }
		private:
			value* _val;
			ubo* _buf;
		};
	public:
		uniform() : _prog(0), _offset(0), _buf(&_def), _bdef(true) {}

		uniform(GLuint program, const char* block_name, ubo& src_data, GLint src_data_offset = NULL)
			: uniform(program, _getidx(program, block_name), src_data, src_data_offset) {}
		uniform(GLuint program, GLuint block_index, ubo& src_data, GLint src_data_offset = NULL)
			: unknown(block_index), _prog(program), _buf(&src_data), _offset(src_data_offset), _bdef(false) {
			_cnt++;
			GLint buflen = _blocklen(program, block_index);
			if (_buf->size() != buflen) throw _GLWL exception("Invalid UBO! [Uniform size: %d, UBO size: %d]",
				buflen, _buf->size());
		}

		uniform(GLuint program, const char* block_name) : uniform(program, _getidx(program, block_name)) {}
		uniform(GLuint program, GLuint block_index) : unknown(block_index), _prog(program), _buf(&_def),
			_def(GL_STATIC_DRAW, _blocklen(program, block_index)), _bdef(true) { _cnt++; }

		uniform(uniform&& src) : unknown(src._id), _def(_STD move(src._def)), _buf(_STD move(src._buf)), _offset(src._offset),
			_idoff(_STD move(src._idoff)), _nameoff(_STD move(src._nameoff)), _prog(_STD move(src._prog)), _bdef(src._bdef) {
			if (_bdef) _buf = &_def;
		}
		uniform(const uniform& src) : _buf(src._buf) {}

		GLWL_ASSIG_LV_STD(uniform)
		GLWL_ASSIG_RV_STD(uniform)

		ubo* rdbuf() { return _buf; }
		const ubo* rdbuf() const { return _buf; }
		void bind() const {
			unknown::call(glBindBufferRange, GL_UNIFORM_BUFFER, _buf->id(), _buf->id(), _offset, _buf->size());
			unknown::call(glUniformBlockBinding, _prog, _id, _buf->id());
		}

		template <typename _Ty>
		inline void write(GLint offset, const _Ty& val) { _buf->write(offset, sizeof(_Ty), &val); }

		baseval operator[](GLuint var_index) {
			if (_idoff.count(var_index)) return baseval(&_idoff[var_index], _buf);
			return baseval(_require(var_index, _idoff[var_index]), _buf);
		}
		baseval operator[](const char* var_name) {
			if (_nameoff.count(var_name)) return baseval(_nameoff[var_name], _buf);
			GLuint id; unknown::call(glGetUniformIndices, _prog, 1, &var_name, &id);
			value& newval = _idoff[id];
			_nameoff[var_name] = &newval;
			if (id == GL_INVALID_INDEX) throw _GLWL exception("Variable %s not found!", var_name);
			return baseval(_require(id, newval), _buf);
		}

		value require(const char* var_name) {
			GLuint id; 
			value out;
			unknown::call(glGetUniformIndices, _prog, 1, &var_name, &id);
			if (id == GL_INVALID_INDEX) throw _GLWL exception("Variable %s not found!", var_name);
			unknown::call(glGetActiveUniformsiv, _prog, 1, &id, GL_UNIFORM_ARRAY_STRIDE, &out.stride);
			unknown::call(glGetActiveUniformsiv, _prog, 1, &id, GL_UNIFORM_OFFSET, &out.offset);
			unknown::call(glGetActiveUniformsiv, _prog, 1, &id, GL_UNIFORM_SIZE, &out.size);
			out.id = id;
			return out;
		}
	private:
		inline value* _require(GLuint id, value& val) {
			unknown::call(glGetActiveUniformsiv, _prog, 1, &id, GL_UNIFORM_ARRAY_STRIDE, &val.stride);
			unknown::call(glGetActiveUniformsiv, _prog, 1, &id, GL_UNIFORM_OFFSET, &val.offset);
			unknown::call(glGetActiveUniformsiv, _prog, 1, &id, GL_UNIFORM_SIZE, &val.size);
			val.id = id;
			return &val;
		}
		inline static GLuint _getidx(GLuint prog, const char* block) {
			GLuint id = glGetUniformBlockIndex(prog, block);
			if (id == GL_INVALID_INDEX) throw _GLWL exception("Uniform block %s not found!", block);
			return id;
		}
		inline static GLuint _blocklen(GLuint prog, GLuint block) {
			GLint buflen;
			glGetActiveUniformBlockiv(prog, block, GL_UNIFORM_BLOCK_DATA_SIZE, &buflen);
			return buflen;
		}

		ubo _def;
		ubo* _buf;
		bool _bdef;

		GLuint _offset;
		_STD map<GLuint, value> _idoff;
		_STD map<const char*, value*> _nameoff;
		GLuint _prog;

		static GLuint _cnt;
	};
	GLuint uniform::_cnt = -1;
}