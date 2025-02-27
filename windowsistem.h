#ifndef __WINDOWSISTEM_H
#define __WINDOWSISTEM_H



#include "glwindow.h"
#include "shlobj.h"
#include "windowsx.h"


/**********************************************************************
					DEFINES
 **********************************************************************/


#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 522
#endif


#define SHIFTED 0x8000 


#define PROJECTIONWINDOW		0
#define MODELLWINDOW			1
#define TEXTUREWINDOW			2
#define ONEDWINDOW				3
#define LOGWINDOW				4

#define NUM_CHILD_WINDOWS		5
#define FIRST_CHILD_WINDOW_ID	100


/**********************************************************************
					TYPEDEFS
 **********************************************************************/

typedef struct WINRECT {
	float x,y;
	float width, height;
} *PWINRECT;



/**********************************************************************
					EXTERNALS
 **********************************************************************/

extern WINRECT		g_rect[NUM_CHILD_WINDOWS];


/**********************************************************************
					PREDEFINED HEADERS
 **********************************************************************/


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL	CALLBACK EnumFunc(HWND , LPARAM);

LRESULT CALLBACK WP_ProjectWin(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WP_ModellWin(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WP_TextureWin(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WP_OneDWin(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DlgProc_TexProp(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DlgProc_LightProp(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DlgProc_LoadCatalog(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DlgProc_Smoothness(HWND, UINT, WPARAM, LPARAM);




#endif
