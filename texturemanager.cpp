#include "texturemanager.h"


#define		LW						16
#define		LH						16


/**************************CLASS TEXTUREINFO********************************/


void TEXTUREINFO::operator =(TEXTUREINFO ti) {
	this->initialized	= ti.initialized;
	this->interval		= ti.interval;
	this->maxfilter		= ti.maxfilter;
	this->minfilter		= ti.minfilter;
	this->num_textures  = ti.num_textures;
	this->transparency	= ti.transparency;
	this->billboarding  = ti.billboarding;

	memcpy(this->gl_tex_ID, ti.gl_tex_ID, ti.num_textures * sizeof(ti.gl_tex_ID));
	for (int i=0; i<this->num_textures; i++)
		strcpy(this->fname[i], ti.fname[i]);
}



/************************************************************************
							FUNCTIONS
	 - loading JPEG images
	 - creating textures
 ************************************************************************/


bool IsPowOf2(int n) {
	int nr = 0;

	while (n>0 && nr<2) {
		if (n&1) nr++;
		n>>=1;
	}

	return nr == 1;
}


void NewTextureInfo(TEXTUREINFO &ti) {
	ti.initialized			= true;
	ti.transparency			= 0.0f;
	ti.interval				= 100;		// milliseconds
	
	ti.num_textures			= 0;
		
	ti.minfilter			= GL_LINEAR_MIPMAP_NEAREST;
	ti.maxfilter			= GL_LINEAR_MIPMAP_LINEAR;
}

bool LoadToOpenGL_JPG(char * fname, GLuint &ID, GLuint min, GLuint max)
{
	
	tImageJPG *img = LoadJPG(fname);

	if (img==NULL) return false;

	if (!IsPowOf2(img->sizeX) || !IsPowOf2(img->sizeY)) {

		if (img->data) free(img->data);
		free(img);

		return false;
	} else {
		glGenTextures(1, &ID);
		glBindTexture(GL_TEXTURE_2D, ID);

		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, img->sizeX, img->sizeY, GL_RGB, GL_UNSIGNED_BYTE, img->data);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, min);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, max);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);

	}
	
	if (img->data) free(img->data);
	free(img);

	return true;
}



void AddTexture(TEXTUREINFO &ti, TEXTURELIST tl, bool first) {
	if (!ti.initialized) {
		MessageBox(NULL, "at:TEXTUREINFO not initialized!", ERROR, MB_ICONEXCLAMATION);
		return;
	}

	if (first) {
		strcpy(ti.fname[0], tl.filename);
		ti.gl_tex_ID[0] = NONE;
		ti.gl_tex_ID[0] = tl.glID_OBJ;
		ti.num_textures	= 1;
	} else {
		if (ti.num_textures>=MAX_OBJ_TEXTURES) return;

		strcpy(ti.fname[ti.num_textures], tl.filename);
		ti.gl_tex_ID[ti.num_textures] = NONE;
		ti.num_textures++;
	}
}


/*********************************************************************************/



void DecodeJPG(jpeg_decompress_struct* cinfo, tImageJPG *pImageData)
{
	jpeg_read_header(cinfo, TRUE);
	
	jpeg_start_decompress(cinfo);

	pImageData->rowSpan = cinfo->image_width * cinfo->num_components;
	pImageData->sizeX   = cinfo->image_width;
	pImageData->sizeY   = cinfo->image_height;
	
	pImageData->data = new unsigned char[pImageData->rowSpan * pImageData->sizeY];
		
	unsigned char** rowPtr = new unsigned char*[pImageData->sizeY];
	for (int i = 0; i < pImageData->sizeY; i++)
		rowPtr[i] = &(pImageData->data[i*pImageData->rowSpan]);

	int rowsRead = 0;
	while (cinfo->output_scanline < cinfo->output_height) 
	{
		rowsRead += jpeg_read_scanlines(cinfo, &rowPtr[rowsRead], cinfo->output_height - rowsRead);
	}
	
	delete [] rowPtr;

	jpeg_finish_decompress(cinfo);
}


void AddToOpenGL(byte *data, UINT &ID)
{
	glGenTextures(1, (GLuint *)(&ID));
	glBindTexture(GL_TEXTURE_2D, ID);

	glTexImage2D(GL_TEXTURE_2D, 0, 3, LW, LH, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
}



tImageJPG *LoadJPG(const char *filename)
{
	jpeg_decompress_struct	 cinfo;
	tImageJPG						*pImageData;
	FILE							*pFile;
		
	pImageData = NULL;


	if((pFile = fopen(filename, "rb")) == NULL) 
	{
		MessageBox(NULL, "Unable to load JPG File!", "Error", MB_OK);
		return NULL;
	}
	
	jpeg_error_mgr jerr;


	cinfo.err = jpeg_std_error(&jerr);
	
	jpeg_create_decompress(&cinfo);
	
		jpeg_stdio_src(&cinfo, pFile);	
		
		pImageData = (tImageJPG*)malloc(sizeof(tImageJPG));

		DecodeJPG(&cinfo, pImageData);
	
	jpeg_destroy_decompress(&cinfo);
	
	fclose(pFile);

	return pImageData;
}



UINT CreateTexture(LPSTR filename, int textureID)
{
	if(!filename) return NONE;
	
	tImageJPG *pImage = LoadJPG(filename);

	if (pImage == NULL) return NONE;

	UINT ID;

	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);

	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, pImage->sizeX, pImage->sizeY, GL_RGB, GL_UNSIGNED_BYTE, pImage->data);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);	
	
	if (pImage)
	{
		if (pImage->data)
		{
			free(pImage->data);
		}
		free(pImage);
	}

	return ID;
}
