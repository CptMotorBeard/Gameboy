// Main documentation comes from http://bgb.bircd.org/pandocs.htm
// Also http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf
// Finally http://www.devrs.com/gb/files/opcodes.html

// TODO outside of fixing stuff
//  memory banking can be implemented (cartridge.c)
//  sound can be implemented (apu.c? audio processing unit?)

#include "cartridge.h"
#include "display.h"
#include "hardware.h"
#include "interrupts.h"
#include "timers.h"
#include "cpu.h"
#include "gpu.h"
#include "test_cases.h"

#ifndef WINDOWS_H
#define WINDOWS_H
#include <windows.h>
#endif

#ifndef GL_H
#define GL_H
#include <gl/GL.h>
#endif

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		return 0;

	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	case WM_DESTROY:
		return 0;

		// JOYPAD CONTROLS
	case WM_KEYDOWN:
		switch (wParam) {

		// DEBUG COMMANDS
		case 'L':
			RECORD_GPU_LOGS();
			break;
		case 'H':
			stopped = !stopped;
			break;
		case 'G':
			halt = !halt;
			break;
		case 'S':
			TOGGLE_SPRITE_LAYER();
			break;
		case 'B':
			TOGGLE_BG_LAYER();
			break;
		case 'W':
			TOGGLE_WINDOW_LAYER();
			break;
		case 'D':
			DEBUG_GPU();
			break;
		case 'P':
			fillOAMFolder("oam");
			ExportScreen("oam");
			break;

		// REGULAR COMMANDS
		// Right joypad down
		case VK_RIGHT:
			keys.keys2.right = 0;
			stopped = 0;
			break;

		// Left joypad down
		case VK_LEFT:
			keys.keys2.left = 0;
			stopped = 0;
			break;

		// Up joypad down
		case VK_UP:
			keys.keys2.up = 0;
			stopped = 0;
			break;

		// Down joypad down
		case VK_DOWN:
			keys.keys2.down = 0;
			stopped = 0;
			break;

		// A joypad down
		case 'X':
			keys.keys1.a = 0;
			stopped = 0;
			break;

		// B joypad down
		case 'Z':
			keys.keys1.b = 0;
			stopped = 0;
			break;

		// Select joypad down
		case VK_BACK:
			keys.keys1.select = 0;
			stopped = 0;
			break;

		// Start joypad down
		case VK_RETURN:
			keys.keys1.start = 0;
			stopped = 0;
			break;
		}
		return 0;

	case WM_KEYUP:
		switch (wParam) {
		// Right joypad up
		case VK_RIGHT:
			keys.keys2.right = 1;
			stopped = 0;
			break;

		// Left joypad up
		case VK_LEFT:
			keys.keys2.left = 1;
			stopped = 0;
			break;

		// Up joypad up
		case VK_UP:
			keys.keys2.up = 1;
			stopped = 0;
			break;

		// Down joypad up
		case VK_DOWN:
			keys.keys2.down = 1;
			stopped = 0;
			break;

		// A joypad up
		case 'X':
			keys.keys1.a = 1;
			stopped = 0;
			break;

		// B joypad up
		case 'Z':
			keys.keys1.b = 1;
			stopped = 0;
			break;

		// Select joypad up
		case VK_BACK:
			keys.keys1.select = 1;
			stopped = 0;
			break;

		// Start joypad up
		case VK_RETURN:
			keys.keys1.start = 1;
			stopped = 0;
			break;
		}
		return 0;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{

	//TEST_OPCODES();

	initializeHardware();

	if (!readROM(lpCmdLine))
	{
		exit(1);
	}

	// DEBUG_CARTRIDGE();

	WNDCLASSEX wcex;
	HWND hwnd;

	HGLRC hRC;
	MSG msg;
	BOOL bQuit = FALSE;

	/* register window class */
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_OWNDC;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "GBCScreen";
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


	if (!RegisterClassEx(&wcex))
		return 0;

	/* create main window */
	hwnd = CreateWindowEx(0,
		"GBCScreen",
		mCartridgeHeader.title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		SCREEN_WIDTH * 2,
		SCREEN_HEIGHT * 2,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hwnd, nCmdShow);

	int y;
	for (y = 0; y < SCREEN_HEIGHT; y++)
	{
		int x;
		for (x = 0; x < SCREEN_WIDTH; x++)
		{
			// This is just initializing all the vertices and colours
			// TODO fix this and figure out what everything does
			vertices[(y * 160 + x) * 2] = -1.0f + ((float)x / 80.0f);
			vertices[(y * 160 + x) * 2 + 1] = (float)y / 72.0f - 1.0f;
			colours[(y * 160 + x) * 3] = 1.0f;
			colours[(y * 160 + x) * 3 + 1] = 1.0f;
			colours[(y * 160 + x) * 3 + 2] = 1.0f;
		}

	}

	/* enable OpenGL for the window */
	EnableOpenGL(hwnd, &hDC, &hRC);

	/////////////// MAIN PROGRAM LOOP ///////////////

	while (!bQuit)
	{

		setJoypad();

		if (stopped != 1)
		{
			cpuStep();
		}

		interruptStep();
		gpuStep();
		timerStep();

		// Reset our clock after each cycle
		clock = 0;		

		if (interrupt.timer == 0x01)
		{
			// Enable interrupts after one more cycle
			interrupt.timer = 0xFF;
			interrupt.master = 1;
		}
		else if (interrupt.timer == 0x00)
		{
			// Disable interrupts after one more cycle
			interrupt.timer = 0xFF;
			interrupt.master = 0;
		}

		/* check for messages */
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			/* handle or dispatch messages */
			if (msg.message == WM_QUIT)
			{
				bQuit = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	/////////////// END OF MAIN PROGRAM LOOP ///////////////
		
	/* shutdown OpenGL */
	DisableOpenGL(hwnd, hDC, hRC);
	/* destroy the window explicitly */
	DestroyWindow(hwnd);

	return 0;
}
