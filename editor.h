#ifndef __EDITOR_H
#define __EDITOR_H




/*********************************************************************************
								INCLUDES						
 *********************************************************************************/



#include "windowsistem.h"
#include "stdio.h"
#include "stdarg.h"
#include "shapes.h"
#include "texturemanager.h"



/*********************************************************************************
								DEFINES						
 *********************************************************************************/


// view modes
#define		LEFT					_X
#define		TOP						_Y
#define		FRONT					_Z


#define		GL_FONT_HEIGHT			12


#define		MIN_GRID_SIZE			8
#define		MAJOR_GRID_SIZE			64


#define		DS_NONE					 0
#define		DS_MODIFY_OBJECTS		 1
#define		DS_SELECT_CONTROLS		 2
#define		DS_MODIFY_CONTROLS		 3
#define		DS_ROTATE_OBJECTS		 4
#define		DS_ROTATE_CONTROLS		 5
#define		DS_SCALE_OBJECTS		 6

#define		BUFF_SIZE				10000

#define		BULB_ICON_SIZE			32


#define		MAX_BRUSHES				1024
#define		MAX_SELECTED			1024
#define		MAX_OBJECTS				1024
#define		MAX_TEXTURES			1024
#define		MAX_LIGHTS				256


#define		RM_POINT				1
#define		RM_LINE					2
#define		RM_FILL					3

#define		IMAGE_ICON_SPACEING		8
#define		IMAGE_ICON_SIZE			1.0f/4.0f

#define		SCROLL_SIZE				16

#define		DEFAULT_HEIGHT			16

#define		POINT_SIZE				2

#define		CAMERA_STEP				0.5

#define		COORD_UNIT				64.0f
#define		SCALE_COORDS			1.0f/COORD_UNIT

#define		NUM_SCALES				3


#define		TEXTURE_UNIT			256.0f


const char STATUS_NAMES[6][32] = {	"NONE",
									"MODIFY OBJECTS",
									"SELECT CONTROLS",
									"MODIFY CONTROLS",
									"ROTATE OBJECTS",
									"ROTATE CONTROLS"	};


/*********************************************************************************
								TYPEDEFS						
 *********************************************************************************/

class OBJECT : public SHAPE {
	private: 
		TEXTUREINFO texture;

	public:
	
	OBJECT();
	~OBJECT();


	void			Drag(CVector3 v);
	void			Reset(bool flag=true);
	void			SetData(SHAPE shape);
	void			SetType(int shapetype);

	void			SetTextureImage(TEXTURELIST tl);
	UINT			GetGLTexID();
	TEXTUREINFO*	GetPTextureInfo() {	return &texture; };
};

struct BULB {
	CVector3		pos;
	float			r,g,b;
	float			range;
	float			brightness;
	float			cutoff;
};


class EDITOR {
	private:
		bool		initialized;

		FONT		font;
		GLWindow	glwindow;

		int			VF;				// viewed from !!!!!!!!!
		int			gridSize;
		int			mapSize;
		float		scalemap;
		float		vCtr[3];		// the projection window's view center
		float		texturewindowscroll;
		
		int			designstatus;
		bool		activeselection;
		bool		snaptogrid;
		bool		rotate;
		bool		scaleobjects;
		bool		dragvertices;
		bool		locktexrotation;
		bool		locktexmove;
		bool		showlightmaps;
		bool		showbulbs;

		int			smoothness;

		OBJECT		tmpobject;
		OBJECT		object[MAX_OBJECTS];
		int			numobjects;

		BULB		bulb[MAX_LIGHTS];
		int			numbulbs;

		int			bulb_sel[MAX_LIGHTS];
		int			numbulbsel;

		int			rendermode;

		
		BRUSH		tmpbrush;
		BRUSH		brush[MAX_BRUSHES];
		int			numbrushes;

		TEXTURELIST texture[MAX_TEXTURES];
		int			numtextures;

		int			selectedtexture;

		int			selected[MAX_SELECTED][2];
		int			numselected;

		float		sel_start[2];
		float		sel_end[2];
		
		
		// instead static variables
		CVector3	gpoint;
		bool		button1D;
		bool		button2D;
		CVector3	clickpos1D;
		CVector3	clickpos2D;
		bool		dragall;

		UINT		defaulttexture;
		UINT		bulbtexture;

		float		imageiconscale;
		
		PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2fARB;
		PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB;
		

	public:
		CAMERA		camera;


	private:
		void		InitializeFonts();

		void		DrawEye(bool is2D);
		void		DrawAxes(bool is2D);
		void		DrawSelection();
		void		DrawSelectionMark(OBJECT obj, BRUSH br, bool is2D);
		void		DrawSelectionMark3D(OBJECT obj, BRUSH br);
		void		DrawObject2D(OBJECT obj);
		void		DrawObject3D(OBJECT obj, bool texture=true);
		void		DrawBulb(BULB bulb);
		void		DrawBulb2D(BULB bulb);
		void		DrawBulb1D(BULB bulb);

