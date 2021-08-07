#pragma once
#include<windows.h>
#include<cassert>
#include<iostream>
#include<string>
#include <Windows.h>
#include<fstream>
#include <cwchar>
#include<atlstr.h>
#include<vector>

#include<fcntl.h>
#include<io.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

//#define _WIN32_WINNT 0x0500
using namespace std;

void cls()
{
	// Get the Win32 handle representing standard output.
	// This generally only has to be done once, so we make it static.
	static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	COORD topLeft = { 0, 0 };

	// std::cout uses a buffer to batch writes to the underlying console.
	// We need to flush that to the console because we're circumventing
	// std::cout entirely; after we clear the console, we don't want
	// stale buffered text to randomly be written out.
	std::cout.flush();

	// Figure out the current width and height of the console window
	if (!GetConsoleScreenBufferInfo(hOut, &csbi)) {
		// TODO: Handle failure!
		abort();
	}
	DWORD length = csbi.dwSize.X * csbi.dwSize.Y;

	DWORD written;

	// Flood-fill the console with spaces to clear it
	FillConsoleOutputCharacter(hOut, TEXT(' '), length, topLeft, &written);

	// Reset the attributes of every character to the default.
	// This clears all background colour formatting, if any.
	FillConsoleOutputAttribute(hOut, csbi.wAttributes, length, topLeft, &written);

	// Move the cursor back to the top left for the next sequence of writes
	SetConsoleCursorPosition(hOut, topLeft);
}

bool removeScrollBar() {
	// get handle to the console window
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	// retrieve screen buffer info
	CONSOLE_SCREEN_BUFFER_INFO scrBufferInfo;
	GetConsoleScreenBufferInfo(hOut, &scrBufferInfo);

	// current window size
	short winWidth = scrBufferInfo.srWindow.Right - scrBufferInfo.srWindow.Left + 1;
	short winHeight = scrBufferInfo.srWindow.Bottom - scrBufferInfo.srWindow.Top + 1;

	// current screen buffer size
	short scrBufferWidth = scrBufferInfo.dwSize.X;
	short scrBufferHeight = scrBufferInfo.dwSize.Y;

	// to remove the scrollbar, make sure the window height matches the screen buffer height
	COORD newSize;
	newSize.X = scrBufferWidth;
	newSize.Y = winHeight;

	int Status = SetConsoleScreenBufferSize(hOut, newSize);
	return Status != -1;
}

void setConsoleSize(int width, int height) {
	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r); //stores the console's current dimensions

	while (!removeScrollBar()) {
	}
	MoveWindow(console, r.left, r.top, width, height + 31, TRUE); // 800 width, 100 height

}

void setCharSize(int x, int y) {
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = x;                   // Width of each character in the font
	cfi.dwFontSize.Y = y;                  // Height
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	wcscpy_s(cfi.FaceName, L"Consolas"); // Choose your font
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
}



/*struct Color {
	int r = 0, g = 0, b = 0;
	Color() {}
	Color(int r, int g, int b) {
		this->r = r;
		this->g = g;
		this->b = b;
	}

	void clear() {
		r = 0; g = 0; b = 0;
	}
};*/

//⬛

class Surface {
	short** screen;
	int width, height;

public:
	Surface(string filename) {
		loadFromFile(filename);
	}

	Surface(int width, int height) {
		this->width = width;
		this->height = height;
		screen = new short* [height];
		for (int i = 0; i < height; ++i)
			screen[i] = new short[width];
	}

