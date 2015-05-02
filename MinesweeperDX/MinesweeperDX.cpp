// MinesweeperDX.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

Graphics *graphics = NULL;		// graphics engine
Loader* loader = NULL;			// assets loader
Box** playingGrid = NULL;		// defines the mine field

DWORD curr = 0, prev = 0;		// timing purposes

int Nrow = 20;					// grid row count
int Ncol = 20;					// grid column count
int Nmines = 30;				// mine count

float boxSize = 32.0f;			// length of a side of a grid cell
float gridWidth;				// absolute dimensions of the grid
float gridHeight;
D2D_POINT_2F gridTL;			// top left corner of the grid matrix
HWND window;					// handle to window
RECT windowRect;

POINT mousePos = { 0, 0 };					// mouse position
POINT cell = { 0, 0 };						// cell coordinates under cursor
IDWriteTextFormat* pFormat = NULL;			// Mine grid text formatter

float highlightAlpha = 0.8f;				// for the highlighting square
float highlightPhase = 0;
BOOL drawHighlight = 0;

ID2D1Bitmap *pSpriteSheet = NULL;			// pointer to bitmap of sprite sheet
D2D1_RECT_F srcDraw, destDraw;				// drawing offsets

int gameState = STATE_MAINMENU;				// game state
int gameOverCondition = 0;
int clickCount = 0;

// enumerates the different tint colours for different number of bombs
D2D1::ColorF::Enum colorEnum[8] = { D2D1::ColorF::Violet,
									D2D1::ColorF::Indigo,
									D2D1::ColorF::Blue,
									D2D1::ColorF::Green,
									D2D1::ColorF::YellowGreen,
									D2D1::ColorF::Yellow,
									D2D1::ColorF::Orange,
									D2D1::ColorF::Red };

// sprite atlas data
D2D1_RECT_F spriteAtlas[8] = 
{
	{ 0, 0, 48, 48 },				// 0 shown, bomb
	{ 0, 48, 48, 96 },				// 1 shown, empty
	{ 48, 0, 96, 48 },				// 2 flagged
	{ 48, 48, 96, 96 },				// 3 unshown
	{ 128, 0, 428, 96 },			// 4 you win
	{ 0, 128, 300, 224 },			// 5 game over
	{ 428, 0, 476, 48 },			// 6 bomb icon
	{ 428, 48, 476, 96 }			// 7 clock icon
};

double fps = 0;
wchar_t debugMsg[256];

// =========================================================
// Main Entry Point
// =========================================================
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
	curr = GetTickCount();
	srand(time(0));

	// init com
	if (!SUCCEEDED(CoInitialize(NULL)))
	{
		OutputDebugStringA("Unable to initialise COM.\n");
		return 2;
	}

	// create window class
	WNDCLASSEX windowclass;
	ZeroMemory(&windowclass, sizeof(WNDCLASSEX));
	windowclass.cbSize = sizeof(WNDCLASSEX);
	windowclass.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
	windowclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowclass.hInstance = hInstance;
	windowclass.lpfnWndProc = (WNDPROC)WndProc;
	windowclass.style = CS_VREDRAW | CS_HREDRAW;
	windowclass.lpszClassName = L"minesweeper";

	if(!RegisterClassEx(&windowclass))
	{
		Cleanup();
		OutputDebugStringA("Unable to register window class.\n");
		return 2;
	}

	// adjust size to client size
	RECT rc = { 0, 0, 1024, 768 };
	AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, 0, 0);
	
	window = CreateWindow(L"minesweeper", L"Timonesweeper", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
	if (!window)
	{
		Cleanup();
		OutputDebugStringA("Unable to create window.\n");
		return 2;
	}
	GetClientRect(window, &windowRect);

	// create playing grid
	RestartGame(Nrow, Ncol, 1);
	if (!playingGrid)
	{
		Cleanup();
		OutputDebugStringA("Unable to create grid.\n");
		return 2;
	}

	// obtain absolute dimensions
	gridWidth = boxSize * Ncol;
	gridHeight = boxSize * Nrow;
	CenterGridOnClient();


	ShowWindow(window, nCmdShow);
	UpdateWindow(window);

	// Initialise graphics engine
	graphics = new Graphics();
	if (!graphics->Initialize(window))
	{
		Cleanup();
		OutputDebugStringA("Unable to initialize graphics engine.\n");
		return 2;
	}
	graphics->callbackCreateResources = CreateDeviceResources;

	if (!graphics->CreateTextFormat(L"Verdana", 20.0f, &pFormat))
	{
		Cleanup();
		OutputDebugStringA("Unable to initialize grid text formatter.\n");
		return 2;
	}
	pFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

	// initialise asset loader
	loader = new Loader();
	if (!loader->Initialize())
	{
		Cleanup();
		OutputDebugStringA("Failed to initialize asset loader.\n");
		return 2;
	}

	// get message
	MSG msg;
	while (GetMessage(&msg, window, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		
	}

	// cleanup
	Cleanup();

	return 0;
}

// =========================================================
// Performs cleaning up of all resources
// =========================================================
void Cleanup()
{
	if (graphics)
	{
		delete graphics;
		graphics = NULL;
	}

	if (loader)
	{
		delete loader;
		loader = NULL;
	}

	if (playingGrid)
	{
		DestroyGrid(&playingGrid, Nrow);
		playingGrid = NULL;
	}

	if (pFormat)
	{
		SafeRelease(&pFormat);
	}

	DiscardDeviceResources();
	CoUninitialize();
}

// =========================================================
// Window Message Processing
// =========================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_SIZE:
	{
		UINT width = LOWORD(lParam);
		UINT height = HIWORD(lParam);
		OnResize(width, height);
	}
	break;

	case WM_LBUTTONUP:
		mousePos.x = LOWORD(lParam);
		mousePos.y = HIWORD(lParam);
		OnMouseUp(MOUSE_LEFT);
		break;

	case WM_RBUTTONUP:
		mousePos.x = LOWORD(lParam);
		mousePos.y = HIWORD(lParam);
		OnMouseUp(MOUSE_RIGHT);
		break;

	case WM_PAINT:
		prev = curr;
		curr = GetTickCount();

		if (curr - prev > 0)
			fps = 1.0 / (curr - prev) * CLOCKS_PER_SEC;

		OnUpdate((float)(curr - prev) / CLOCKS_PER_SEC);
		OnPaint();
		InvalidateRect(window, NULL, FALSE);
	
		break;

	case WM_KEYUP:
		OnKeyUp(wParam);
		break;

	case WM_MOUSEMOVE:
		mousePos.x = LOWORD(lParam);
		mousePos.y = HIWORD(lParam);
		break;
	
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}

