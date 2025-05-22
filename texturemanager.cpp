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
	 - loading BMP images (replacing JPEG)
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
	// Try to load as BMP first, then fall back to our BMP loader
	GLuint texID;
	if (LoadToOpenGL(fname, texID, min, max)) {
		ID = texID;
		return true;
	}
	return false;
}



void AddTexture(TEXTUREINFO &ti, TEXTURELIST tl, bool first) {
	if (!ti.initialized) {
		MessageBoxA(NULL, "at:TEXTUREINFO not initialized!", "Error", MB_ICONEXCLAMATION);
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


tImageJPG *LoadBMP_Internal(const char *filename)
{
	if (!filename) return NULL;

	FILE *File = fopen(filename, "rb");
	if (!File) {
		MessageBoxA(NULL, "Unable to load BMP File!", "Error", MB_OK);
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
	tImageJPG *image = (tImageJPG*)malloc(sizeof(tImageJPG));
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

// Legacy function name for compatibility
tImageJPG *LoadJPG(const char *filename)
{
	// First try to use the LoadBMP from glutils.h
	AUX_RGBImageRec *auxImage = LoadBMP((char*)filename);
	if (auxImage) {
		// Convert AUX_RGBImageRec to tImageJPG
		tImageJPG *image = (tImageJPG*)malloc(sizeof(tImageJPG));
		if (image) {
			image->sizeX = auxImage->sizeX;
			image->sizeY = auxImage->sizeY;
			int imageSize = image->sizeX * image->sizeY * 3;
			image->data = (unsigned char*)malloc(imageSize);
			if (image->data) {
				memcpy(image->data, auxImage->data, imageSize);
			}
		}
		// Free the AUX image
		if (auxImage->data) free(auxImage->data);
		free(auxImage);
		return image;
	}
	
	// Fall back to our internal BMP loader
	return LoadBMP_Internal(filename);
}

// Stub function for compatibility - not used with JPEG library removed
void DecodeJPG(jpeg_decompress_struct* cinfo, tImageJPG *pImageData)
{
	// This function is no longer used since we removed JPEG support
	// Keep it as a stub for compatibility
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
