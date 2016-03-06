#ifndef __GLWL_FILE_H
#define __GLWL_FILE_H

#include <Windows.h>

namespace glwl {
	class file {
	public:
		enum acess {
			read = GENERIC_READ,
			write = GENERIC_WRITE,
			readwrite = GENERIC_READ | GENERIC_WRITE
		};
		enum share {
			no_share = 0,
			share_read,
			share_write,
			share_readwrite,
		};
		enum openmode {
			if_exist = OPEN_EXISTING,
		};
		enum file_attrib {
			normal = FILE_ATTRIBUTE_NORMAL,
			archive = FILE_ATTRIBUTE_ARCHIVE
		};

		file(LPWSTR name, acess desired_acess = read, openmode mode = if_exist,
			share share_mode = no_share, file_attrib attrib = normal) : _badfl(false),
			_file(INVALID_HANDLE_VALUE), _map(nullptr) {
			_file = CreateFile(name, desired_acess, share_mode, nullptr, mode, attrib, nullptr);
			if (_file == INVALID_HANDLE_VALUE) { _badfl = true; return; }

			_size = GetFileSize(_file, nullptr);
			if (_size == INVALID_FILE_SIZE) { _badfl = true; this->~file(); return; }

			_map = CreateFileMapping(_file, nullptr, PAGE_READONLY, 0, 0, nullptr);
			if (_map == nullptr) { _badfl = true; this->~file(); return; }

			_data = MapViewOfFile(_map, FILE_MAP_READ, 0, 0, _size);
			if (_data == nullptr) { _badfl = true; this->~file(); return; }
		}

		inline bool good() const { return !_badfl; }
		inline bool bad() const { return _badfl; }

		inline char* data() const { return reinterpret_cast<char*>(_data); }

		GLuint size() const { return _size; }

		HANDLE handle() const { return _file; }
	private:
		HANDLE _file;
		HANDLE _map;
		DWORD _size;
		bool _badfl;

		void* _data;
	};
}

#endif //__GLWL_FILE_H