	void loadFromMatrix(short** matrix) {
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				screen[i][j] = matrix[i][j];
			}
		}
	}

	void loadFromFile(string filename) {
		ifstream image(filename);
		string read;
		while (!image.eof()) {
			getline(image, read);
			cout << read << endl;
		}
	}

	// get & set //
	inline int getWidth() {
		return width;
	}

	inline int getHeight() {
		return height;
	}

	inline void setSize(int width, int height) {
		short** temp = new short* [height];
		for (int i = 0; i < height; ++i)
			temp[i] = new short[height];

		for (int i = 0; i < this->height; i++) {
			for (int j = 0; j < this->width; j++) {
				temp[i][j] = screen[i][j];
			}
		}
		delete[] screen;
		screen = temp;

		this->width = width;
		this->height = height;
	}
	// -------------- //

	void fill(short color) {
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				screen[i][j] = color;
			}
		}
	}

	short colorAt(int x, int y) {
		assert(x >= 0 && x < width&& y >= 0 && y < height);
		return screen[y][x];
	}

	inline void setPixel(int x, int y, short color) {
		if (x >= 0 && x < width && y >= 0 && y < height) {
			screen[y][x] = color;
		}
	}
};


class ConsoleEngine {
	short** screen;
	int width, height, charSizeX, charSizeY;
	bool updated = 0;

	char keys[37] = { 'Q','W','E','R','T','Y','U','I','O','P','A','S','D','F','G','H','J','K','L','Z','X','C','V','B','N','M',' ','1','2','3','4','5','6','7','8','9','0' };

	HWND console = GetConsoleWindow();

	void resize(int w, int h) {
		setConsoleSize(w * charSizeX, h * charSizeY);
	}

public:
	ConsoleEngine(int width, int height, short charSizeX = 1, short charSizeY = 1) {
		//set unresizable
		SetWindowLong(console, GWL_STYLE, GetWindowLong(console, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);

		::_setmode(::_fileno(stdout), _O_U16TEXT);
		setCharSize(charSizeX, charSizeY);

		this->charSizeX = charSizeX;
		this->charSizeY = charSizeY;
		this->width = width;
		this->height = height;
		screen = new short* [height];
		for (int i = 0; i < height; ++i)
			screen[i] = new short[width];

		resize(width, height);
	}

	// get & set //
	inline void setCaption(string caption) {
		SetConsoleTitleA(caption.c_str());
	}

	inline int getWidth() {
		return width;
	}

	inline int getHeight() {
		return height;
	}

	inline void setSize(int width, int height) {
		updated = 0;
		short** temp = new short* [height];
		for (int i = 0; i < height; ++i)
			temp[i] = new short[height];

		for (int i = 0; i < this->height; i++) {
			for (int j = 0; j < this->width; j++) {
				temp[i][j] = screen[i][j];
			}
		}
		delete[] screen;
		screen = temp;

		this->width = width;
		this->height = height;
		resize(width, height);
	}
	// -------------- //

	void fill(short color) {
		updated = 0;
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				screen[i][j] = color;
			}
		}
	}

	short colorAt(int x, int y) {
		assert(x >= 0 && x < width&& y >= 0 && y < height);
		return screen[y][x];
	}

	inline void setPixel(int x, int y, short color) {
		updated = 0;
		if (x >= 0 && x < width && y >= 0 && y < height) {
			screen[y][x] = color;
		}
	}

	void draw(Surface surf, int x, int y) {
		updated = 0;
		for (int i = 0; i < surf.getHeight(); i++) {
			for (int j = 0; j < surf.getWidth(); j++) {
				setPixel(j + x, i + y, surf.colorAt(j, i));
			}
		}
	}

	void display() {
		if (updated) return;
		updated = 1;
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

		CHAR_INFO* charInfo = new CHAR_INFO[width * height];
		COORD charBufSize = { width, height };
		COORD characterPos = { 0, 0 };
		SMALL_RECT writeArea = { 0,0,width,height };
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				charInfo[i * width + j].Char.UnicodeChar = L'█';
				charInfo[i * width + j].Attributes = screen[i][j];

			}
		}
		WriteConsoleOutputW(hConsole, charInfo, charBufSize, characterPos, &writeArea);
		delete[] charInfo;
	}


	void getKeysPressed(vector<char>* dest) {
		dest->clear();
		for (int i = 0; i < 37; i++) {
			if (GetKeyState(keys[i]) & 0x8000)
				dest->push_back(keys[i]);
		}
	}

	bool isPressed(char k) {
		return GetKeyState(k) & 0x8000;
	}
};
