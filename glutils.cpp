#include "glutils.h"



/*************************************************************************
							FONT CLASS
 *************************************************************************/



CAMERA::CAMERA() {
	dx = 0;
	dy = 0;
	dz = -1;

	ux = 0;
	uy = 1;
	uz = 0;

	x = y = 0;
	z = 1;
}

void CAMERA::Apply() {
	gluLookAt(x,y,z,   x+dx,y+dy,z+dz,  ux,uy,uz);
}

void CAMERA::Forward(float step) {
	x += dx * step;
	y += dy * step;
	z += dz * step;
}

void CAMERA::Backward(float step) {
	x -= dx * step;
	y -= dy * step;
	z -= dz * step;
}

void CAMERA::Left(float step) {
	float lx, lz;
	lx = dx;
	lz = dz;

	rot(-90, lx, lz);

	x += lx * step;
	z += lz * step;
}

void CAMERA::Right(float step) {
	float lx, lz;
	lx = dx;
	lz = dz;

	rot(90, lx, lz);

	x += lx * step;
	z += lz * step;
}

void CAMERA::TurnTo(float hor, float ver) {
	heading+=hor;
	updown+=ver;

	if (heading>360.0f) heading -= 360.0f;
	if (heading<0.0f) heading += 360.0f;
	if (updown>360.0f) updown -= 360.0f;
	if (updown<0.0f) updown += 360.0f;

	dx = 0;
	dy = 0;
	dz = -1;

	rot(updown, dz, dy);
	rot(heading, dx, dz);

	ux = 0;
	uy = 1;
	uz = 0;

	rot(updown, uz, uy);
	rot(heading, ux, uz);
	
}





/*************************************************************************
							FONT CLASS
 *************************************************************************/


FONT::
		FONT() 
{
	m_fontListID = 0;
	m_fontHeight= 32;
	m_size = 0;
}


void FONT::SetScreenSize(LPARAM size) {
	m_size = size;
}

UINT FONT::
		CreateOpenGLFont(LPSTR strFontName, int height, HDC hDC)
{
	
	HFONT	hFont;
	
	m_fontHeight	= height;

	m_fontListID	= glGenLists(MAX_CHARS);

	hFont = CreateFontA(height,
					   0,
					   0,
					   0,
					   FW_BOLD,
					   FALSE,
					   FALSE,
					   FALSE,
					   ANSI_CHARSET,
					   OUT_TT_PRECIS,
					   CLIP_DEFAULT_PRECIS,
					   ANTIALIASED_QUALITY,
					   FF_DONTCARE|DEFAULT_PITCH,
					   strFontName);

	m_hOldFont = (HFONT)SelectObject(hDC, hFont);

	wglUseFontBitmaps(hDC, 0, MAX_CHARS - 1, m_fontListID);

	return m_fontListID;
}

void FONT::
		PositionText(int x, int y)
{
	glPushAttrib(GL_TRANSFORM_BIT | GL_VIEWPORT_BIT);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
				glLoadIdentity();

				y = HIWORD(m_size) - m_fontHeight - y;

				glViewport(x - 1, y - 1, 0, 0);
				glRasterPos4f(0, 0, 0, 1);
			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	glPopAttrib();
}


void FONT::
		glPrint(int x, int y, const char *strString, ...)
{
	char		strText[256];
	va_list		argumentPtr;

	if (strString == NULL)
		return;
	
	va_start(argumentPtr, strString);
		vsprintf(strText, strString, argumentPtr);
	va_end(argumentPtr);

	PositionText(x, y);
	
	glPushAttrib(GL_LIST_BIT);
		glDisable(GL_DEPTH_TEST);
		glListBase(m_fontListID);
		glCallLists(strlen(strText), GL_UNSIGNED_BYTE, strText);
		glEnable(GL_DEPTH_TEST);
	glPopAttrib();
}