// =========================================================
// On Paint
// =========================================================
void OnPaint()
{
	if (graphics)
	{
		if (graphics->BeginDraw() != DRAWSTATUS_OK)
			return;

		graphics->SetColor(D2D1::ColorF::Black);
		graphics->SetAlpha(1.0f);
		graphics->Clear();

		graphics->SetColor(1.0f, 1.0f, 1.0f);
		graphics->DrawString(debugMsg, 0, 0, 250.0f, 120.0f, 0);

		switch (gameState)
		{
		case STATE_SETUP:
		case STATE_READY:
		case STATE_GAMEOVER:
			DrawGrid();
			break;
		}

		if (graphics->EndDraw() == DRAWSTATUS_INVOKE_DISCARD)
			DiscardDeviceResources();
	}
}

// =========================================================
// Update event, with elapsed time in seconds
// =========================================================
void OnUpdate(float elapsed)
{
	//elapsed /= 10.0f;
	switch (gameState)
	{
	case STATE_READY:
	{
		if ((drawHighlight = GetCellCoords(mousePos, &cell)))
		{
			highlightAlpha = float(0.3f + sin(PI * highlightPhase)*0.1f);
			highlightPhase += float(elapsed * PI / 3.0f);
		}
		else
		{
			highlightPhase = 0;
			highlightAlpha = 0.3f;
		}

		// update each cell
		for (int r = 0; r < Nrow; r++)
			for (int c = 0; c < Ncol; c++)
				Update(playingGrid, playingGrid[r][c], elapsed, Nrow, Ncol);

		// check for winning condition
		if (GetNoneCount() <= 0)
		{
			gameState = STATE_GAMEOVER;
			gameOverCondition = GAMEOVER_WIN;
		}
	}
	break;

	case STATE_SETUP: 
		drawHighlight = 0;

		// check that all cells are visible
		if (GetNvisible() >= Nrow * Ncol)	
			gameState = STATE_READY;

		for (int r = 0; r < Nrow; r++)
			for (int c = 0; c < Ncol; c++)
				Update(playingGrid, playingGrid[r][c], elapsed, Nrow, Ncol);
		break;

	}
	
	swprintf_s(debugMsg, 256, L"READY: %d FPS: %f\tMOUSE: %d,%d NONECOUNT: %d", gameState == STATE_READY ? 1 : 0, fps, cell.x, cell.y, GetNoneCount());
}

