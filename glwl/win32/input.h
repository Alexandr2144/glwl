#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include <vector>

#pragma once

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

enum {
	GLWL_IK_ESC = DIK_ESCAPE,

	GLWL_IK_UPARROW = DIK_UPARROW,
	GLWL_IK_DOWNARROW = DIK_DOWNARROW,
	GLWL_IK_LEFTARROW = DIK_LEFTARROW,
	GLWL_IK_RIGHTARROW = DIK_RIGHTARROW,

	GLWL_IK_0 = DIK_0,
	GLWL_IK_1 = DIK_1,
	GLWL_IK_2 = DIK_2,
	GLWL_IK_3 = DIK_3,
	GLWL_IK_4 = DIK_4,

	GLWL_IK_A = DIK_A,
	GLWL_IK_D = DIK_D,
	GLWL_IK_R = DIK_R,
	GLWL_IK_S = DIK_S,
	GLWL_IK_W = DIK_W,

	GLWL_IM_RBUTTON,
	GLWL_IM_LBUTTON,
	GLWL_IM_BBUTTON,
	GLWL_IM_X,
	GLWL_IM_Y,
	GLWL_IM_Z,
};

namespace glwl {
	class input {
	public:
		struct keybd_event {
			void(*f)(void);
			short key;
		};

		struct mouse_event {
			void(*f)(DIMOUSESTATE);
			short type;
		};

		input() : input(glwlGetWindow().handle(), GetModuleHandle(NULL)) {}
		input(HWND hWnd, HINSTANCE hInstance = GetModuleHandle(NULL)) {
			DirectInput8Create(hInstance, DIRECTINPUT_VERSION,
				IID_IDirectInput8, (void**)&_main_dev, NULL);
			if (!_main_dev) throw _STD exception();
			_main_dev->CreateDevice(GUID_SysKeyboard, &_key_dev, NULL);
			if (!_key_dev) throw _STD exception();
			_key_dev->SetDataFormat(&c_dfDIKeyboard);
			_key_dev->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
			_main_dev->CreateDevice(GUID_SysMouse, &_mouse_dev, NULL);
			if (!_mouse_dev) throw _STD exception();
			_mouse_dev->SetDataFormat(&c_dfDIMouse);
			_mouse_dev->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

			_key_dev->Acquire();
			_mouse_dev->Acquire();
			return;
		}

		void event(keybd_event&& type) { _key_events.emplace_back(_STD forward<keybd_event>(type)); }
		void event(mouse_event&& type) { _mouse_events.emplace_back(_STD forward<mouse_event>(type)); }

		bool key(short code) { return _key_buf[code] != NULL; }
		LONG mouse(short code) {
			if (code == GLWL_IM_LBUTTON)
			if (_mouse_buf.rgbButtons[0] & 0x80) return 1;
			else return 0;
			if (code == GLWL_IM_RBUTTON)
			if (_mouse_buf.rgbButtons[1] & 0x80) return 1;
			else return 0;
			if (code == GLWL_IM_BBUTTON)
			if (_mouse_buf.rgbButtons[2] & 0x80) return 1;
			else return 0;
			if (code == GLWL_IM_X) return _mouse_buf.lX;
			if (code == GLWL_IM_Y) return _mouse_buf.lY;
			if (code == GLWL_IM_Z) return _mouse_buf.lZ;
			return 0;
		}

		void update() {
			if (FAILED(_key_dev->GetDeviceState(sizeof(_key_buf), _key_buf)))
				_key_dev->Acquire();
			if (FAILED(_mouse_dev->GetDeviceState(sizeof(DIMOUSESTATE), &_mouse_buf)))
				_mouse_dev->Acquire();
			for (auto& i : _key_events) if (_key_buf[i.key]) i.f();
			for (auto& i : _mouse_events) if (mouse(i.type)) i.f(_mouse_buf);
		}

		~input() {
			_mouse_dev->Release();
			_key_dev->Release();
			_main_dev->Release();
		}
	private:
		IDirectInput* _main_dev;
		IDirectInputDevice* _key_dev;
		IDirectInputDevice* _mouse_dev;

		DIMOUSESTATE _mouse_buf;
		char _key_buf[256];

		_STD vector<keybd_event> _key_events;
		_STD vector<mouse_event> _mouse_events;
	};
}