#include "editor.h"


WPARAM MainLoop() {
	MSG msg;

	
	while(1)										
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
        { 
			if(msg.message == WM_QUIT)
				break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
		else {				
				g_editor.Update();
		}
	}

	g_editor.DeInitEditor();	

	return(msg.wParam);									// Return from the program
}




int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	if (!g_editor.InitWindows()) { 
		MessageBox(NULL, "Could not create windows!","ERROR", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	g_editor.InitEditor();


	return MainLoop();
}