// =========================================================
// Responsible for drawing the grid
// =========================================================
void DrawGrid()
{
	if (!playingGrid) return;

	wchar_t txt[2] = L"";

	float x, y, scaledSize, factor;
	for (int r = 0; r < Nrow; r++)
	{
		for (int c = 0; c < Ncol; c++)
		{
			// get absolute coordinates of top left corner of cell
			x = c*boxSize + gridTL.x;
			y = r*boxSize + gridTL.y;

			// destination draw settings
			destDraw.left = x;
			destDraw.top = y;
			destDraw.right = x + boxSize;
			destDraw.bottom = y + boxSize;

			Box &gridCell = playingGrid[r][c];


			if (gridCell.invokeShow) // animate the reveal
			{
				scaledSize = (gridCell.animPhase*0.5f + 1.0f)*boxSize;
				factor = (boxSize - scaledSize) / 2.0f;
				destDraw.left += factor;
				destDraw.top += factor;
				destDraw.bottom = destDraw.top + scaledSize;
				destDraw.right = destDraw.left + scaledSize;

				srcDraw = spriteAtlas[1];
				graphics->DrawSprite(srcDraw, destDraw, pSpriteSheet);
			}
			else if (gridCell.invokeStartAnim) // animate the initial setup of grid cell
			{
				scaledSize = (gridCell.startAnimPhase*0.5f + 1.0f)*boxSize;
				factor = (boxSize - scaledSize) / 2.0f;
				destDraw.left += factor;
				destDraw.top += factor;
				destDraw.bottom = destDraw.top + scaledSize;
				destDraw.right = destDraw.left + scaledSize;

				srcDraw = spriteAtlas[3];
				graphics->DrawSprite(srcDraw, destDraw, pSpriteSheet);
			}
			else if (gridCell.visible)
			{
				if (gridCell.isShown)
				{
					srcDraw.left = 0;
					srcDraw.top = (gridCell.gotBomb ? 0 : 48.0f);
				}
				else
				{
					srcDraw.left = 48.0f;
					srcDraw.top = (gridCell.isFlagged ? 0 : 48.0f);
				}

				srcDraw.bottom = srcDraw.top + 48.0f;
				srcDraw.right = srcDraw.left + 48.0f;
				graphics->DrawSprite(srcDraw, destDraw, pSpriteSheet);

				if (gridCell.isShown && !gridCell.gotBomb && gridCell.adjBombs > 0 && gridCell.adjBombs<9)
				{
					txt[0] = 0x30 + gridCell.adjBombs;

					// tint the cell
					graphics->SetColor(colorEnum[8-gridCell.adjBombs]);
					graphics->SetAlpha(0.25f * gridCell.textAlpha);
					graphics->DrawRectangle(destDraw.left + 1, destDraw.top + 1, boxSize - 2, boxSize - 2, 1.0f, 1);

					// draw the number
					graphics->SetColor(D2D1::ColorF::Black);
					graphics->SetAlpha(gridCell.textAlpha);
					graphics->DrawString(txt, destDraw.left, destDraw.top+3.0f, boxSize, boxSize, pFormat);
					graphics->SetAlpha();
					graphics->SetColor(1.0f, 1.0f, 1.0f);
				}
			}
		}
	}

	// draw the highlighting rectangle
	if (drawHighlight && gameState == STATE_READY && playingGrid[cell.y][cell.x].visible)
	{
		graphics->SetColor(1.0f, 1.0f, 1.0f);
		graphics->SetAlpha(highlightAlpha);
		graphics->DrawRectangle(gridTL.x + cell.x*boxSize, gridTL.y + cell.y*boxSize, boxSize, boxSize, 1.2f, 1);
	}

	if (gameState == STATE_GAMEOVER)
	{
		float wndWidth, wndHeight;
		wndWidth = windowRect.right - windowRect.left;
		wndHeight = windowRect.bottom - windowRect.top;

		// draw a black rectangle
		graphics->SetColor(0, 0, 0);
		graphics->SetAlpha(0.65f);
		destDraw.top = destDraw.left = 0;
		destDraw.right = windowRect.right;
		destDraw.bottom = windowRect.bottom;
		graphics->DrawRectangle(0, 0, wndWidth, wndHeight, 1.0f, 1);
		graphics->SetAlpha();
		graphics->SetColor(1, 1, 1);

		destDraw.left = wndWidth / 2.0f - 150.0f;
		destDraw.top = wndHeight / 2.0f - 48.0f;
		destDraw.right = destDraw.left + 300.0f;
		destDraw.bottom = destDraw.top + 96.0f;
		
		srcDraw = spriteAtlas[gameOverCondition == GAMEOVER_LOSE ? 5 : 4];

		graphics->DrawSprite(srcDraw, destDraw, pSpriteSheet);
	}

}

// =========================================================
// Obtains the cell coordinates directly under the mouse cursor
// =========================================================
BOOL GetCellCoords(POINT mouse, LPPOINT cell)
{
	cell->x = -1;
	cell->y = -1;

	if (mouse.x > gridTL.x && mouse.y > gridTL.y && mouse.x < gridTL.x + gridWidth && mouse.y < gridTL.y + gridHeight)
	{
		cell->x = (int)((mouse.x - gridTL.x) / boxSize);
		cell->y = (int)((mouse.y - gridTL.y) / boxSize);
		return TRUE;
	}

	return FALSE;
}

