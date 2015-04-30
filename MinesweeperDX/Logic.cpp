#include "stdafx.h"

// number of visible cells. This is set to zero when startAnim is first invoked
// by "user"
int Nvisible;

int GetNvisible()
{
	return Nvisible;
}

void ClearBox(Box &cell)
{
	cell.adjBombs = 0;
	cell.gotBomb = 0;
	cell.invokeShow = 0;
	cell.isFlagged = 0;
	cell.animPhase = 1.0f;
	cell.x = 0;
	cell.y = 0;
	cell.isShown = 0;
	cell.textAlpha = 0;
	cell.invokedNeighbours = 0;
	cell.invokeStartAnim = 0;
	cell.startAnimPhase = 1.0f;
	cell.startAnimInvokedNeighbours = 0;
	cell.visible = 0;
}


void Update(Box** parent, Box& cell, float elapsed, int rowcount, int colcount)
{
	if (cell.invokeShow && !cell.isShown)
	{
		cell.animPhase -= elapsed * 5.0f;

		if (!cell.invokedNeighbours && cell.animPhase <= 0.8f && !cell.adjBombs)
		{
			int thisc = cell.x;
			int thisr = cell.y;

			CallRevealAround(parent, thisr, thisc, rowcount, colcount, 0);
			cell.invokedNeighbours = 1;
		}

		if (cell.animPhase <= 0.01)
		{
			cell.isShown = 1;
			cell.invokeShow = 0;
		}
	}


	if (cell.isShown == 1)
	{
		// fade in fully
		cell.textAlpha += elapsed * 2.0f;
		if (cell.textAlpha > 1)
		{
			cell.isShown = 2;
			cell.animPhase = 0;
			cell.textAlpha = 1.0f;
		}
	}
	else if (cell.isShown == 2)
	{
		cell.animPhase += elapsed * 1.0f;
		// fade in and out subtly
		cell.textAlpha = 0.85f + cos(PI*cell.animPhase)*0.15f;

		if (cell.animPhase > 2*PI) cell.animPhase -= 2*PI;
	}

	if (cell.invokeStartAnim)
	{
		cell.startAnimPhase -= elapsed * 4.0f;
		if (!cell.startAnimInvokedNeighbours && cell.startAnimPhase <= 0.85f)
		{
			CallRevealOrtho(parent, cell.y, cell.x, rowcount, colcount, 0, 1);
			cell.startAnimInvokedNeighbours = 1;
		}

		if (cell.startAnimPhase <= 0.01)
		{
			cell.invokeStartAnim = 0;
			cell.startAnimPhase = 0;
			cell.visible = 1;
			Nvisible++; // add to count
		}
	}
}

void CallRevealAround(Box** arr, int thisr, int thisc, int rowcount, int colcount, BOOL userInvoked, BOOL invokeStartAnim)
{
	Reveal(arr, thisr - 1, thisc, rowcount, colcount, userInvoked, invokeStartAnim);
	Reveal(arr, thisr + 1, thisc, rowcount, colcount, userInvoked, invokeStartAnim);
	Reveal(arr, thisr - 1, thisc - 1, rowcount, colcount, userInvoked, invokeStartAnim);
	Reveal(arr, thisr + 1, thisc - 1, rowcount, colcount, userInvoked, invokeStartAnim);
	Reveal(arr, thisr, thisc + 1, rowcount, colcount, userInvoked, invokeStartAnim);
	Reveal(arr, thisr, thisc - 1, rowcount, colcount, userInvoked, invokeStartAnim);
	Reveal(arr, thisr - 1, thisc + 1, rowcount, colcount, userInvoked,invokeStartAnim);
	Reveal(arr, thisr + 1, thisc + 1, rowcount, colcount, userInvoked, invokeStartAnim);
}

void CallRevealOrtho(Box** arr, int thisr, int thisc, int rowcount, int colcount, BOOL userInvoked, BOOL invokeStartAnim)
{
	Reveal(arr, thisr - 1, thisc, rowcount, colcount, userInvoked, invokeStartAnim);
	Reveal(arr, thisr + 1, thisc, rowcount, colcount, userInvoked, invokeStartAnim);
	Reveal(arr, thisr, thisc + 1, rowcount, colcount, userInvoked, invokeStartAnim);
	Reveal(arr, thisr, thisc - 1, rowcount, colcount, userInvoked, invokeStartAnim);
}