void FONT::DestroyFont()
{
	glDeleteLists(m_fontListID, MAX_CHARS);	
}


/*************************************************************************
							FUNCTIONS
 *************************************************************************/


// Simple BMP loader to replace auxDIBImageLoad
AUX_RGBImageRec * LoadBMP(char *Filename) 
{
	if (!Filename) return NULL;

	FILE *File = fopen(Filename, "rb");
	if (!File) {
		return NULL;
	}

	// Read BMP header
	unsigned char header[54];
	if (fread(header, 1, 54, File) != 54) {
		fclose(File);
		return NULL;
	}

	// Check if it's a BMP file
	if (header[0] != 'B' || header[1] != 'M') {
		fclose(File);
		return NULL;
	}

	// Get image info
	int width = *(int*)&header[18];
	int height = *(int*)&header[22];
	int bitsPerPixel = *(short*)&header[28];

	// Only support 24-bit BMPs
	if (bitsPerPixel != 24) {
		fclose(File);
		return NULL;
	}

	// Calculate image size
	int imageSize = width * height * 3;
	
	// Allocate memory for image
	AUX_RGBImageRec *image = (AUX_RGBImageRec*)malloc(sizeof(AUX_RGBImageRec));
	if (!image) {
		fclose(File);
		return NULL;
	}

	image->data = (unsigned char*)malloc(imageSize);
	if (!image->data) {
		free(image);
		fclose(File);
		return NULL;
	}

	image->sizeX = width;
	image->sizeY = height;

	// Read image data
	if (fread(image->data, 1, imageSize, File) != imageSize) {
		free(image->data);
		free(image);
		fclose(File);
		return NULL;
	}

	fclose(File);

	// Convert BGR to RGB
	for (int i = 0; i < imageSize; i += 3) {
		unsigned char temp = image->data[i];
		image->data[i] = image->data[i + 2];
		image->data[i + 2] = temp;
	}

	return image;
}


bool LoadToOpenGL(char * fname, GLuint &ID, GLuint min, GLuint max)
{
	bool Status = true;
	AUX_RGBImageRec *TextureImage;

	glGenTextures(1, (GLuint *)(&ID));

	if (TextureImage = LoadBMP(fname))
	{
		glBindTexture(GL_TEXTURE_2D, (GLuint)ID);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage->sizeX, TextureImage->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage->data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, max);
	} else {
		Status = false;
	}
	
	if (TextureImage)
	{
		if (TextureImage->data)
		{
			free(TextureImage->data);
		}
		free(TextureImage);
	}
	
	return Status;
}



void OrthoMode(int left, int top, int right, int bottom) 
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();	

	glLoadIdentity();
	glOrtho(left, right, bottom, top, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();								
}



void PerspectiveMode() 
{
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
}



/**********VERY FAST SORT*****************/


inline void SORT::Merge(int *x, int s1, int e1, int s2, int e2) {
	int		i = s1;
	int		j = s2;
	int		nr = 0;	

	while (i<=e1 && j<=e2) {
		while (x[i]>=x[j] && i<=e1) {
			tmp[nr] = x[i];
			nr++; i++;
		}
		while (x[j]>=x[i] && j<=e2) {
			tmp[nr] = x[j];
			nr++; j++;
		}
	}

	while (i<=e1) {
		tmp[nr] = x[i];
		nr++; i++;
	}
	while (j<=e2) {
		tmp[nr] = x[j];
		nr++; j++;
	}

	memcpy(&x[s1], tmp, nr*sizeof(int));
}


void SORT::Divide(int *x, int l, int h) {
	int c = (l+h)/2;

	if (l!=h) {		
		Divide(x,l,c);
		Divide(x,c+1,h);
	} else return;

	Merge(x, l, c, c+1, h);
}


void SORT::StartSort(int *x, int n) {
	if (n<2) return;

	tmp = new int[n];
	
	Divide(x,0,n-1);
}
