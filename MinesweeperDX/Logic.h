#ifndef __LOGIC_H
#define __LOGIC_H

#include <random>
#include "stdafx.h"

typedef struct {
	int gotBomb;
	int isShown;
	int isFlagged;
	int adjBombs;
	int invokeShow;
	int visible;

	// animation
	int x;
	int y;
	float animPhase;
	int invokedNeighbours;
	float textAlpha;

	// initial animation
	int invokeStartAnim;
	float startAnimPhase;
	int startAnimInvokedNeighbours;
} Box;

void Update(Box**,Box&, float, int, int);
void ClearBox(Box&);
void Reveal(Box** arr, int thisr, int thisc, int rowcount, int colcount, BOOL userInvoked, BOOL invokeStartAnimFunc = 0);
Box** CreateGrid(int rowcount, int colcount, int minecount);
void DestroyGrid(Box*** grid, int rowcount);
void ResetGrid(Box** grid, int rowcount, int colcount, int minecount);
void RecalculateGrid(Box** grid, int rowcount, int colcount);
void CallRevealAround(Box** grid, int thisr, int thisc, int rowcount, int colcount, BOOL userInvoked, BOOL invokeStartAnim = 0);
void CallRevealOrtho(Box** grid, int thisr, int thisc, int rowcount, int colcount, BOOL userInvoked, BOOL invokeStartAnim = 0);
int GetNvisible();

#endif