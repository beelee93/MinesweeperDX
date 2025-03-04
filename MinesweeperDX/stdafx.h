// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here
#include <wincodec.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "Loader.h"
#include "Logic.h"
#include "Graphics.h"
#include "MinesweeperDX.h"

#define PI           3.14159265358979323846 

template<class T> inline void SafeRelease(T **interfaceToRelease)
{
	if (*interfaceToRelease)
	{
		(*interfaceToRelease)->Release();
		(*interfaceToRelease) = NULL;
	}
}