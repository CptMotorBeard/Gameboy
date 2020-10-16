#ifndef GPU_H
#define GPU_H

#ifndef GL_H
#define GL_H
#include <GL/gl.h>
#endif

void gpuStep(void);
void cleanLine(void);
void processLine(void);
void renderScanline(void);
void fillOAMFolder(char foldername[]);
void ExportScreen(char foldername[]);
void DEBUG_GPU(void);
void TOGGLE_WINDOW_LAYER(void);
void TOGGLE_BG_LAYER(void);
void TOGGLE_SPRITE_LAYER(void);
void RECORD_GPU_LOGS(void);
#endif
