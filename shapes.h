#ifndef ___SHAPES_H
#define ___SHAPES_H

#include "memory.h"
#include "stdlib.h"
#include "3dmath.h"
#include "texturemanager.h"


#define		_X						0
#define		_Y						1
#define		_Z						2


#define		MIN_SIZE				2

#define		DEFAULT_SMOOTHNESS		6
#define		MAX_SMOOTHNESS			128
#define		MIN_SMOOTHNESS			3

#define		NONE				   -1

#define		TEXTURE_COEF			1.0f/128.0f


enum SHAPE_TYPES {	ST_NURB=0,
					ST_BEZIERMESH,
					ST_BCAP,
					ST_ECAP,
					ST_IBCAP,
					ST_IECAP,
					ST_BEVEL,
					ST_BOX,
					ST_ENDCAP,
					ST_NCILINDER,
					ST_NCONE,
					ST_NPLANE,
					ST_SPHERE,
					ST_BULB	};

const int NUM_CONTROLS[14] = {9,4,4,5,4,5,9,8,15,24,17,9,2,8};



class BRUSH {
	public: 

	float	start[3];
	float	end[3];
	int		object;
	bool	active;

	BRUSH();
	BRUSH(CVector3 s, CVector3 e);

	void			operator =(BRUSH brush);
	CVector3		GetStart();
	CVector3		GetEnd();
	void			SetData(CVector3 start, CVector3 end);
	void			SetOrientation(byte orientation);
	byte 			GetOrientation();
};


class SHAPE {
	protected:

	CVector3	*p;
	CVector3    *vbuff3;	
	CVector2    *vbuff2;
	Triangle	*tbuff;
	int			numpoints;	
	int			numtriangles;
	
	int		    smoothness;
	int			type;

	byte		orientation;
	CVector3	centrum;	

	bool		initialized;

	public:

	CVector2	tex_size;
	CVector2	tex_shift;

	
	BRUSH		GetBBoxN(bool autocorrect=true);
	BRUSH		GetBBoxO(bool autocorrect=true);	

	int			GetType() {		return	type;	}
	int			GetSmoothness() { return smoothness; }
	int			GetNumPoints() { return numpoints; }
	int			GetNumTriangles() { return numtriangles; }
	int		    GetNumControlPoints() { return NUM_CONTROLS[type]; }
	CVector3   	GetCenter() { return centrum; };
	CVector2   	GetTextureSize() { return tex_size; };
	CVector2   	GetTextureShift() { return tex_shift; };
	Triangle   *GetTriangleList() { return tbuff; } 
	CVector3   *GetVertexList() { return vbuff3; }
	CVector2   *GetTexCoordList() { return vbuff2; }
	CVector3   *GetControlPointList() { return p; }
	void		SetControlPoints(CVector3 *points);
	void		SetCenter(CVector3 point) { centrum = point; }
	void		SetSmoothness(int smoothness) { this->smoothness = smoothness; }
	void		SetOrientation(byte orientation) { this->orientation = orientation; }
	byte		GetOrientation() { return orientation; }
	bool		IsInitialized() {	return initialized; }	
	void		SetInitialized(bool flag) {	this->initialized=flag;	}
	void		FitTexture();
	void		ShiftTexture(float u, float v);
	void		ScaleTexture(float u, float v);
	void		FreeMemory();
};


class NURB : public SHAPE {
	private:

		Triangle	GetTrianglePoints(int PointIDX, bool TriangleIDX);

	public:	
	
					NURB();
					~NURB() { FreeMemory(); };
		void		Init(CVector3 *controlPoints, int smoothness);
		CVector3	Bezier(CVector3 p1, CVector3 p2, CVector3 p3, float t);		
		void		GenerateShape();
};		


class BEVEL : public SHAPE {
	private:
		
		NURB	m_nurb;

	public: 		

		BEVEL();
		~BEVEL() { FreeMemory(); };
		void	Init(CVector3 bottomStart, CVector3 topEnd, int smoothness);
		void	GenerateShape();
};


class ENDCAP : public SHAPE {
	private:
		
		NURB	slice[2];

	public: 		

		ENDCAP();
		~ENDCAP() { FreeMemory(); };
		void	Init(CVector3 bottomStart, CVector3 topEnd, int smoothness);
		void	GenerateShape();
};


class SPHERE : public SHAPE {
	public:

		SPHERE();
		~SPHERE() { FreeMemory(); };
		void		Init(CVector3 bottom_start, CVector3 top_end, int sides);
		void		GenerateShape();
};


class BOX : public SHAPE {
	public:

		BOX();
		~BOX() { FreeMemory(); };
		void Init(CVector3 bottom_start, CVector3 top_end);
		void GenerateShape();
};



class NPLANE : public SHAPE {
	private:

		NURB	m_nurb;
		
	public:

		NPLANE();
		~NPLANE() { FreeMemory(); };
		// orientation can be: FRONT, TOP, LEFT
		void Init(CVector3 bottom_start, CVector3 top_end, int smoothness, int orientation);
		void GenerateShape();
};



class NCONE : public SHAPE {
	private:
		
		NURB	slice[4];

	public: 		

		NCONE();
		~NCONE() { FreeMemory(); };
		void Init(CVector3 bottomStart, CVector3 topEnd, int smoothness);
		void GenerateShape();
};


class NCILINDER : public SHAPE {
	private:
		
		NURB	slice[4];

	public: 		

		NCILINDER();
		~NCILINDER() { FreeMemory(); };
		void Init(CVector3 bottomStart, CVector3 topEnd, int smoothness);
		void GenerateShape();
};

class BEZIERMESH : public SHAPE {
	private:
		
		CVector3 Bezier3(CVector3 p1, CVector3 p2, CVector3 p3, float t);
		CVector2 Bezier2(CVector2 p1, CVector2 p2, CVector2 p3, float t);	

	public:
		BEZIERMESH();
		~BEZIERMESH() { FreeMemory(); };
		// lowhigh == true -> low
		void Init(CVector3 *points, int smoothness);
		void GenerateShape(bool inverted);
};

class BCAP : public SHAPE {
	private:
		
		BEZIERMESH	m_bm;

	public:
		BCAP();
		~BCAP() { FreeMemory(); };
		// lowhigh == true -> low
		void Init(SHAPE nurb, bool lowhigh);
		void GenerateShape();
};


class ECAP : public SHAPE {
	private:
		
		BEZIERMESH m_bm[2];
		
	public:
		ECAP();
		~ECAP() { FreeMemory(); };
		void Init(SHAPE endcap, bool lowhigh);
		void GenerateShape();
};


class IBCAP : public SHAPE {
	private:
		
		BEZIERMESH	m_bm;

	public:
		IBCAP();
		~IBCAP() { FreeMemory(); };
		void Init(SHAPE nurb, bool lowhigh);
		void GenerateShape();
};

class IECAP : public SHAPE {
	private:
		
		BEZIERMESH m_bm[2];
		
	public:
		IECAP();
		~IECAP() { FreeMemory(); };
		void Init(SHAPE endcap, bool lowhigh);
		void GenerateShape();
};


int append_ptr(byte *dst, byte *src, int size_dst, int size_src, int size);
int GA(int VF, bool first);
int GetMax(float x, float y, float z);


#endif