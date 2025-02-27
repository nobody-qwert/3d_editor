#ifndef __TEXTUREMANGER_H
#define __TEXTUREMANGER_H


#include "glutils.h"
#include "jpeglib.h"
#include "glext.h"


#define		NONE					-1
#define		MAX_OBJ_TEXTURES		1
#define		MAX_NAME				256


#define		LW						16
#define		LH						16


typedef struct TEXTURELIST {
	UINT	glID;
	UINT	glID_OBJ;

	int		width;
	int		height;		
	char	filename[MAX_NAME];

	int		x;
	int		y;

} *PTEXTURELIST;



class TEXTUREINFO {
	public:
		bool		initialized;

		bool		billboarding;

		float		transparency;
		float		interval;

		char		fname[MAX_OBJ_TEXTURES][MAX_NAME];
		UINT		gl_tex_ID[MAX_OBJ_TEXTURES];
		int			num_textures;
	
		int			minfilter;
		int			maxfilter;

	void operator =(TEXTUREINFO ti);
};


void			AddToOpenGL(byte *data, UINT &ID);
void			NewTextureInfo(TEXTUREINFO &ti);
void			AddTexture(TEXTUREINFO &ti, TEXTURELIST tl, bool first=true);
bool			IsPowOf2(int n);
void			DecodeJPG(jpeg_decompress_struct* cinfo, tImageJPG *pImageData);
tImageJPG*		LoadJPG(const char *filename);
bool			LoadToOpenGL_JPG(char * fname, GLuint &ID, GLuint min, GLuint max);
UINT			CreateTexture(LPSTR filename, int textureID);


#endif
