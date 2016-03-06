#include <Windows.h>

enum GLWL_PROFILE {
	GLWL_CORE_PROFILE = WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
	GLWL_COMPATIBILITY_PROFILE = WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB
};

void glwlQuit() { PostQuitMessage(0); }

namespace glwl {
	class context {
	public:
		struct setting {
			struct gl_version {
				int major, minor;
				gl_version() : major(4), minor(0) {}
			} version;

			union color {
				struct { float r, g, b, a; };
				float rgba[4];
				color() : r(0), g(0), b(0), a(1) {}
			} background;

			struct pixel_format {
				enum {
					RGBA = PFD_TYPE_RGBA
				} type;
				BYTE color;
				BYTE depth;
				BYTE stencil;
				BYTE alpha;

				pixel_format() : type(RGBA), color(32), depth(24), stencil(0), alpha(8) {}
			} pformat;
			GLWL_PROFILE profile;
			setting() : profile(GLWL_CORE_PROFILE) {}
		};

		~context() {
			if (_hglrc){
				wglMakeCurrent(NULL, NULL);
				wglDeleteContext(_hglrc);
			} if (_hdc) ReleaseDC(_hwnd, _hdc);
		}

		context() : _hwnd(NULL), _hdc(NULL), _hglrc(NULL) {}
		context(HWND wnd, setting param) : _hwnd(wnd) {
			UINT pformat;
			PIXELFORMATDESCRIPTOR pformat_desc;
			ZeroMemory(&pformat_desc, sizeof(pformat_desc));
			pformat_desc.nSize = sizeof(pformat_desc);
			pformat_desc.nVersion = 1;
			pformat_desc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			pformat_desc.iPixelType = param.pformat.type;
			pformat_desc.cColorBits = param.pformat.color;
			pformat_desc.cDepthBits = param.pformat.depth;
			pformat_desc.cAlphaBits = param.pformat.alpha;
			pformat_desc.cStencilBits = param.pformat.stencil;

			int context_attrib[] = {
				WGL_CONTEXT_MAJOR_VERSION_ARB, param.version.major,
				WGL_CONTEXT_MINOR_VERSION_ARB, param.version.minor,
				WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
				WGL_CONTEXT_PROFILE_MASK_ARB, param.profile, 0 };

			/*DEVMODE dm;
			DISPLAY_DEVICE dd;
			ZeroMemory(&dm, sizeof(dm));
			dd.cb = sizeof(DISPLAY_DEVICE);
			EnumDisplayDevices(NULL, 0, &dd, 0);
			EnumDisplaySettings(dd.DeviceName, 0, &dm);
			_hdc = CreateDC(NULL, dd.DeviceName, NULL, NULL);
			//EnumDisplayDevices(NULL, 1, &dd, NULL);
			//EnumDisplaySettings(dd.DeviceName, 1, &dm);
			_hdc = CreateDC(NULL, dm.dmDeviceName, NULL, NULL);
			EnumDisplayDevices(NULL, 2, &dd, NULL);
			EnumDisplaySettings(dd.DeviceName, 2, &dm);
			_hdc = CreateDC(NULL, dd.DeviceName, NULL, NULL);
			EnumDisplayDevices(NULL, 3, &dd, 0);
			EnumDisplaySettings(dd.DeviceName, 0, &dm);
			_hdc = CreateDC(NULL, dd.DeviceName, NULL, &dm);
			auto e = GetLastError();*/
	
			if (!(_hdc = GetDC(_hwnd)))
				throw _STD exception("Can't Create A GL Device Context");
			if (!(pformat = ChoosePixelFormat(_hdc, &pformat_desc)))
				throw _STD exception("Can't Find A Suitable PixelFormat");
			if (!SetPixelFormat(_hdc, pformat, &pformat_desc))
				throw _STD exception("Can't Set The PixelFormat");
			if (!(_hglrc = wglCreateContext(_hdc)))
				throw _STD exception("Can't Create A GL Rendering Context");
			this->bind();

			if (glewInit() != GLEW_OK)
				throw _STD exception("Can't Initialize GLEW");
			if (!(WGLEW_ARB_create_context && WGLEW_ARB_pixel_format))
				throw _STD exception("Can't Activate The GL Rendering Context");
			if (!wglMakeCurrent(NULL, NULL))
				throw _STD exception("Can't Activate The GL Rendering Context");
			if (!wglDeleteContext(_hglrc))
				throw _STD exception("Can't Activate The GL Rendering Context");
			if (!(_hglrc = wglCreateContextAttribsARB(_hdc, 0, context_attrib)))
				throw _STD exception("Can't Activate The GL Rendering Context");
			this->bind();
		}

