#ifndef ___NURB_H
#define ___NURB_H

#include "3dmath.h"
#include "stdlib.h"
#include "memory.h"




typedef struct Triangle {
	int p1, p2, p3;
} *TRIANGLE;





#define NUM_CONTROL_POINTS		9


class NURB {
	public:
		CVector3  p[NUM_CONTROL_POINTS];	// list of control points

		CVector3* vbuff3;
		CVector2* vbuff2;
		TRIANGLE tbuff;
		int numpoints;
		int numtriangles;
		int smoothness;

					NURB();		
		void		Init(CVector3 *controlPoints);
		Triangle	GetTrianglePoints(int PointIDX, bool TriangleIDX);
		CVector3	Bezier(CVector3 p1, CVector3 p2, CVector3 p3, float t);
		void		GenerateNURB(bool endcap, int smoothness);
};


#endif