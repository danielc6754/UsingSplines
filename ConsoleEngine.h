/*
OneLoneCoder.com - Command Line Game Engine
"Who needs a frame buffer?" - @Javidx9
Disclaimer
~~~~~~~~~~
I don't care what you use this for. It's intended to be educational, and perhaps
to the oddly minded - a little bit of fun. Please hack this, change it and use it
in any way you see fit. BUT, you acknowledge that I am not responsible for anything
bad that happens as a result of your actions. However, if good stuff happens, I
would appreciate a shout out, or at least give the blog some publicity for me.
Cheers!
Background
~~~~~~~~~~
If you've seen any of my videos - I like to do things using the windows console. It's quick
and easy, and allows you to focus on just the code that matters - ideal when you're
experimenting. Thing is, I have to keep doing the same initialisation and display code
each time, so this class wraps that up.
Author
~~~~~~
Twitter: @javidx9
Blog: www.onelonecoder.com
Video:
~~~~~~
https://youtu.be/cWc0hgYwZyc
Added mouse support: https://youtu.be/tdqc9hZhHxM
Last Updated: 30/08/2017
*/

#pragma once
#pragma comment(lib, "winmm.lib")

//#ifndef UNICODE
//#error Please enable UNICODE for the compiler.VS: Project Properties -> General -> \Character Set -> Use Unicode
//#endif

#include <iostream>
#include <vector>
#include <list>
#include <thread>
#include <atomic>
#include <condition_variable>

#include <windows.h>

// Colors in Hex
enum COLOUR {
	FG_BLACK = 0x0000,
	FG_DARK_BLUE = 0x0001,
	FG_DARK_GREEN = 0x0002,
	FG_DARK_CYAN = 0x0003,
	FG_DARK_RED = 0x0004,
	FG_DARK_MAGENTA = 0x0005,
	FG_DARK_YELLOW = 0x0006,
	FG_GREY = 0x0007, // Thanks MS :-/
	FG_DARK_GREY = 0x0008,
	FG_BLUE = 0x0009,
	FG_GREEN = 0x000A,
	FG_CYAN = 0x000B,
	FG_RED = 0x000C,
	FG_MAGENTA = 0x000D,
	FG_YELLOW = 0x000E,
	FG_WHITE = 0x000F,
	BG_BLACK = 0x0000,
	BG_DARK_BLUE = 0x0010,
	BG_DARK_GREEN = 0x0020,
	BG_DARK_CYAN = 0x0030,
	BG_DARK_RED = 0x0040,
	BG_DARK_MAGENTA = 0x0050,
	BG_DARK_YELLOW = 0x0060,
	BG_GREY = 0x0070,
	BG_DARK_GREY = 0x0080,
	BG_BLUE = 0x0090,
	BG_GREEN = 0x00A0,
	BG_CYAN = 0x00B0,
	BG_RED = 0x00C0,
	BG_MAGENTA = 0x00D0,
	BG_YELLOW = 0x00E0,
	BG_WHITE = 0x00F0
};

enum PIXEL_TYPE {
	PIXEL_SOLID = 0x2588,
	PIXEL_ThreeQuarters = 0x2593,
	PIXEL_HALF = 0x2592,
	PIXEL_QUARTER = 0x2591
};

// Sprite Generation
class TemplateSprite {
public:
	TemplateSprite() {
	
	}

	TemplateSprite(int w, int h) {
		Create(w, h);
	}

	TemplateSprite(wstring sFile) {
		if (!Load(sFile))
			Create(8, 8);
	}

	int nWidth = 0;
	int nHeight = 0;

private:
	wchar_t* m_Glyphs = nullptr;
	short* m_Colours = nullptr;

	void Create(int w, int h) {
		nWidth = w;
		nHeight = h;
		m_Glyphs = new wchar_t[w * h];
		m_Colours = new short[w * h];
		for (int i = 0; i < w * h; i++) {
			m_Glyphs[i] = L' ';
			m_Colours[i] = FG_BLACK;
		}
	}

public:
	void SetGlyph(int x, int y, wchar_t c) {
		if (x < 0 || x > nWidth || y < 0 || y > nHeight)
			return;
		else
			m_Glyphs[y * nWidth + x] = c;
	}

	void SetColour(int x, int y, short c) {
		if (x < 0 || x > nWidth || y < 0 || y > nHeight)
			return;
		else
			m_Colours[y * nWidth + x] = c;
	}

	wchar_t GetGlyphs(int x, int y) {
		if (x < 0 || x > nWidth || y < 0 || y > nHeight)
			return L' ';
		else
			return m_Glyphs[y * nWidth + x];
	}

	wchar_t GetColour(int x, int y) {
		if (x < 0 || x > nWidth || y < 0 || y > nHeight)
			return FG_BLACK;
		else
			return m_Colours[y * nWidth + x];
	}