void Reveal(Box** arr, int thisr, int thisc, int rowcount, int colcount, BOOL userInvoked,BOOL invokeStartAnimFunc)
{
	
	if ((thisr < 0) || (thisr >= rowcount) || (thisc < 0) || (thisc >= colcount)) return; // do not count past edges
	if (arr[thisr][thisc].isShown) return; // do not recount counted squares
	if (!invokeStartAnimFunc)
	{
		if (arr[thisr][thisc].gotBomb || arr[thisr][thisc].isFlagged) return; // nonecount does not include mines, obviously
		
		if (userInvoked)
		{
			arr[thisr][thisc].isShown = 1;
			CallRevealAround(arr, thisr, thisc, rowcount, colcount, 0);
		}
		else
		{
			arr[thisr][thisc].invokeShow = 1;
		}
	}
	else
	{
		if (userInvoked)
		{
			arr[thisr][thisc].invokeStartAnim = 1;
			Nvisible = 0; // no visible cells yet
			CallRevealAround(arr, thisr, thisc, rowcount, colcount, 0,1);
		}
		else
		{
			arr[thisr][thisc].invokeStartAnim = 1;
		}
	}

}

void ResetGrid(Box** grid, int rowcount, int colcount, int minecount)
{
	int& H = rowcount;
	int& W = colcount;
	int h, w;
	// hide and randomise the grid
	for (h = 0; h < H; h++)
		for (w = 0; w < W; w++)
		{
			ClearBox(grid[h][w]);
			grid[h][w].x = w;
			grid[h][w].y = h;
		}

	int thisr, thisc;
	while (minecount > 0)
	{
		thisr = rand() % H;
		thisc = rand() % W;
		if (grid[thisr][thisc].gotBomb == 0)
		{
			grid[thisr][thisc].gotBomb = 1;
			minecount--;
		}
	}

	RecalculateGrid(grid, rowcount, colcount);
}

void RecalculateGrid(Box** grid, int rowcount, int colcount)
{
	if (!grid) return;
	int &H = rowcount;
	int &W = colcount;
	int h, w;
	//fill in the number of adjacent bombs
	//middle parts
	for (h = 1; h < H - 1; h++)
		for (w = 1; w < W - 1; w++)
			grid[h][w].adjBombs = grid[h - 1][w - 1].gotBomb + grid[h - 1][w].gotBomb +
			grid[h - 1][w + 1].gotBomb + grid[h][w - 1].gotBomb +
			grid[h][w + 1].gotBomb + grid[h + 1][w - 1].gotBomb +
			grid[h + 1][w].gotBomb + grid[h + 1][w + 1].gotBomb;
	//sides
	for (w = 1; w < W - 1; w++)
	{
		grid[0][w].adjBombs = grid[0][w - 1].gotBomb + grid[0][w + 1].gotBomb +
			grid[1][w - 1].gotBomb + grid[1][w].gotBomb + grid[1][w + 1].gotBomb;

		grid[H - 1][w].adjBombs = grid[H - 1][w - 1].gotBomb + grid[H - 1][w + 1].gotBomb +
			grid[H - 2][w - 1].gotBomb + grid[H - 2][w].gotBomb + grid[H - 2][w + 1].gotBomb;
	}
	for (h = 1; h < H - 1; h++)
	{
		grid[h][0].adjBombs = grid[h - 1][0].gotBomb + grid[h + 1][0].gotBomb +
			grid[h - 1][1].gotBomb + grid[h][1].gotBomb + grid[h + 1][1].gotBomb;
		grid[h][W - 1].adjBombs = grid[h - 1][W - 1].gotBomb + grid[h + 1][W - 1].gotBomb +
			grid[h - 1][W - 2].gotBomb + grid[h][W - 2].gotBomb + grid[h + 1][W - 2].gotBomb;
	}

	//corners
	grid[0][0].adjBombs = grid[1][0].gotBomb + grid[1][1].gotBomb + grid[0][1].gotBomb;
	grid[H - 1][W - 1].adjBombs = grid[H - 2][W - 1].gotBomb + grid[H - 1][W - 2].gotBomb + grid[H - 2][W - 2].gotBomb;
	grid[0][W - 1].adjBombs = grid[1][W - 1].gotBomb + grid[0][W - 2].gotBomb + grid[1][W - 2].gotBomb;
	grid[H - 1][0].adjBombs = grid[H - 1][1].gotBomb + grid[H - 2][0].gotBomb + grid[H - 2][1].gotBomb;

}

Box** CreateGrid(int rowcount, int colcount, int minecount)
{
	// allocate memory for grid
	Box** grid = NULL;
	grid = new Box*[rowcount];
	for (int i = 0; i < rowcount; i++)
	{
		grid[i] = NULL;
		grid[i] = new Box[colcount];
	}

	ResetGrid(grid, rowcount, colcount, minecount);
	return grid;
}

void DestroyGrid(Box*** grid, int rowcount)
{
	if (*grid)
	{
		// clean up every row
		for (int i = 0; i < rowcount; i++)
		{
			delete[](*grid)[i];
			(*grid)[i] = NULL;
		}

		// now clean up the entire object
		delete[](*grid);
		(*grid) = NULL;
	}
}