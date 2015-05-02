#pragma once

#include "resource.h"

// Mouse click constants
#define MOUSE_LEFT	0x0
#define MOUSE_RIGHT 0x1

// Game states
#define STATE_SETUP		0x00
#define STATE_READY		0x01
#define STATE_GAMEOVER	0x02
#define STATE_MAINMENU	0x10

#define GAMEOVER_LOSE	0x00
#define GAMEOVER_WIN	0x01

int APIENTRY			WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow);
LRESULT CALLBACK		WndProc(HWND, UINT, WPARAM, LPARAM);
void					OnPaint();
void					OnResize(UINT, UINT);
void					OnUpdate(float);
void					OnKeyUp(UINT key);
void					DiscardDeviceResources();
void					CreateDeviceResources(const ID2D1HwndRenderTarget&, BOOL&);
void					OnMouseUp(int button);
void					Cleanup();
void					DrawGrid();
void					CenterGridOnClient();
BOOL					GetCellCoords(POINT, LPPOINT);
void					RestartGame(int rowcount, int colcount, BOOL justResetGrid = 0);

void					OnReadyStateForMouse(int mouseButton);