namespace error {
	namespace buf {
		struct std {
		public:
			enum code {
				sucess = 0x00000000,
				fail = 0xffffffff,

				bad_glUnmapBuffer = 0x1,
				bad_glBindBuffer = 0x2,
				bad_glBindBufferBase = 0x4,
				bad_glBufferData = 0x8,
				bad_glBufferStorage = 0x16,
				bad_glBufferSubData = 0x32,
				bad_glBindBufferRange = 0x64,
				bad_glCopyBufferSubData = 0x128,

				none = 0x00000000,
				all = 0xffffffff
			};

			GLenum glerr() const { return _glerr; }
			code error() const { return _error; }
			code exception() const { return _mask; }
			code exception(code mask) const { code old = _mask; _mask = mask; return old; }
		protected:
			~std() {}
			std() {}

		#define GLWL_BUF_THROW(proc, format) \
			case bad_##proc: throw _GLWL exception(#proc##" function failure.Arguments: {\n"##format##"} \nError message : %s"
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
				GLWL_BUF_THROW(glBindBuffer, "    Type: %s,\n    ID: %d\n"),
					macro::str(a1), a2, macro::str(_glerr));
				GLWL_BUF_EXCEPT_CALL_END
			}

			template <typename FTy, typename A1Ty, typename A2Ty>
			inline void call(FTy f, A1Ty a1, A2Ty a2) const {
				f(a1, a2);
				GLWL_BUF_EXCEPT_CALL_BEG
				GLWL_BUF_THROW(glBindBuffer, "    Type: %s,\n    ID: %d\n"),
					macro::str(a1), a2, macro::str(_glerr));
				GLWL_BUF_EXCEPT_CALL_END
			}

			template <typename FTy, typename A1Ty, typename A2Ty, typename A3Ty>
			inline void call(FTy f, A1Ty a1, A2Ty a2, A3Ty a3) const {
				f(a1, a2, a3);
				GLWL_BUF_EXCEPT_CALL_BEG
				GLWL_BUF_THROW(glBindBufferBase, "    Type: %s,\n    BindIdx: %d,\n    ID: %d\n\n"),
					macro::str(a1), a2, a3, macro::str(_glerr));
				GLWL_BUF_EXCEPT_CALL_END
			}

			template <typename FTy, typename A1Ty, typename A2Ty, typename A3Ty, typename A4Ty>
			inline void call(FTy f, A1Ty a1, A2Ty a2, A3Ty a3, A4Ty a4) const {
				f(a1, a2, a3, a4);
				GLWL_BUF_EXCEPT_CALL_BEG
				GLWL_BUF_THROW(glBufferData, "    Type: %s,\n    Size: %d,\n    Data: %x,\n    Usage: %s\n"),
					macro::str(a1), a2, a3, macro::str(a4), macro::str(_glerr));
				GLWL_BUF_THROW(glBufferStorage, "    Type: %s,\n    Size: %d,\n    Data: %x,\n    Flags: %x\n"),
					macro::str(a1), a2, a3, a4, macro::str(_glerr));
				GLWL_BUF_THROW(glBufferSubData, "    Type: %s,\n    Offset: %d,\n    Size: %d,\n    Data: %x\n"),
					macro::str(a1), a2, a3, a4, macro::str(_glerr));
				GLWL_BUF_EXCEPT_CALL_END
			}

			template <typename FTy, typename A1Ty, typename A2Ty, typename A3Ty, typename A4Ty, typename A5Ty>
			inline void call(FTy f, A1Ty a1, A2Ty a2, A3Ty a3, A4Ty a4, A5Ty a5) const {
				f(a1, a2, a3, a4, a5);
				GLWL_BUF_EXCEPT_CALL_BEG
				GLWL_BUF_THROW(glBindBufferRange, "    Type: %s,\n    BindIdx: %d,\n    ID: %x,\n    Offset: %d,\n    Size: %d\n"),
					macro::str(a1), a2, a3, a4, a5, macro::str(_glerr));
				GLWL_BUF_THROW(glCopyBufferSubData, "    Target1: %s,\n    Target2: %s,\n    Offset1: %d,\n    Offset2: %d,\n    Size: %d\n"),
					macro::str(a1), macro::str(a2), a3, a4, a5, macro::str(_glerr));
				GLWL_BUF_EXCEPT_CALL_END
			}

			template <typename... MsgTypes>
			inline void check(bool err, MsgTypes... msg) const {
				if (err) throw _GLWL exception(msg...); }

			#undef GLWL_BUF_THROW
			#undef GLWL_BUF_EXCEPT_CALL_BEG
			#undef GLWL_BUF_EXCEPT_CALL_END
		private:
			mutable GLenum _glerr;
			mutable code _mask;
			mutable code _error;

			inline void setf(const void* f) const {
				if (f == glUnmapBuffer)	{ _error = bad_glUnmapBuffer; return; }
				if (f == glBindBuffer)	{ _error = bad_glBindBuffer; return; }
				if (f == glBindBufferBase)	{ _error = bad_glBindBufferBase; return; }
				if (f == glBufferData)	{ _error = bad_glBufferData; return; }
				if (f == glBufferStorage)	{ _error = bad_glBufferStorage; return; }
				if (f == glBufferSubData)	{ _error = bad_glBufferSubData; return; }
				if (f == glBindBufferRange)	{ _error = bad_glBindBufferRange; return; }
				if (f == glCopyBufferSubData)	{ _error = bad_glCopyBufferSubData; return; }
				_error = fail;
			}
		};
	}
	namespace shader {
		struct std {
		public:
			enum code {
				sucess = 0x00000000,
				fail = 0xffffffff,

				bad_glCompileShader = 0x1,
				bad_glDeleteShader = 0x2,
				bad_glAttachShader = 0x4,
				bad_glDetachShader = 0x8,
				bad_glGetShaderiv = 0x16,
				bad_glShaderSource = 0x32,
				bad_glGetShaderInfoLog = 0x64,

				none = 0x00000000,
				all = 0xffffffff
			};

			GLenum glerr() const { return _glerr; }
			code error() const { return _error; }
			code exception() const { return _mask; }
			code exception(code mask) const { code old = _mask; _mask = mask; return old; }
		protected:
			~std() {}
			std() {}

		#define GLWL_BUF_THROW(proc, format) \
			case bad_##proc: throw _GLWL exception(#proc##" function failure.Arguments: {\n"##format##"} \nError message : %s"
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
				GLWL_BUF_THROW(glAttachShader, "    ProgramID: %d,\n    ShaderID: %d\n"),
					a1, a2, macro::str(_glerr));
				GLWL_BUF_THROW(glDetachShader, "    ProgramID: %d,\n    ShaderID: %d\n"),
					a1, a2, macro::str(_glerr));
				GLWL_BUF_EXCEPT_CALL_END
			}

			template <typename FTy, typename A1Ty, typename A2Ty>
			inline void call(FTy f, A1Ty a1, A2Ty a2) const {
				f(a1, a2);
				GLWL_BUF_EXCEPT_CALL_BEG
				GLWL_BUF_THROW(glAttachShader, "    ProgramID: %d,\n    ShaderID: %d\n"),
					a1, a2, macro::str(_glerr));
				GLWL_BUF_THROW(glDetachShader, "    ProgramID: %d,\n    ShaderID: %d\n"),
					a1, a2, macro::str(_glerr));
				GLWL_BUF_EXCEPT_CALL_END
			}

			template <typename FTy, typename A1Ty, typename A2Ty, typename A3Ty>
			inline void call(FTy f, A1Ty a1, A2Ty a2, A3Ty a3) const {
				f(a1, a2, a3);
				GLWL_BUF_EXCEPT_CALL_BEG
				GLWL_BUF_THROW(glGetShaderiv, "    ID: %d,\n    ParamName: %s,\n    Data: %x\n"),
					a1, macro::str(a2), a3, macro::str(_glerr));
				GLWL_BUF_EXCEPT_CALL_END
			}

			template <typename FTy, typename A1Ty, typename A2Ty, typename A3Ty, typename A4Ty>
			inline void call(FTy f, A1Ty a1, A2Ty a2, A3Ty a3, A4Ty a4) const {
				f(a1, a2, a3, a4);
				GLWL_BUF_EXCEPT_CALL_BEG
				GLWL_BUF_THROW(glShaderSource, "    ID: %d,\n    Count: %d,\n    Data: %x,\n    Length: %d\n"),
					a1, a2, a3, a4, macro::str(_glerr));
				GLWL_BUF_THROW(glGetShaderInfoLog, "    ID: %d,\n    BufSize: %d,\n    Length: %d,\n    Data: %x\n"),
					a1, a2, a3, a4, macro::str(_glerr));
				GLWL_BUF_EXCEPT_CALL_END
			}

			template <typename... MsgTypes>
			inline void check(bool err, MsgTypes... msg) const {
				if (err) throw _GLWL exception(msg...); }

			#undef GLWL_BUF_THROW
			#undef GLWL_BUF_EXCEPT_CALL_BEG
			#undef GLWL_BUF_EXCEPT_CALL_END
		private:
			mutable GLenum _glerr;
			mutable code _mask;
			mutable code _error;

			inline void setf(const void* f) const {
				if (f == glCompileShader)	{ _error = bad_glCompileShader; return; }
				if (f == glDeleteShader)	{ _error = bad_glDeleteShader; return; }
				if (f == glAttachShader)	{ _error = bad_glAttachShader; return; }
				if (f == glDetachShader)	{ _error = bad_glDetachShader; return; }
				if (f == glGetShaderiv)	{ _error = bad_glGetShaderiv; return; }
				if (f == glShaderSource)	{ _error = bad_glShaderSource; return; }
				if (f == glGetShaderInfoLog)	{ _error = bad_glGetShaderInfoLog; return; }
				_error = fail;
			}
		};
	}
	namespace texture {
		struct std {
		public:
			enum code {
				sucess = 0x00000000,
				fail = 0xffffffff,

				bad_glBindTexture = 0x1,
				bad_glTexParameteri = 0x2,
				bad_glCompressedTexImage2D = 0x4,
				bad_glTexImage2D = 0x8,

				none = 0x00000000,
				all = 0xffffffff
			};

			GLenum glerr() const { return _glerr; }
			code error() const { return _error; }
			code exception() const { return _mask; }
			code exception(code mask) const { code old = _mask; _mask = mask; return old; }
		protected:
			~std() {}
			std() {}

		#define GLWL_BUF_THROW(proc, format) \
			case bad_##proc: throw _GLWL exception(#proc##" function failure.Arguments: {\n"##format##"} \nError message : %s"
		#define GLWL_BUF_EXCEPT_CALL_BEG \
			_glerr = glGetError(); \
			if (_glerr) {\
			setf(reinterpret_cast<const void*>(f)); \
			if (!(_mask & _error)) return; \
			switch (_error) {
		#define GLWL_BUF_EXCEPT_CALL_END }}

			template <typename FTy, typename A1Ty, typename A2Ty>
			inline void call(FTy f, A1Ty a1, A2Ty a2) const {
				f(a1, a2);
				GLWL_BUF_EXCEPT_CALL_BEG
				GLWL_BUF_THROW(glBindTexture, "    Target: %s,\n    ID: %d\n"),
					macro::str(a1), a2, macro::str(_glerr));
				GLWL_BUF_EXCEPT_CALL_END
			}

			template <typename FTy, typename A1Ty, typename A2Ty, typename A3Ty>
			inline void call(FTy f, A1Ty a1, A2Ty a2, A3Ty a3) const {
				f(a1, a2, a3);
				GLWL_BUF_EXCEPT_CALL_BEG
				GLWL_BUF_THROW(glTexParameteri, "    Target: %s,\n    Setting: %s,\n    Param: %x\n"),
					macro::str(a1), macro::str(a2), macro::str(a3), macro::str(_glerr));
				GLWL_BUF_EXCEPT_CALL_END
			}

			template <typename FTy, typename A1Ty, typename A2Ty, typename A3Ty, typename A4Ty, 
typename A5Ty, typename A6Ty, typename A7Ty, typename A8Ty>
			inline void call(FTy f, A1Ty a1, A2Ty a2, A3Ty a3, A4Ty a4, A5Ty a5, A6Ty a6, A7Ty a7, A8Ty a8) const {
				f(a1, a2, a3, a4, a5, a6, a7, a8);
				GLWL_BUF_EXCEPT_CALL_BEG
				GLWL_BUF_THROW(glCompressedTexImage2D, "    Target: %s,\n    Level: %d,\n    InternalFormat: %s,\n    Width: %d,\n    Height: %d,\n    Border: %d,\n    ImdSize: %d,\n    Data: %x\n"),
					macro::str(a1), a2, macro::str(a3), a4, a5, a6, a7, a8, macro::str(_glerr));
				GLWL_BUF_EXCEPT_CALL_END
			}

			template <typename FTy, typename A1Ty, typename A2Ty, typename A3Ty, typename A4Ty, 
typename A5Ty, typename A6Ty, typename A7Ty, typename A8Ty, typename A9Ty>
			inline void call(FTy f, A1Ty a1, A2Ty a2, A3Ty a3, A4Ty a4, A5Ty a5, A6Ty a6, A7Ty a7, A8Ty a8, A9Ty a9) const {
				f(a1, a2, a3, a4, a5, a6, a7, a8, a9);
				GLWL_BUF_EXCEPT_CALL_BEG
				GLWL_BUF_THROW(glTexImage2D, "    Target: %s,\n    Level: %d,\n    InternalFormat: %s,\n    Width: %d,\n    Height: %d,\n    Border: %d,\n    PixelFormat: %s,\n    PixelType: %s,\n    Data: %x\n"),
					macro::str(a1), a2, macro::str(a3), a4, a5, a6, macro::str(a7), macro::str(a8), a9, macro::str(_glerr));
				GLWL_BUF_EXCEPT_CALL_END
			}

			template <typename... MsgTypes>
			inline void check(bool err, MsgTypes... msg) const {
				if (err) throw _GLWL exception(msg...); }

			#undef GLWL_BUF_THROW
			#undef GLWL_BUF_EXCEPT_CALL_BEG
			#undef GLWL_BUF_EXCEPT_CALL_END
		private:
			mutable GLenum _glerr;
			mutable code _mask;
			mutable code _error;

			inline void setf(const void* f) const {
				if (f == glBindTexture)	{ _error = bad_glBindTexture; return; }
				if (f == glTexParameteri)	{ _error = bad_glTexParameteri; return; }
				if (f == glCompressedTexImage2D)	{ _error = bad_glCompressedTexImage2D; return; }
				if (f == glTexImage2D)	{ _error = bad_glTexImage2D; return; }
				_error = fail;
			}
		};
	}
}
