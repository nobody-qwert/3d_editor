#include "glwindow.h"


void GLWindow::
		AddChildWindows(HWND *hWnd, char **classnames, int count) {

	m_chWnd = new HWND[count];
	m_chDC	= new HDC[count];
	m_chRC  = new HGLRC[count];
	m_cCLASSNAMES = new STRING[count];
	m_mouse = new MOUSESTRUCT[count];

	// setting 0 for the dragging
	memset(m_mouse, 0, sizeof(MOUSESTRUCT)*count);

	memcpy(m_chWnd, hWnd, count * sizeof(HWND));
	
	m_numChildren = count;

	for (int i=0; i<count; i++)
	{
		strcpy(m_cCLASSNAMES[i], classnames[i]);
		m_chDC[i] = NULL;
		m_chRC[i] = NULL;
		memset(&m_mouse[i], 0, sizeof(MOUSESTRUCT));
	}
}

void GLWindow::
		SetInfo(int infoID, void *data, int param) 
{
	switch (infoID) {
		case SI_CLDRAG: {
			int ID = param;

			if (ID<=NONE || ID>=m_numChildren) return;

			if (data==NULL) {
				memset(&m_mouse[ID].LDrag, 0, sizeof(POINT));
			} else {
				m_mouse[ID].LDrag = *((POINT *)data);
			}
			break;
		}
		case SI_CRDRAG: {
			int ID = param;

			if (ID<=NONE || ID>=m_numChildren) return;

			if (data==NULL) {
				memset(&m_mouse[ID].RDrag, 0, sizeof(POINT));
			} else {
				m_mouse[ID].RDrag = *((POINT *)data);
			}
			break;
		}
		case SI_CMOUSEPOS: {
			int ID = param;

			if (ID<=NONE || ID>=m_numChildren) return;

			if (data==NULL) {
				memset(&m_mouse[ID].pos, 0, sizeof(POINT));
			} else {
				m_mouse[ID].pos = *((POINT *)data);
			}
			break;
		}
		case SI_CMOUSELB: {
			int ID = param;

			if (ID<=NONE || ID>=m_numChildren) return;

			if (data==NULL) {
				m_mouse[ID].lb = false;
			} else {
				m_mouse[ID].lb = *((bool *)data);
			}
			break;
		}
		case SI_CMOUSEMB: {
			int ID = param;

			if (ID<=NONE || ID>=m_numChildren) return;

			if (data==NULL) {
				m_mouse[ID].mb = false;
			} else {
				m_mouse[ID].mb = *((bool *)data);
			}
			break;
		}
		case SI_CMOUSERB: {
			int ID = param;

			if (ID<=NONE || ID>=m_numChildren) return;

			if (data==NULL) {
				m_mouse[ID].rb = false;
			} else {
				m_mouse[ID].rb = *((bool *)data);
			}
			break;
		}
	}
}

void *GLWindow::
		GetInfo(int infoID, int param) {

	switch (infoID) {
		case GI_FULLSCREEN_FLAG:
			return (void *)(&m_FullScreen);
		case GI_HINSTANCE:
			return (void *)(&m_hInstance);
		case GI_HWND:
			return (void *)(&m_hWnd);
		case GI_HDC:
			return (void *)(&m_hDC);
		case GI_CHWND: {
			int ID = param;

			if (ID<=NONE || ID>=m_numChildren) return NULL;

			return (void *)(&m_chWnd[ID]);
		}
		case GI_CHDC: {
			int ID = param;

			if (ID<=NONE || ID>=m_numChildren) return NULL;

			return (void *)(&m_chDC[ID]);
		}
		case GI_HMENU:
			return (void *)(&m_hMenu);
		case GI_CSIZE: {
			int ID = param;

			if (ID<=NONE || ID>=m_numChildren) return NULL;
			
			RECT rc;
			GetClientRect(m_chWnd[ID], &rc);
			int width = rc.right - rc.left;
			int height = rc.bottom - rc.top;

			LPARAM res[1] = { MAKELPARAM(width, height) };
			return (void *)(res);
		}
		case GI_CCENTER: {
			int ID = param;

			if (ID<=NONE || ID>=m_numChildren) return NULL;
			
			RECT rc;
			GetClientRect(m_chWnd[ID], &rc);
			int cx = (rc.right + rc.left)/2;
			int cy = (rc.bottom + rc.top)/2;

			LPARAM res[1] = { MAKELPARAM(cx, cy) };
			return (void *)(res);
		}
		case GI_CMOUSESTRUCT: {
			int ID = param;

			if (ID<=NONE || ID>=m_numChildren) return NULL;

			return (void *)(&m_mouse[ID]);
		}
		case GI_CRDRAG: {
			int ID = param;

			if (ID<=NONE || ID>=m_numChildren) return NULL;

			return (void *)(&m_mouse[ID].RDrag);
		}
		case GI_CLDRAG: {
			int ID = param;

			if (ID<=NONE || ID>=m_numChildren) return NULL;

			return (void *)(&m_mouse[ID].LDrag);
		}
		case GI_CMOUSEPOS: {
			int ID = param;

			if (ID<=NONE || ID>=m_numChildren) return NULL;

			return (void *)(&m_mouse[ID].pos);
		}
		case GI_CMOUSELB: {
			int ID = param;

			if (ID<=NONE || ID>=m_numChildren) return NULL;

			return (void *)(&m_mouse[ID].lb);
		}
		case GI_CMOUSEMB: {
			int ID = param;

			if (ID<=NONE || ID>=m_numChildren) return NULL;

			return (void *)(&m_mouse[ID].mb);
		}
		case GI_CMOUSERB: {
			int ID = param;

			if (ID<=NONE || ID>=m_numChildren) return NULL;

			return (void *)(&m_mouse[ID].rb);
		}
		case GI_CURRENTID: {
			return (void *)(&m_currentID);
		}
	}

	return NULL;
}