		// Tramnsform Windows Coordinates to polar
		void		TransformWCTo1DPolar(float *y, float factor = 1.0f);
		void		TransformWCTo2DPolar(float *x, float *y, float factor = 1.0f);
		void		Transform1DToPolar(float *z, float factor = 1.0f);
		void		Transform2DToPolar(float *x, float *y, float factor = 1.0f);
		bool		InsideBrush1D(BRUSH b, float v[3]);
		bool		InsideBrush2D(BRUSH b, float v[3]);
		bool		InsideSelection(CVector3 p);

		bool		IsPointInBox2D(CVector3 pos, BRUSH br);

		// returns mouse position in the 1D&2D window
		float   	GetMousePos1D(bool snap=true); 
		CVector2	GetMousePos2D(bool snap=true); 

		
		void		CreateObject(OBJECT &obj, int type, bool lowhigh=false);
		void		Recalc(OBJECT &obj); 
		void		FitToBrush(OBJECT &obj, BRUSH newbrush, BRUSH oldbrush);
		void		ModifyBrush1D(BRUSH &br, OBJECT &obj, float t[3], float v[3], bool drag);
		void		ModifyBrush2D(BRUSH &br, OBJECT &obj, float t[3], float v[3], bool drag);
		void		ScaleBrush(BRUSH &br, OBJECT &obj, float coef);
		bool		AreZeroSizedObjects1D();
		bool		AreZeroSizedObjects2D();
		bool		HitVertex(CVector3 point, CVector3 t);
		bool		IsBoxInBox2D(BRUSH box, BRUSH sel);
		void		RotateObject(OBJECT &obj, float deg, CVector3 centrum);
		void		RotatePoint(CVector3 *point, float deg, CVector3 centrum);
		CVector3	GetHub();	

		void		ApplyNewTexture();

		void		GenerateLightMap(int obj, int tri);
				
	public:

		EDITOR();
		bool		InitWindows();
		void		InitEditor();		
		GLWindow*	GetGLWindow();
		bool		IsInitialized() { return initialized; }
		void		Check_vCtr_Change();
		void		Update();
		void		DrawProjection();
		void		DrawModell();
		void		DrawTexture();
		void		DrawOneD();


		void		CreateObject(int type);
		bool		Actualize();
		void		BackStatus();
		void		SetViewedFrom(int VF);	
		void		SnapToGrid(bool snap);
		bool		Rotation(bool flag);
		bool		EnableScaleObjects(bool flag);
		bool		DragVertices(bool flag);
		void		ScaleMap(float scale);
		void		DeleteObject(int idx, bool freedata=false);
		void		SelectInside();

		void		DeleteBulb(int idx);

		void		InsertLight();

		void		FlushTextures();
		void		AddTexture(char filename[MAX_NAME]);
		void		SetTextureWindowScroll(int scroll);
		void		CalculateTextureScroll();

		void		PickTexture(int x, int y);
		UINT		GetDefaultTexture() { return defaulttexture; }

		bool		GetSmoothness(int &sm);
		void		SetSmoothness(int sm);

		void		SetShowLightmaps(bool flag);

		void		SaveFile(char *filename);
		void		LoadFile(char *filename);
		void		AppendObject(char *filename);

		void		GenerateLightMaps();

		bool		GetLightInfo(BULB &bulb);
		void		SetLightInfo(BULB bulb);

		void		SetShowBulbs(bool flag); 

		void		DeleteSelectedObject();
		void		DuplicateSelectedObject();

		void		LockTextureRotation(bool flag);
		void		LockTextureMove(bool flag);
		bool		GetTextureInfo(CVector2 &size, CVector2 &shift, float &tr, bool &bb);
		void		SetTextureInfo(CVector2 size, CVector2 shift, float tr, bool bb);
		///////////////////////////////////////////////////
		void		ScaleTexture(float u, float v);
		void		ShiftTexture(float u, float v);
		/////////////////////////////////////////////////

		void		SetRenderMode(int mode) { rendermode = mode; }
		
		void		PrintLog(char* text, ...);
		void		DeInitEditor();
		void		Reset();
};


typedef struct _3DW_HEADER {
	int numobjects;
	int numlights;
	int numtextures;
};

typedef struct _3DW_OBJECT {
	int			type;
	int			smoothness;
	int			texture_index;	// index of file stored in the f i l e 
	CVector2	tex_shift;
	CVector2	tex_size;
	float		transparency;
	// control points
	// texture coordonates
};

typedef BULB _3DW_LIGHT;

typedef struct _3DW_TEXTURE {
	int namesize;
	// name
};



/*********************************************************************************
								GLOBAL VARIABLES						
 *********************************************************************************/

extern EDITOR	g_editor;

	
#endif