		context(context&& src) { 
			_STD memcpy(this, &src, sizeof(context));
			_STD memset(&src, NULL, sizeof(context));
		}
		context& operator=(context&& src) {
			this->~context();
			new(this) context(_STD move(src));
			return *this;
		}

		context(const context& src) = delete;
		context& operator=(const context& src) = delete;

		void bind() const {
			if (!wglMakeCurrent(_hdc, _hglrc))
				throw _STD exception("Can't Activate The GL Rendering Context");
		}

		void clear() const {
			glClearColor(_param.background.r, _param.background.g,
				_param.background.b, _param.background.a);
			glClear(GL_COLOR_BUFFER_BIT);
			glClear(GL_DEPTH_BUFFER_BIT);
		}
		void update() const { SwapBuffers(_hdc); }

		void drawText(int x, int y, const char* text) const {
			TextOutA(_hdc, x, y, text, strlen(text)); }

		void enable(GLuint mode) const { glEnable(mode); }
		void disable(GLuint mode) const { glDisable(mode); }

		void enableDepthTest(GLuint func = GL_LEQUAL) const {
			enable(GL_DEPTH_TEST);
			glDepthFunc(func); }

		void cull(GLenum mode = GL_BACK) const {
			enable(GL_CULL_FACE); 
			glCullFace(mode);
		}
		void wireframe() const { 
			enable(GL_LINE_SMOOTH);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
	private:
		HDC _hdc;
		HGLRC _hglrc;
		HWND _hwnd;

		setting _param;
	};

	class app {
	public:
		struct setting {
			LPWSTR wnd_name = L"Unnamed";
			LPWSTR wnd_class_name = L"Unnamed";

			bool fullscreen = false;

			size_t width = 500;
			size_t height = 500;
			int x = 0;
			int y = 0;

			context::setting gl;
		};

		~app();
		app() {
			if (_instance) throw _STD exception("Can be only one app instance", 0);
			_instance = this;
		}
		
		app(window&& src) = delete;
		app(const app& src) = delete;
		app& operator=(app&& src) = delete;
		app& operator=(const app& src) = delete;

		void setContextProfile(GLWL_PROFILE profile) { _setting.gl.profile = profile; }
		void setContextVersion(int major, int minor) {
			_setting.gl.version.major = major,
			_setting.gl.version.minor = minor;
		}

		void setWindowClassName(LPWSTR name) {
			_setting.wnd_class_name = name; }
		void setWindowName(LPWSTR name) { 
			_setting.wnd_name = name; 
			if (!lstrcmp(_setting.wnd_class_name, L"Unnamed"))
				_setting.wnd_class_name = name;
		}

		void showWindow(int nShowCmd = SW_SHOW) {
			::ShowWindow(_wnd, nShowCmd);
			::SetForegroundWindow(_wnd);
			::SetFocus(_wnd);
			::glViewport(0, 0, _setting.width, _setting.height);
		}

		void setWindowSize(GLuint uiWidth, GLuint uiHeight) {
			if (uiHeight == 0) uiHeight = 1;
			_setting.width = uiWidth, _setting.height = uiHeight;
			::glViewport(0, 0, uiWidth, uiHeight);
		}

		GLuint getWindowWidth() const { return _setting.width; }
		GLuint getWindowHeight() const { return _setting.height; }

		void launch();