void GLWindow::
		MakeCurrent(int cID) 
{
	if (cID >= m_numChildren || cID<=NONE) return;

	if (m_font!=NULL) m_font->SetScreenSize(*((LPARAM *)GetInfo(GI_CSIZE, cID)));

	m_currentID = cID;
	wglMakeCurrent(m_chDC[m_currentID], m_chRC[m_currentID]);

	ResizeOpenGLScreen();
}


void GLWindow::
		PrintStatus(char *text, ...)
{
	RECT rc;
	GetClientRect(m_statushWnd, &rc);

	va_list arg_list;
	char buff[MAXSTR];

	va_start(arg_list, text);
		vsprintf(buff, text, arg_list);
	va_end(arg_list);
/*
	HDC hDC = GetDC(m_statushWnd);

	DrawStatusText(hDC, &rc, buff, SBT_NOBORDERS);
	*/
}


void GLWindow::
	SetFont(FONT *font) 
{
	m_font = font;
}


void GLWindow::
		PaintFrame() 
{

	if (m_currentID == NONE) return;

	SwapBuffers(m_chDC[m_currentID]);
}

void GLWindow::
		ResizeOpenGLScreen() 
{
	RECT	rc;
	int		width, height;

	if (m_currentID<=NONE || m_currentID>=m_numChildren) return;

	GetClientRect(m_chWnd[m_currentID], &rc);

	width = rc.right - rc.left;
	height = rc.bottom - rc.top;

	if (height==0)
	{
		height=1;
	}

	glViewport(0,0,width,height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height, 1, 500.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();		
}



bool GLWindow::InitializeOpenGLType(int cID) {
	if (m_chWnd[cID]==NULL) {
		MessageBox(NULL, "The window doesn't exist!", "Error", MB_OK | MB_ICONEXCLAMATION); 
		return false;
	}
	
	m_chDC[cID] = GetDC(m_chWnd[cID]);

	if (m_chDC[cID]==NULL) {
		MessageBox(NULL, "Could not create DC!", "Error", MB_OK | MB_ICONEXCLAMATION); 
		return false;
	}

	if (!SetupPixelFormat(m_chDC[cID]))
        return false;	

	m_chRC[cID] = wglCreateContext(m_chDC[cID]);
	
	return true;
}



GLWindow::
		GLWindow() {

	m_hInstance = NULL;
	m_hMenu		= NULL;
	m_hWnd		= NULL;
	m_hDC		= NULL;
	m_hRC		= NULL;

	m_chWnd		= NULL;
	m_chDC		= NULL;
	m_chRC		= NULL;

	m_mouse		= NULL;

	m_MENU_ID	= NOMENU;
	m_currentID = NONE;

	m_font		= NULL;

	m_numChildren = 0;
}

void GLWindow::
		ChangeToFullScreen() {

	DEVMODE dmSettings;

	memset(&dmSettings,0,sizeof(dmSettings));

	if(!EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&dmSettings))
	{
		MessageBox(NULL, "Could Not Enum Display Settings", "Error", MB_OK);
		return;
	}

	int width = m_WINDOW_WIDTH;
	int height = m_WINDOW_HEIGHT;

	dmSettings.dmPelsWidth	= width;
	dmSettings.dmPelsHeight	= height;
	
	int result = ChangeDisplaySettings(&dmSettings,CDS_FULLSCREEN);	

	if(result != DISP_CHANGE_SUCCESSFUL)
	{
		MessageBox(NULL, "Display Mode Not Compatible", "Error", MB_OK);
		PostQuitMessage(0);
	}
}


