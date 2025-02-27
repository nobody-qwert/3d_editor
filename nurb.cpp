#include "nurb.h"


CVector3 NURB::Bezier(CVector3 p1, CVector3 p2, CVector3 p3, float t) {
	float  c = (1-t);

	CVector3 p4;
	
	p4 = p3;
	p3 = p2;
	
	CVector3 pointOnCurve;
	
	pointOnCurve.x = (p1.x*c*c*c) + (p2.x*3*t*c*c) + (p3.x*3*t*t*c) + (p4.x*t*t*t);
	pointOnCurve.y = (p1.y*c*c*c) + (p2.y*3*t*c*c) + (p3.y*3*t*t*c) + (p4.y*t*t*t);
	pointOnCurve.z = (p1.z*c*c*c) + (p2.z*3*t*c*c) + (p3.z*3*t*t*c) + (p4.z*t*t*t);

	return pointOnCurve;
}


void NURB::GenerateNURB(bool endcap, int smoothness) {

	CVector3	startPoint, 
				controlPoint,
				endPoint;
	CVector3	p1, p2, p3, p4;
	float		tv,	th;
	float		step;

	if (smoothness<3) smoothness = 3;

	this->smoothness = smoothness;
	
	step = 1.0/smoothness;

	tbuff  = new Triangle[2*smoothness*smoothness];
	vbuff3 = new CVector3[(smoothness+1)*(smoothness+1)];
	vbuff2 = new CVector2[(smoothness+1)*(smoothness+1)];
	

	for(int vv=0; vv<=smoothness; vv++)
		for (int hh=0; hh<=smoothness; hh++) {
			
			
			th = hh*1.0/smoothness;
			tv = vv*1.0/smoothness;

			startPoint	= Bezier(p[0], p[3], p[6], tv);
			controlPoint= Bezier(p[1], p[4], p[7], tv);
			endPoint	= Bezier(p[2], p[5], p[8], tv);

			vbuff3[numpoints] = Bezier(startPoint, controlPoint, endPoint, th);
			numpoints++;			
		}

	for (int i=0; i<smoothness; i++) 
		for (int j=0; j<smoothness; j++) {
			tbuff[numtriangles] = GetTrianglePoints(i*(smoothness+1)+j, true);
			numtriangles++;
			tbuff[numtriangles] = GetTrianglePoints(i*(smoothness+1)+j, false);
			numtriangles++;
		}
}


Triangle NURB::GetTrianglePoints(int PointIDX, bool TriangleIDX) {
	Triangle t;

	if (TriangleIDX) {
		t.p1 = PointIDX;
		t.p2 = PointIDX + smoothness + 1;
		t.p3 = PointIDX + smoothness + 2;		
	} else {
		t.p1 = PointIDX;
		t.p2 = PointIDX + smoothness + 2;
		t.p3 = PointIDX + 1;
	}

	return t;
}


void NURB::Init(CVector3 *point) {
	if (point==NULL) return;	
	memcpy(p, point, NUM_CONTROL_POINTS*sizeof(CVector3));
}



NURB::NURB() {
	numpoints = 0;
	numtriangles = 0;
	smoothness = 3;

	vbuff3 = NULL;
	vbuff2 = NULL;
	tbuff = NULL;
}