// =========================================================
// On Resize
// =========================================================
void OnResize(UINT width, UINT height)
{
	if (graphics) graphics->ResizeRenderTarget(width, height);
	CenterGridOnClient();
	GetClientRect(window, &windowRect);
}

// =========================================================
// Centers the grid in the client area
// =========================================================
void CenterGridOnClient()
{
	RECT rc;
	GetClientRect(window,&rc);

	gridTL.x = (rc.right - rc.left) / 2.0f - gridWidth / 2.0f;
	gridTL.y = (rc.bottom - rc.top) / 2.0f - gridHeight / 2.0f;
}

// =========================================================
// Discard Device Resources
// =========================================================
void DiscardDeviceResources()
{
	SafeRelease(&pSpriteSheet);
}

// =========================================================
// Create Device Resources with the given render target
// =========================================================
void CreateDeviceResources(const ID2D1HwndRenderTarget& renderTarget, BOOL &statusOK)
{
	statusOK = loader->LoadBitmapFromResource(MAKEINTRESOURCE(IDB_PNG1), L"PNG", graphics, &pSpriteSheet);
}

// =========================================================
// Handle keyboard up event
// =========================================================
void OnKeyUp(UINT key)
{
	switch (key)
	{
	case 'R':
		RestartGame(Nrow, Ncol);
		break;
	}
}
// =========================================================
// Handle mouse event
// =========================================================
void OnMouseUp(int button)
{
	switch (gameState)
	{
	case STATE_READY:
		OnReadyStateForMouse(button);
		break;
	}

}

// =========================================================
// Restart the game
// =========================================================
void RestartGame(int rowcount, int colcount, BOOL justResetGrid)
{
	gameState = STATE_SETUP;
	clickCount = 0;

	if (Nrow != rowcount && Ncol != colcount)
	{
		// specified grid is of different dimensions
		DestroyGrid(&playingGrid, Nrow);

		// recreate grid with new dimensions
		Nrow = rowcount;
		Ncol = colcount;
	}
	if(!playingGrid)
		playingGrid = CreateGrid(Nrow, Ncol, Nmines);

	if (!playingGrid)
	{
		Cleanup();
		OutputDebugStringA("Cannot create grid.\n");
		return;
	}

	// reset the grid
	ResetGrid(playingGrid, Nrow, Ncol, Nmines);

	if (!justResetGrid)
	{
		// get the middle cell to start showing itself
		Reveal(playingGrid, 0, 0, Nrow, Ncol, 1, 1);
	}
}

// =========================================================
// Handles the mouse up event during STATE_READY
// =========================================================
void OnReadyStateForMouse(int button)
{
	if (GetCellCoords(mousePos, &cell))
	{
		// it is a valid cell
		Box &gridCell = playingGrid[cell.y][cell.x];

		if (gridCell.isShown)
			return;			// cell is already shown, do nothing

		if (button == MOUSE_RIGHT)
		{
			gridCell.isFlagged = (gridCell.isFlagged ? 0 : 1);
		}
		else
		{
			if (gridCell.isFlagged) return;

			if (gridCell.gotBomb)
			{
				if (clickCount > 0) // user already revealed a grid cell previously
				{
					// display all bombs, cause it's over
					for (int r = 0; r < Nrow; r++)
					{
						for (int c = 0; c < Ncol; c++)
						{
							if (playingGrid[r][c].gotBomb)
								playingGrid[r][c].isShown = 1;
						}
					}

					// since we clicked on a bomb, it's game over
					gameState = STATE_GAMEOVER;
					gameOverCondition = GAMEOVER_LOSE;
				}
				else
				{
					// give chance, we move the bomb to another place
					int newx = 0, newy = 0;
					do
					{
						newx = rand() % Nrow;
						newy = rand() % Ncol;
					} while (playingGrid[newy][newx].gotBomb &&
						newx == cell.x && newy == cell.y);

					playingGrid[newy][newx].gotBomb = 1;
					playingGrid[cell.y][cell.x].gotBomb = 0;
					RecalculateGrid(playingGrid, Nrow, Ncol);

					// Now reveal this cell
					Reveal(playingGrid, cell.y, cell.x, Nrow, Ncol, 1);

					// increment click count
					clickCount++;

					OutputDebugStringA("Found a bomb upon first click. Nevermind, moving it to another place.\n");
				}
			}
			else
			{
				// just reveal the cell
				Reveal(playingGrid, cell.y, cell.x, Nrow, Ncol, 1);

				// increment click count
				clickCount++;
			}
		}
	}
}