bool GLWindow::
		CreateGLWindow(int style, OPENGLWNDPROC WndProc) {

	WNDCLASSEX wc = {0};
	
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.style		 = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc	 = WndProc;
	wc.hInstance	 = m_hInstance;
	wc.hIcon	 	 = LoadIcon(NULL, MAKEINTRESOURCE(IDI_3DW));
	wc.hIconSm		 = LoadIcon(NULL, MAKEINTRESOURCE(IDI_3DW));
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOWFRAME);
	wc.lpszClassName = m_CLASSNAME;
	
	RegisterClassEx(&wc);

	if(m_FullScreen)
	{
		if (!style)
			style = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

		ChangeToFullScreen();
		if (m_MENU_ID==NOMENU) ShowCursor(false);
	}
	else if(!style)
		style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	


	RECT r;

	r.left		= 0;
	r.top		= 0;
	r.right		= m_WINDOW_WIDTH;
	r.bottom	= m_WINDOW_HEIGHT;

	AdjustWindowRect(&r, style, m_MENU_ID==NOMENU ? false:true);

	m_hWnd = CreateWindow(m_CLASSNAME,
						  m_CLASSNAME,
						  style,
						  0,
						  0,
						  r.right - r.left,
						  r.bottom - r.top,
						  NULL,
						  m_hMenu,
						  m_hInstance,
						  NULL);
						  
	if (m_hWnd==NULL) return false;

	ShowWindow(m_hWnd, SW_SHOWMAXIMIZED);
	UpdateWindow(m_hWnd);
	SetFocus(m_hWnd);

	m_hDC = GetDC(m_hWnd);

	m_statushWnd = CreateStatusWindow(WS_CHILD | WS_VISIBLE, 
									  "Initialized", 
									  m_hWnd, 
									  STATUS_WINDOW_ID);

	return true;
}

bool GLWindow::
		SetupPixelFormat(HDC hDC) {

	PIXELFORMATDESCRIPTOR pfd; 
    int pixelformat;
 
    pfd.nSize		 = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion	 = 1;
    pfd.dwFlags		 = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; 
    pfd.dwLayerMask  = PFD_MAIN_PLANE;
    pfd.iPixelType	 = PFD_TYPE_RGBA;
    pfd.cColorBits	 = m_bpp;
    pfd.cDepthBits	 = m_bpp;
    pfd.cAccumBits	 = 0;
    pfd.cStencilBits = 0;
 
    if ( (pixelformat = ChoosePixelFormat(hDC, &pfd)) == FALSE ) 
    { 
        MessageBox(NULL, "ChoosePixelFormat failed", "Error", MB_OK); 
        return false;
    } 
	
    if (SetPixelFormat(hDC, pixelformat, &pfd) == FALSE) 
    { 
        MessageBox(NULL, "SetPixelFormat failed", "Error", MB_OK); 
        return false;
    } 
 
    return true;
}

bool GLWindow::
		Init(int width, int height, int bpp, bool fullscreen,
		     char *title, int style, OPENGLWNDPROC WndProc, 
			 int MENU_ID) {

	m_hInstance		= GetModuleHandle(NULL);

	m_WINDOW_WIDTH	= width;
	m_WINDOW_HEIGHT	= height;

	m_bpp			= bpp;
	m_FullScreen	= fullscreen;
	m_MENU_ID		= MENU_ID;
	strcpy(m_CLASSNAME, title);
	
	if (m_MENU_ID != NOMENU)
		m_hMenu = LoadMenu(m_hInstance, MAKEINTRESOURCE(m_MENU_ID));

	
	if (!CreateGLWindow(style, WndProc)) return false;

	return true;
}

void GLWindow::
		DeInit() {

	for (int i=0; i<m_numChildren; i++)
	{
		if (m_chRC[i]!=NULL)
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(m_chRC[i]);
		}
	
		if (m_chDC[i]) 
			ReleaseDC(m_chWnd[i], m_chDC[i]);


		UnregisterClass(m_cCLASSNAMES[i], m_hInstance);
	}

	
	if(m_FullScreen)
	{
		ChangeDisplaySettings(NULL,0);
	}
}
