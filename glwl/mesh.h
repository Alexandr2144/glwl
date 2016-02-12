#pragma once

#include "buffer.h"
#include "shader.h"
//#include "material.h"

#include "core.h"
#include "buffer2.h"

#include <vector>

#include <glm\mat4x4.hpp>
#include <glm\gtc\matrix_transform.hpp>

namespace glwl {
	class vao : public unknown {
	private:
		struct proxy {
			proxy(GLuint buf, GLuint off) : _buf(buf), _offset(off) {}
			GLuint _buf;
			GLintptr _offset;
		};

		class buf : proxy {
		public:
			buf(GLuint slot, GLuint attr) : proxy(slot, 0), _attrib(attr) {}
			buf& operator[](GLuint offset) { _offset = offset; return *this; }
			buf& operator<<(offset&& off) { _offset = off.val; return *this; }
			template <typename AttribTy>
			void operator<<(_GLWL format<AttribTy>&& form) {
				typedef type<AttribTy> inf;
				glVertexAttribFormat(_attrib, inf::count, inf::id, form.norm, _offset);
				glVertexAttribBinding(_attrib, _buf);
				glEnableVertexAttribArray(_attrib);
			}
		private:
			GLuint _attrib;
		};

		class setbuf : proxy {
		public:
			setbuf(GLuint slot) : proxy(slot, 0) {}
			setbuf& operator[](GLuint offset) { _offset = offset; return *this; }
			setbuf& operator<<(offset&& off) { _offset = off.val; return *this; }
			template <typename ValueTy>
			void operator<<(const vbo<ValueTy>& vbuffer) {
				glBindVertexBuffer(_buf, vbuffer.rdbuf()->id(), _offset, sizeof(ValueTy));
			}
			template <class Buf, class ValueTy, class Cache>
			void operator<<(const _GLWL buf::stream<Buf, ValueTy, Cache>& vbuffer) {
				glBindVertexBuffer(_buf, vbuffer.rdbuf()->id(), _offset, sizeof(ValueTy));
			}
		};

		class attrib {
		public:
			attrib(GLuint attr) : _idx(attr) {}
			void on() { glEnableVertexAttribArray(_idx); }
			void off() { glDisableVertexAttribArray(_idx); }
			vao::buf operator[](GLintptr index) { return vao::buf(index, _idx); }
		private:
			GLuint _idx;
		};
	public:
		~vao() { if (_id) glDeleteVertexArrays(1, &_id); }
		vao() { glGenVertexArrays(1, &_id); }

		vao(const vao& src) : unknown(src) {}

		GLWL_CTOR_RV_STD(vao)
		GLWL_ASSIG_RV_STD(vao)
		GLWL_ASSIG_LV_STD(vao)

		template <typename ValueTy>
		void format(GLuint attrib_slot, size_t buffer_slot, GLboolean normalized, GLuint offset) {
			typedef type<ValueTy> inf;
			glVertexAttribFormat(attrib_slot, inf::count, inf::id, normalized, offset);
			glVertexAttribBinding(attrib_slot, buffer_slot);
			enable_attrib(attrib_slot);
		}

		template <typename ValueTy, typename... ValuesTy>
		void format(GLuint buffer_slot, _STD initializer_list<GLboolean> normalized) {
			_format<0, ValueTy, ValuesTy...>(0, buffer_slot, normalized.begin(), 0);
		}

		void bind() const { glBindVertexArray(_id); }

		void enable_attrib(size_t attrib_slot) {
			glEnableVertexAttribArray(attrib_slot);
		}
		void disable_attrib(size_t attrib_slot) { glDisableVertexAttribArray(attrib_slot); }

		template <typename AttribTy>
		void attach(GLuint buffer_slot, const vbo<AttribTy>& vbuffer, GLintptr offset = 0) {
			glBindVertexBuffer(buffer_slot, vbuffer.rdbuf().id(), offset, sizeof(AttribTy));
		}

		setbuf operator[](GLuint buffer_slot) { return setbuf(buffer_slot); }
		attrib operator()(GLuint attrib_slot) { return attrib(attrib_slot); }
	private:
		template <int, typename ValueTy, typename... ValuesTy>
		void _format(GLuint attrib_slot, size_t buffer_slot, const GLboolean* normalized, GLuint offset) {
			format<ValueTy>(attrib_slot, buffer_slot, *normalized, offset);
			_format<0, ValuesTy...>(attrib_slot + 1, buffer_slot, normalized + 1, offset + sizeof(ValueTy));
		}
		template <int>
		void _format(GLuint attrib_slot, size_t buffer_slot, const GLboolean* normalized, GLuint offset) {}
	};

	class location {
	public:
		location(buf::ubo<>::part cbuf, GLint offset) : location(cbuf, offset, glm::mat4()) {}
		location(buf::ubo<>::part cbuf, GLint offset, const glm::mat4& mat)
			: _mat(mat), _cbuf(cbuf), _offset(offset) {}

		location& operator=(const glm::mat4& mat) { _mat = mat; return *this; }

		const glm::mat4& get() const { return _mat; }
		void update() const { _cbuf.bind(); _cbuf.write(_offset, _mat); }

		void move(const glm::vec3& position) { _mat = glm::translate(_mat, position); }
		void spawn(const glm::vec3& position) { _mat[3] = glm::vec4(position, _mat[3].w); }
		void rotate(float angle, const glm::vec3& axis) { _mat = glm::rotate(_mat, angle, axis); }
		void scale(const glm::vec3& value) { _mat = glm::scale(_mat, value); }

		void perspective(float fov, float aspect = 1.6f, float zNear = 0.1f, float zFar = 100.0f) {
			_mat = glm::perspective(fov, aspect, zNear, zFar); }
		void perspective(float fov, float width, float height, float zNear = 0.1f, float zFar = 100.0f) {
			_mat = glm::perspectiveFov(fov, width, height, zNear, zFar); }
		void look(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up = glm::vec3(0, 1, 0)) {
			_mat = glm::lookAt(position, position + direction, up); }

		operator const glm::mat4&() const { return _mat; }
	private:
		mutable buf::ubo<>::part _cbuf;
		glm::mat4 _mat;
		GLuint _offset;
	};


	template <typename ValTy, typename IdxTy = GLuint>
	class vertex_indexed_attrib {
	public:
		vertex_indexed_attrib() {}
		vertex_indexed_attrib(_STD initializer_list<ValTy> values,
			_STD initializer_list<IdxTy> indices) : _data(values), _idx(indices) {}

		inline void add_attrib(ValTy&& value) { _data.emplace_back(_STD forward<ValTy>(value)); }
		inline void add_index(IdxTy index) { _idx.push_back(index); }

		inline ValTy& operator[](size_t index) { return _data[_idx[index]]; }
		inline const ValTy& operator[](size_t index) const { return _data[_idx[index]]; }

		inline size_t size() const { return _idx.size(); }
	private:
		_STD vector<ValTy> _data;
		_STD vector<IdxTy> _idx;
	};

	template <typename _ValTy, typename _IdxTy>
	stream<_ValTy>& operator<<(stream<_ValTy>& os, const vertex_indexed_attrib<_ValTy, _IdxTy>& out) {
		const size_t len = out.size();
		for (size_t i = 0; i < len; i++) os << out[i];
		return os;
	}
}