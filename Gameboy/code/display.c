#ifndef WINDOWS_H
#define WINDOWS_H
#include <windows.h>
#endif

#ifndef GL_H
#define GL_H
#include <GL/gl.h>
#endif

#include "display.h"
#include "hardware.h"

// Our screen size is 160 x 144. 2 vertices per pixel and 3 colours
GLfloat vertices[2 * SCREEN_WIDTH * SCREEN_HEIGHT];
GLfloat colours[3 * SCREEN_WIDTH * SCREEN_HEIGHT];

void scanLine(GLfloat lineColours[3 * SCREEN_WIDTH], int thisLine)
{
	int i;
	for (i = 0; i < SCREEN_WIDTH * 3; i++)
	{
		colours[SCREEN_WIDTH * (thisLine) * 3 + i] = lineColours[i];
	}
}

void drawScreen()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(2.0f);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glColorPointer(3, GL_FLOAT, 0, colours);
	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glDrawArrays(GL_POINTS, 0, SCREEN_WIDTH * SCREEN_HEIGHT);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	SwapBuffers(hDC);
}

void EnableOpenGL(HWND hwnd, HDC* GLhDC, HGLRC* hRC)
{
	PIXELFORMATDESCRIPTOR pfd;

	int iFormat;

	/* get the device context (DC) */
	*GLhDC = GetDC(hwnd);

	/* set the pixel format for the DC */
	ZeroMemory(&pfd, sizeof(pfd));

	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;

	iFormat = ChoosePixelFormat(*GLhDC, &pfd);

	SetPixelFormat(*GLhDC, iFormat, &pfd);

	/* create and enable the render context (RC) */
	*hRC = wglCreateContext(*GLhDC);

	wglMakeCurrent(*GLhDC, *hRC);
}

void DisableOpenGL(HWND hwnd, HDC GLhDC, HGLRC hRC)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hwnd, GLhDC);
}
