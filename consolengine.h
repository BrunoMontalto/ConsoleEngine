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
#include<cmath>

#include<fcntl.h>
#include<io.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN


//#define _WIN32_WINNT 0x0500
using namespace std;

enum Colors {
	BLACK, DARK_BLUE, DARK_GREEN, WATER, BORDEAUX, PURPLE, GREEN, LIGHT_GRAY, GRAY, BLUE, LIME, LIGHTBLUE, RED, MAGENTA, YELLOW, WHITE,_TRANSPARENT
};

float getDist(int x, int y, int x2, int y2) {
	return sqrt(pow(x - x2, 2) + pow(y - y2, 2));
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
		fill(16);
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

	short colorAt(int x, int y) {
		assert(x >= 0 && x < width&& y >= 0 && y < height);
		return screen[y][x];
	}

	inline void setPixel(int x, int y, short color) {
		if (x >= 0 && x < width && y >= 0 && y < height) {
			screen[y][x] = color;
		}
	}
	// -------------- //

	// drawing //
	void fill(short color) {
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				screen[i][j] = color;
			}
		}
	}

	void drawRect(int x, int y, int width, int height, short color, bool fill = 1) {
		if (fill) {
			for (int i = 0; i < height; i++) {
				for (int j = 0; j < width; j++) {
					setPixel(x + j, y + i, color);
				}
			}
			return;
		}
		for (int i = 0; i < height; i++) {
			setPixel(x, y + i, color);
			setPixel(x + width, y + i, color);
		}
		for (int i = 0; i < width + 1; i++) {
			setPixel(x + i, y, color);
			setPixel(x + i, y + height, color);
		}
	}

	void drawCircle(int x, int y, int radius, short color, bool fill = 1) {
		if (fill) {
			for (int i = 0; i < radius * 2; i++) {
				for (int j = 0; j < radius * 2; j++) {
					if (getDist(x, y, x - radius + j, y - radius + i) < radius) {
						setPixel(x - radius + j, y - radius + i, color);
					}
				}
			}
			return;
		}
		for (int i = 0; i < radius * 2; i++) {
			for (int j = 0; j < radius * 2; j++) {
				if (getDist(x, y, x - radius + j, y - radius + i) < radius && getDist(x, y, x - radius + j, y - radius + i) >= radius - 1) {
					setPixel(x - radius + j, y - radius + i, color);
				}
			}
		}
	}

	void drawLine(int x, int y, int endx, int endy, short color) {
		float angle = atan2(endy - y, endx - x);
		for (int i = 0; i < getDist(x, y, endx, endy); i++) {
			setPixel(x + cos(angle) * i, y + sin(angle) * i, color);
		}
	}


	void draw(Surface surf, int x, int y) {
		for (int i = 0; i < surf.getHeight(); i++) {
			for (int j = 0; j < surf.getWidth(); j++) {
				short color = surf.colorAt(j, i);
				if (color % 17 == 16) continue;
				setPixel(j + x, i + y, color);
			}
		}
	}
	// -------------- //



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
	// -------------- //


	// drawing //
	void fill(short color) {
		updated = 0;
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				screen[i][j] = color;
			}
		}
	}

	void drawRect(int x, int y, int width, int height, short color, bool fill = 1) {
		if (fill) {
			for (int i = 0; i < height; i++) {
				for (int j = 0; j < width; j++) {
					setPixel(x + j, y + i, color);
				}
			}
			return;
		}
		for (int i = 0; i < height; i++) {
			setPixel(x, y + i, color);
			setPixel(x+width, y + i, color);
		}
		for (int i = 0; i < width+1; i++) {
			setPixel(x+i,y, color);
			setPixel(x+i, y + height, color);
		}
	}

	void drawCircle(int x, int y, int radius, short color, bool fill = 1) {
		if (fill) {
			for (int i = 0; i < radius * 2; i++) {
				for (int j = 0; j < radius * 2; j++) {
					if (getDist(x, y, x - radius + j, y - radius + i) < radius) {
						setPixel(x - radius + j, y - radius + i, color);
					}
				}
			}
			return;
		}
		for (int i = 0; i < radius * 2; i++) {
			for (int j = 0; j < radius * 2; j++) {
				if (getDist(x, y, x - radius + j, y - radius + i) < radius && getDist(x, y, x - radius + j, y - radius + i) >= radius -1) {
					setPixel(x - radius + j, y - radius + i, color);
				}
			}
		}
	}

	void drawLine(int x, int y, int endx, int endy, short color) {
		float angle = atan2(endy - y, endx - x);
		for (int i = 0; i < getDist(x, y, endx, endy);i++) {
			setPixel(x + cos(angle) * i, y + sin(angle) * i, color);
		}
	}

	void draw(Surface surf, int x, int y) {
		updated = 0;
		for (int i = 0; i < surf.getHeight(); i++) {
			for (int j = 0; j < surf.getWidth(); j++) {
				short color = surf.colorAt(j, i);
				if (color % 17 == 16) continue;
				setPixel(j + x, i + y, color);
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
	// -------------- //


	// input //
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
	// ----- //
};