	bool Save(wstring sFile) {
		FILE* f = nullptr;
		_wfopen_s(&f, sFile.c_str(), L"wb");
		if (f == nullptr)
			return false;

		fwrite(&nWidth, sizeof(int), 1, f);
		fwrite(&nHeight, sizeof(int), 1, f);
		fwrite(m_Colours, sizeof(short), nWidth * nHeight, f);
		fwrite(m_Glyphs, sizeof(wchar_t), nWidth * nHeight, f);

		fclose(f);

		return true;
	}

	bool Load(wstring sFile) {
		delete[] m_Glyphs;
		delete[] m_Colours;
		nWidth = 0;
		nHeight = 0;

		FILE* f = nullptr;
		_wfopen_s(&f, sFile.c_str(), L"rb");
		if (f == nullptr)
			return false;
		
		fread(&nWidth, sizeof(int), 1, f);
		fread(&nHeight, sizeof(int), 1, f);

		Create(nWidth, nHeight);

		fread(m_Colours, sizeof(short), nWidth * nHeight, f);
		fread(m_Glyphs, sizeof(wchar_t), nWidth * nHeight, f);

		fclose(f);
		return true;
	}
};

// Game Engine 
class ConsoleTemplateEngine {
public:
	// Game Engine Starting Setup
	ConsoleTemplateEngine() {
		m_nScreenWidth = 80;
		m_nScreenHeight = 30;

		m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		m_hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);

		m_keyNewState = new short[256];
		m_keyOldState = new short[256];
		memset(m_keyNewState, 0, 256 * sizeof(short));
		memset(m_keyOldState, 0, 256 * sizeof(short));

		memset(m_keys, 0, 256 * sizeof(sKeyState));

		//m_mousePosX = 0;
		//m_mousePosY = 0;

		m_sAppName = L"Default";
	}


	int ConstructConsole(int width, int height, int fontw = 12, int fonth = 12) {
		m_nScreenWidth = width;
		m_nScreenHeight = height;

		CONSOLE_FONT_INFOEX cfi;
		cfi.cbSize = sizeof(cfi);
		// Index of font in console font table
		cfi.nFont = 0;
		// contains width and height of each character in font (X is width, Y is height). Coord structure
		cfi.dwFontSize.X = fontw;
		cfi.dwFontSize.Y = fonth;
		// Font pitch and family more info in TEXTMETRIC struct
		cfi.FontFamily = FF_DONTCARE;
		// The thickness of fonts (400 is normal, 700 is bold)
		cfi.FontWeight = FW_NORMAL;
		// Name of typeface
		wcscpy_s(cfi.FaceName, L"Consoles");

		// Sets extended information about the current console font
		if (!SetCurrentConsoleFontEx(m_hConsole, false, &cfi))
			return Error(L"SetCurrentConsoleFontEx");

		COORD coordLargest = GetLargestConsoleWindowSize(m_hConsole);
		if (m_nScreenHeight > coordLargest.Y)
			return Error(L"Game Height Too Large");
		if (m_nScreenWidth > coordLargest.X)
			return Error(L"Game Width Too Large");

		COORD buffer = { (short)m_nScreenWidth, (short)m_nScreenHeight };
		if (!SetConsoleScreenBufferSize(m_hConsole, buffer))
			Error(L"SetConsoleScreenBufferSize");

		m_rectWindow = { 0, 0, (short)m_nScreenWidth - 1, (short)m_nScreenHeight - 1 };
		if (!SetConsoleWindowInfo(m_hConsole, TRUE, &m_rectWindow))
			Error(L"SetConsoleWindowInfo");

		m_bufScreen = new CHAR_INFO[m_nScreenWidth * m_nScreenHeight];

		return 1;
	}

	virtual void Draw(int x, int y, wchar_t c = 0x2588, short col = 0x000F) {
		if (x >= 0 && x < m_nScreenWidth && y >= 0 && m_nScreenHeight) {
			m_bufScreen[y * m_nScreenWidth + x].Char.UnicodeChar = c;
			m_bufScreen[y * m_nScreenWidth + x].Attributes = col;
		}
	}

	void Fill(int x1, int y1, int x2, int y2, wchar_t c = 0x2588, short col = 0x000F) {
		Clip(x1, y1);
		Clip(x2, y2);
		for (int x = x1; x < x2; x++)
			for (int y = y1; y < y2; y++)
				Draw(x, y, c, col);
	}

	void DrawString(int x, int y, wstring c, short col = 0x000F) {
		for (size_t i = 0; i < c.size(); i++) {
			m_bufScreen[y * m_nScreenWidth + x + i].Char.UnicodeChar = c[i];
			m_bufScreen[y * m_nScreenWidth + x + i].Attributes = col;
		}
	}

	void DrawStringAlpha(int x, int y, wstring c, short col = 0x000F) {
		for (size_t i = 0; i < c.size(); i++) {
			if (c[i] != L' ') {
				m_bufScreen[y * m_nScreenWidth + x + i].Char.UnicodeChar = c[i];
				m_bufScreen[y * m_nScreenWidth + x + i].Attributes = col;
			}
		}
	}

	void Clip(int& x, int& y) {
		if (x < 0)
			x = 0;
		if (x >= m_nScreenWidth)
			x = m_nScreenWidth;
		if (y < 0)
			y = 0;
		if (y >= m_nScreenHeight)
			y = m_nScreenHeight;
	}

	void DrawSprite(int x, int y, TemplateSprite *sprite) {
		if (sprite == nullptr)
			return;

		for (int i = 0; i < sprite->nWidth; i++)
			for (int j = 0; j < sprite->nHeight; j++)
				if (sprite->GetGlyphs(i, j) != L' ')
					Draw(x + i, y + j, sprite->GetGlyphs(i, j), sprite->GetColour(i, j));
	}

	void DrawPartialSprite(int x, int y, TemplateSprite *sprite, int ox, int oy, int w, int h) {
		if (sprite == nullptr)
			return;
		
		for (int i = 0; i < w; i++)
			for (int j = 0; j < h; j++)
				if (sprite->GetGlyphs(i + ox, j + oy) != L' ')
					Draw(x + i, y + j, sprite->GetGlyphs(i + ox, j + oy), sprite->GetColour(i + ox, j + oy));
	}

	~ConsoleTemplateEngine() {
		SetConsoleActiveScreenBuffer(m_hOriginalConsole);
		delete[] m_bufScreen;
	}

