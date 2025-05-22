#ifndef __GLWINDOW_H
#define __GLWINDOW_H


/******************************************************************
 						INCLUDES
 ******************************************************************/


#include "glutils.h"
#include "commctrl.h"
#include "stdio.h"
#include "resource.h"


/******************************************************************
 						DEFINES
 ******************************************************************/


#define NONE	-1
#define NOMENU  -1
#define MAXSTR	256

// Information ids
#define GI_FULLSCREEN_FLAG		0
#define GI_HINSTANCE			1
#define GI_HWND					2
#define GI_HDC					3
#define GI_HMENU				4
#define GI_CHWND				5
#define GI_CHDC					6
#define GI_CSIZE				7
#define GI_CCENTER				8
#define GI_CLDRAG				9
#define GI_CRDRAG				10
#define GI_CMOUSEPOS			11
#define GI_CMOUSELB				12
#define GI_CMOUSEMB				13
#define GI_CMOUSERB				14
#define GI_CURRENTID			15
#define GI_CMOUSESTRUCT			16



#define SI_CLDRAG				0
#define SI_CRDRAG				1
#define SI_CMOUSEPOS			2
#define SI_CMOUSELB				3
#define SI_CMOUSEMB				4
#define SI_CMOUSERB				5
#define SI_CMOUSEACTIVE			6				


#define STATUS_HEIGHT			15

#define STATUS_WINDOW_ID	    0x00000001

/******************************************************************
 						TYPEDEFS
 ******************************************************************/


typedef char STRING[MAXSTR];

typedef LRESULT (CALLBACK *OPENGLWNDPROC)(HWND, UINT, WPARAM, LPARAM);



typedef struct MOUSESTRUCT {
	POINT	LDrag;
	POINT	RDrag;
	POINT	pos;
	bool	lb, mb, rb;
} *PMOUSESTRUCT;

/******************************************************************
 						CLASSES
 ******************************************************************/


class GLWindow {


private:

	HINSTANCE	 m_hInstance;
	HMENU		 m_hMenu;
	HWND		 m_hWnd;
	HDC		     m_hDC;
	HGLRC	     m_hRC;

	HWND		 m_statushWnd;

	FONT	    *m_font;
	
	HWND	    *m_chWnd;
	HDC		    *m_chDC;
	HGLRC	    *m_chRC;
	STRING	    *m_cCLASSNAMES;
	MOUSESTRUCT *m_mouse;		// stores mouse events in child windows
	int			 m_numChildren;
	
	int			 m_currentID;
	int			 m_bpp;
	bool		 m_FullScreen;
	int			 m_MENU_ID;
	char		 m_CLASSNAME[MAXSTR];
	int			 m_WINDOW_WIDTH;
	int			 m_WINDOW_HEIGHT;

	// Static storage for return values to avoid returning local variables
	static LPARAM m_sizeResult;
	static LPARAM m_centerResult;

	bool	CreateGLWindow(int style, OPENGLWNDPROC WndProc);
	bool	SetupPixelFormat(HDC hDC);
	
public:
	
			GLWindow();	
	bool	Init(int width, int height, int bpp, bool fullscreen,
				 char *title, int style, OPENGLWNDPROC WndProc, 
				 int MENU_ID = NOMENU);
	void	AddChildWindows(HWND *hWnd, char **classnames, int count);
	void	ChangeToFullScreen();
	void	MakeCurrent(int cID);
	void	ResizeOpenGLScreen();
	bool	InitializeOpenGLType(int cID);
	void	PaintFrame();
	void    SetFont(FONT *font);
	void	DeInit();

	void   *GetInfo(int infoID, int param = NONE);
	void   SetInfo(int infoID, void *data, int param = NONE);

	void   PrintStatus(char *text, ...);
};




#endif