		template <typename LambdaTy>
		void loop(LambdaTy dispaly) {
			while (true) {
				if (PeekMessage(&_msg, NULL, 0, 0, PM_REMOVE)) {
					if (_msg.message == WM_QUIT) return;
					else {
						TranslateMessage(&_msg);
						DispatchMessage(&_msg);
						_wndproc_ext(_msg);
					}
				} else {
					context.clear();
					dispaly();
					context.update();
				}
			}
		}

		bool fullscreen() const { return _setting.fullscreen; }
		void fullscreen(bool fl) { _setting.fullscreen = fl; }

		HWND handle() const { return _wnd; }
		
		static app& instance() { return *_instance; }

		glwl::context context;
	private:
		void _wndproc_ext(MSG msg);
		static app* _instance;
		setting _setting;

		MSG _msg;
		HWND _wnd;

		HDC _comp_c;
		HGLRC _gl_c;
	};

	app* app::_instance = nullptr;

	void app::launch() {
		HINSTANCE hInstance = GetModuleHandle(NULL);
		RECT wndrect;
		wndrect.left = _setting.x;
		wndrect.right = _setting.x + _setting.width;
		wndrect.top = _setting.y;
		wndrect.bottom = _setting.y + _setting.height;

		WNDCLASS wndclass;
		wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wndclass.lpfnWndProc = [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LPARAM {
			switch (uMsg) {
			case WM_CLOSE:
				PostQuitMessage(0);
				return 0;
			}
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		};
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = hInstance;
		wndclass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
		wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndclass.hbrBackground = NULL;
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = _setting.wnd_class_name;

		if (!RegisterClass(&wndclass)) throw _STD exception("Failed To Register The Window Class");

		DWORD dwExStyle;
		DWORD dwStyle;
		if (_setting.fullscreen){
			DEVMODE devmode;
			ZeroMemory(&devmode, sizeof(devmode));
			devmode.dmSize = sizeof(devmode);
			devmode.dmPelsWidth = _setting.width;
			devmode.dmPelsHeight = _setting.height;
			devmode.dmBitsPerPel = 32;
			devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			if (ChangeDisplaySettings(&devmode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
				throw _STD exception("The Requested Fullscreen Mode Is Not Supported By Your Video Card");

			dwExStyle = WS_EX_APPWINDOW;
			dwStyle = WS_POPUP;
			ShowCursor(false);
		}
		else {
			dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
			dwStyle = WS_OVERLAPPEDWINDOW;
		}
		AdjustWindowRectEx(&wndrect, dwStyle, false, dwExStyle);
		if (!(_wnd = CreateWindowEx(dwExStyle, _setting.wnd_class_name, _setting.wnd_name,
			WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle, 0, 0,
			_setting.width, _setting.height,
			NULL, NULL, hInstance, NULL))) throw _STD exception("Window Creation Error");

		context = glwl::context(_wnd, _setting.gl);

		//glShadeModel(GL_SMOOTH);
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_TEXTURE_2D);
		//err = glGetError();
		glDepthFunc(GL_LEQUAL);
		//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		//err = glGetError();
	}

	app::~app() {
		if (fullscreen()){
			ChangeDisplaySettings(NULL, 0);
			::ShowCursor(true);
		}
		if (_wnd) ::DestroyWindow(_wnd);
		::UnregisterClassW(_setting.wnd_class_name, GetModuleHandle(NULL));
	}

	void app::_wndproc_ext(MSG msg) {
		PAINTSTRUCT ps;
		HDC hdc;

		switch (msg.message) {
		case WM_SYSCOMMAND:
			switch (msg.wParam) {
			case SC_SCREENSAVE:
			case SC_MONITORPOWER: return;
			} return;
		case WM_PAINT:
			hdc = ::BeginPaint(msg.hwnd, &ps);
			::EndPaint(msg.hwnd, &ps); return;
		case WM_CLOSE:
			::PostQuitMessage(0); return;
		case WM_SIZE:
			setWindowSize(LOWORD(msg.lParam), HIWORD(msg.lParam)); return;
		} return;
	}
}