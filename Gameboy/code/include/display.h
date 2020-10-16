#ifndef DISPLAY_H
#define DISPLAY_H

#ifndef WINDOWS_H
#define WINDOWS_H
#include <windows.h>
#endif

#ifndef GL_H
#define GL_H
#include <GL/gl.h>
#endif

GLfloat vertices[2 * 160 * 144];
GLfloat colours[3 * 160 * 144];
HDC hDC;

void scanLine(GLfloat*, int);
void drawScreen(void);
void DisableOpenGL(HWND, HDC, HGLRC);
void EnableOpenGL(HWND, HDC*, HGLRC*);
#endif