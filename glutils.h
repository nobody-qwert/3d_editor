#ifndef __GLUTILS_H
#define __GLUTILS_H

// Include compatibility header first
#include "compat.h"

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
// glaux.h is handled by compat.h
#include <stdio.h>
#include "3dmath.h"

#ifndef byte	
	typedef unsigned char byte;
#endif

class CAMERA {
	private:
		float heading;
		float updown;
		float dx,dy,dz;	// view
		float x,y,z;
		float ux,uy,uz;	// up vector 
	public:
		CAMERA();
		CVector3	GetPos() {	return CVector3(x,y,z); };
		CVector3	GetDir() {	return CVector3(dx,dy,dz); };
		void Forward(float coef=1.0f);
		void Backward(float coef=1.0f);
		void Left(float coef=1.0f);
		void Right(float coef=1.0f);
		void TurnTo(float hor, float ver);
		void Apply();
};

#define MAX_CHARS	256

class FONT {

	int		m_fontHeight;
	UINT	m_fontListID;

	LPARAM	m_size;			//screen size
	HFONT	m_hOldFont;

public:

			FONT();
	UINT	CreateOpenGLFont(LPSTR strFontName, int height, HDC hDC);
	void	PositionText(int x, int y);
	void	SetScreenSize(LPARAM size);
	void	glPrint(int x, int y, const char *strString, ...);
	void	DestroyFont();
};

class SORT {
	private:
		int *tmp;
		inline void Merge(int *x, int s1, int e1, int s2, int e2);
		void Divide(int *x, int l, int h);
	public:
		void StartSort(int *x, int n);
};

/*************************************************************************
							FUNCTIONS
 *************************************************************************/

extern AUX_RGBImageRec * LoadBMP(char *Filename);
extern bool LoadToOpenGL(char * fname, GLuint &ID, GLuint min, GLuint max);
extern void OrthoMode(int left, int top, int right, int bottom);
extern void PerspectiveMode();

#endif
