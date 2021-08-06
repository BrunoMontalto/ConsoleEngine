#pragma once
#include<windows.h>
#include<cassert>
#include<iostream>
#include<string>
#include <Windows.h>
#include<fstream>
#include <cwchar>

#include<fcntl.h>
#include<io.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#define _WIN32_WINNT 0x0500
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

void removeScrollBar() {
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
}

void setConsoleSize(int width, int height) {
	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r); //stores the console's current dimensions

	removeScrollBar();
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

	void loadFromMatrix(short *matrix[]) {
		screen = matrix;
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
	int getWidth() {
		return width;
	}

	int getHeight() {
		return height;
	}

	void setSize(int width, int height) {
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

	void clear() {
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				screen[i][j] = 0;
			}
		}
	}

	short colorAt(int x, int y) {
		assert(x >= 0 && x < width&& y >= 0 && y < height);
		//wcout << "y2:" << y << " x2: " << x << endl;
		return screen[y][x];
	}

	void setPixel(int x, int y, short color) {
		if (x >= 0 && x < width && y >= 0 && y < height) {
			screen[y][x] = color;
		}
	}
};


class Console_Engine{
	short** screen;
	int width, height, charSizeX, charSizeY;
	bool updated = 0;

	void resize(int w, int h) {
		setConsoleSize(w * charSizeX, h * charSizeY);
	}

public:
	Console_Engine(int width, int height, short charSizeX = 1, short charSizeY = 1) {
		//set unresizable
		HWND consoleWindow = GetConsoleWindow();
		SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);

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
	int getWidth() {
		return width;
	}

	int getHeight() {
		return height;
	}

	void setSize(int width, int height) {
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

	void setPixel(int x, int y, short color) {
		updated = 0;
		if (x >= 0 && x < width && y >= 0 && y < height) {
			//wcout << "y:" << y << " x: " << x << endl;
			screen[y][x] = color;
		}
	}

	void draw(Surface surf, int x, int y) {
		updated = 0;
		for (int i = 0; i < surf.getHeight(); i++) {
			for (int j = 0; j < surf.getWidth(); j++) {
				setPixel(j+x, i+y, surf.colorAt(j, i));
			}
		}
	}

	void display() {
		if (updated) return;
		updated = 1;
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		CHAR_INFO CI; CI.Char.UnicodeChar = L'█';
		COORD charBufSize = { 1, 1 };
		COORD characterPos = { 0, 0 };
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				CI.Attributes = screen[i][j];
				SMALL_RECT writeArea = {j, i, j, i};
				WriteConsoleOutputW(hConsole, &CI, charBufSize, characterPos, &writeArea);
			}
		}

	}


	void badisplay() {
		if (updated) return;
		updated = 1;
		cls();
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

		
		wchar_t* temp = new wchar_t[width*height + height];
		int len = 0;
		short color = screen[0][0];
		
		for (int i = 0; i < height + 1; i++) {
			for (int j = 0; j < width; j++) {
				if (i * width + j + 1 > height * width) {
					SetConsoleTextAttribute(hConsole, color);
					wcout.write(temp, len);
					delete[] temp; temp = new wchar_t[width * height]; len = 1;
					temp[0] = L'█';
					goto END;
				}
				if (screen[i][j] == color) {
					temp[len++] = L'█';
				}
				else {
					SetConsoleTextAttribute(hConsole, color);
					wcout.write(temp, len);
					delete[] temp; temp = new wchar_t[width * height]; len = 1;
					temp[0] = L'█';
				}
				
				
				color = screen[i][j];

			}
			temp[len++] = L'\n';
		}
		END:wcout.flush();
	}
};