public:
	void Start() {
		m_bAtomActive = true;

		// Start the thread
		thread t = thread(&ConsoleTemplateEngine::GameThread, this);

		// Wait for thread to exit
		unique_lock<mutex> m_muxLock(m_muxGame);
		m_cvGameFinished.wait(m_muxLock);

		// Tidy up
		t.join();
	}

	int ScreenWidth() {
		return m_nScreenWidth;
	}

	int ScreenHeight() {
		return m_nScreenHeight;
	}
private:
	void GameThread() {
		// Create user rescources at part of this thread
		if (!OnUserCreate())
			return;

		auto tp1 = chrono::system_clock::now();
		auto tp2 = chrono::system_clock::now();

		// Run fast as possible
		while (m_bAtomActive) {
			// Handle Timing
			tp2 = chrono::system_clock::now();
			chrono::duration<float> elapsedTime = tp2 - tp1;
			tp1 = tp2;
			float fElapsedTime = elapsedTime.count();

			// Handle Keyboard Input
			for (int i = 0; i < 256; i++) {
				m_keyNewState[i] = GetAsyncKeyState(i);

				m_keys[i].bPressed = false;
				m_keys[i].bReleased = false;

				if (m_keyNewState[i] != m_keyOldState[i]) {
					if (m_keyNewState[i] & 0x8000) {
						m_keys[i].bPressed = !m_keys[i].bHeld;
						m_keys[i].bHeld = true;
					}

					else {
						m_keys[i].bReleased = true;
						m_keys[i].bHeld = false;
					}
				}

				m_keyOldState[i] = m_keyNewState[i];
			}

			// Check for window events
			INPUT_RECORD inBuf[32];
			DWORD events = 0;
			GetNumberOfConsoleInputEvents(m_hConsoleIn, &events);
			if (events > 0)
				ReadConsoleInput(m_hConsole, inBuf, events, &events);

			// Handle Frame Update
			if (!OnUserUpdate(fElapsedTime))
				m_bAtomActive = false;

			// Update Title & Present Screen Buffer
			wchar_t s[128];
			swprintf_s(s, 128, L"OneLoneCoder.com - Console Game Engine - %s - FPS: %3.2f - %d ", m_sAppName.c_str(), 1.0f / fElapsedTime, events);
			SetConsoleTitle(s);
			WriteConsoleOutput(m_hConsole, m_bufScreen, { (short)m_nScreenWidth, (short)m_nScreenHeight }, { 0, 0 }, &m_rectWindow);
		}

		m_cvGameFinished.notify_one();
	}

public:
	// Override in individual programs
	virtual bool OnUserCreate() = 0;
	virtual bool OnUserUpdate(float fElapsedTime) = 0;

protected:
	int m_nScreenWidth;
	int m_nScreenHeight;
	CHAR_INFO* m_bufScreen;
	atomic<bool> m_bAtomActive;
	condition_variable m_cvGameFinished;
	mutex m_muxGame;
	wstring m_sAppName;

	struct sKeyState {
		bool bPressed;
		bool bReleased;
		bool bHeld;
	} m_keys[256], m_mouse[5];

	//int m_mousePosX;
	//int m_mousePosY;

protected:
	int Error(const wchar_t* msg) {
		wchar_t buf[256];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 256, NULL);
		SetConsoleActiveScreenBuffer(m_hOriginalConsole);
		return -1;
	}

private:
	HANDLE m_hOriginalConsole;
	CONSOLE_SCREEN_BUFFER_INFO m_OriginalConsoleInfo;
	HANDLE m_hConsole;
	HANDLE m_hConsoleIn;
	SMALL_RECT m_rectWindow;
	short* m_keyOldState;
	short* m_keyNewState;
	//bool m_mouseOldState[5];
	//bool m_mouseNewState[5];
};