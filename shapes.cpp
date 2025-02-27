#include "shapes.h"



int append_ptr(byte *dst, byte *src, int size_dst, int size_src, int size) {
	memcpy((dst+size*size_dst), src, size*size_src); 	
	return size_dst + size_src;
}

int GA(int VF, bool first) {
	switch(VF) {
		case _X: return !first ? _Z:_Y; 
		case _Y: return !first ? _X:_Z; 
		case _Z: return !first ? _X:_Y;
	}
	return -1;
}

int GetMax(float x, float y, float z) {
	if (x>y && x>z) return _X;
	if (y>x && y>z) return _Y;
	
	return _Z;
}


/******************************BRUSH CLASS*********************************************/


BRUSH::BRUSH() {
	object = NONE;
	active = false;
}

BRUSH::BRUSH(CVector3 s, CVector3 e)
{	
	start[_X] = s.x; start[_Y] = s.y; start[_Z] = s.z; 
	end[_X] = e.x; end[_Y] = e.y; end[_Z] = e.z;
	active = false;
	object = NONE;
}

void BRUSH::operator=(BRUSH brush)	
{ 
	start[0] = brush.start[0]; start[1] = brush.start[1]; start[2] = brush.start[2];
	end[0] = brush.end[0]; end[1] = brush.end[1]; end[2] = brush.end[2];
	object = brush.object;
	active = brush.active;
}

void BRUSH::SetOrientation(byte or) {
	if (DIR(end[_X]-start[_X])!=(or&1))
		swap(start[_X], end[_X]);
	if (DIR(end[_Y]-start[_Y])!=((or>>1)&1))
		swap(start[_Y], end[_Y]);
	if (DIR(end[_Z]-start[_Z])!=((or>>2)&1))
		swap(start[_Z], end[_Z]);
}

byte BRUSH::GetOrientation() {
	return	DIR(end[_X]-start[_X]) | (DIR(end[_Y]-start[_Y])<<1) | (DIR(end[_Z]-start[_Z])<<2);
}

void BRUSH::SetData(CVector3 s, CVector3 e) {
	start[_X] = s.x;
	start[_Y] = s.y;
	start[_Z] = s.z;
	end[_X] = e.x;
	end[_Y] = e.y;
	end[_Z] = e.z;
}

CVector3 BRUSH::GetStart() {	
	return CVector3(start[_X], start[_Y], start[_Z]);
}

CVector3 BRUSH::GetEnd() {
	return CVector3(end[_X], end[_Y], end[_Z]);
}




/******************************SHAPE CLASS*********************************************/


void SHAPE::SetControlPoints(CVector3 *points) {
	if (p==NULL) p = new CVector3[NUM_CONTROLS[type]];
	memcpy(p, points, NUM_CONTROLS[type]*sizeof(CVector3));
}


void SHAPE::FreeMemory() {
	numpoints = numtriangles = 0;

	if (p!=NULL) free(p);
	if (vbuff3!=NULL) free(vbuff3);
	if (vbuff2!=NULL) free(vbuff2);
	if (tbuff!=NULL) free(tbuff);

	vbuff3 = NULL;
	vbuff2 = NULL;
	tbuff = NULL;
	p = NULL;

	initialized = false;
}


void SHAPE::ShiftTexture(float u, float v) {
	tex_shift = tex_shift + CVector2(u,v);
}

void SHAPE::ScaleTexture(float u, float v) {
	tex_size = tex_size + CVector2(u,v);
	if (tex_size.x < 0) tex_size.x = 0.0f;
	if (tex_size.y < 0) tex_size.y = 0.0f;
}

void SHAPE::FitTexture() {
	BRUSH		bbox;
	CVector3	normal;
	CVector3	p[3];
	float		v[3];
	float		t[3];
	int			AV;


	if (type==ST_NCILINDER || type==ST_NCONE || type==ST_BEVEL || 
		type==ST_SPHERE || type==ST_ENDCAP) return;

	bbox = GetBBoxO(false);

	t[_X] = bbox.end[_X] - bbox.start[_X];
	t[_Y] = bbox.end[_Y] - bbox.start[_Y];
	t[_Z] = bbox.end[_Z] - bbox.start[_Z];

	for(int i=0; i<numtriangles; i++) {
		p[0] = vbuff3[tbuff[i].p1];
		p[1] = vbuff3[tbuff[i].p2];
		p[2] = vbuff3[tbuff[i].p3];
		
		normal	= Normal(p);

		normal.x = fabs(normal.x);
		normal.y = fabs(normal.y);
		normal.z = fabs(normal.z);

		AV		= GetMax(normal.x, normal.y, normal.z);
		
		v[_X] = p[0].x;
		v[_Y] = p[0].y;
		v[_Z] = p[0].z;
		vbuff2[tbuff[i].p1].x = (v[GA(AV,0)] - bbox.start[GA(AV,0)]) / t[GA(AV,0)];
		vbuff2[tbuff[i].p1].y = (v[GA(AV,1)] - bbox.start[GA(AV,1)]) / t[GA(AV,1)];
		vbuff2[tbuff[i].p1].x*= t[GA(AV,0)] * TEXTURE_COEF;
		vbuff2[tbuff[i].p1].y*= t[GA(AV,1)] * TEXTURE_COEF;
		v[_X] = p[1].x;
		v[_Y] = p[1].y;
		v[_Z] = p[1].z;
		vbuff2[tbuff[i].p2].x =  (v[GA(AV,0)] - bbox.start[GA(AV,0)]) / t[GA(AV,0)];
		vbuff2[tbuff[i].p2].y = (v[GA(AV,1)] - bbox.start[GA(AV,1)]) / t[GA(AV,1)];
		vbuff2[tbuff[i].p2].x*= t[GA(AV,0)] * TEXTURE_COEF;
		vbuff2[tbuff[i].p2].y*= t[GA(AV,1)] * TEXTURE_COEF;
		v[_X] = p[2].x;
		v[_Y] = p[2].y;
		v[_Z] = p[2].z;
		vbuff2[tbuff[i].p3].x = (v[GA(AV,0)] - bbox.start[GA(AV,0)]) / t[GA(AV,0)];
		vbuff2[tbuff[i].p3].y = (v[GA(AV,1)] - bbox.start[GA(AV,1)]) / t[GA(AV,1)];
		vbuff2[tbuff[i].p3].x*= t[GA(AV,0)] * TEXTURE_COEF;
		vbuff2[tbuff[i].p3].y*= t[GA(AV,1)] * TEXTURE_COEF;
	}
}


BRUSH SHAPE::GetBBoxO(bool autocorrect) {
	BRUSH	bbox;	
	
	if (!initialized) return bbox;

	bbox.start[_X] = p[0].x;
	bbox.start[_Y] = p[0].y;
	bbox.start[_Z] = p[0].z;
	bbox.end[_X] = p[0].x;
	bbox.end[_Y] = p[0].y;
	bbox.end[_Z] = p[0].z;

	for (int i=0; i<NUM_CONTROLS[type]; i++) {
		if (p[i].x<bbox.start[_X]) bbox.start[_X] = p[i].x;
		if (p[i].y<bbox.start[_Y]) bbox.start[_Y] = p[i].y;
		if (p[i].z<bbox.start[_Z]) bbox.start[_Z] = p[i].z;

		if (p[i].x>bbox.end[_X]) bbox.end[_X] = p[i].x;
		if (p[i].y>bbox.end[_Y]) bbox.end[_Y] = p[i].y;
		if (p[i].z>bbox.end[_Z]) bbox.end[_Z] = p[i].z;
	}

	for (i=0; i<3; i++) {
		if (fabs(bbox.start[i]-bbox.end[i])<NEAR_ZERO && autocorrect) {
			float dir = bbox.end[i]-bbox.start[i];
			
			bbox.end[i] += SGN(dir)*MIN_SIZE;
			bbox.start[i] -= SGN(dir)*MIN_SIZE;
		}

		if ((1&(orientation>>i))!=DIR(bbox.end[i]-bbox.start[i]))
			swap(bbox.start[i], bbox.end[i]);
	}

	bbox.active = true;

	return bbox;
}


BRUSH SHAPE::GetBBoxN(bool autocorrect) {
	BRUSH	bbox;

	if (!initialized) return bbox;

	if (NUM_CONTROLS[type]==0) return bbox;

	bbox.start[_X] = p[0].x;
	bbox.start[_Y] = p[0].y;
	bbox.start[_Z] = p[0].z;
	bbox.end[_X] = p[0].x;
	bbox.end[_Y] = p[0].y;
	bbox.end[_Z] = p[0].z;

	for (int i=0; i<NUM_CONTROLS[type]; i++) {
		if (p[i].x<bbox.start[_X]) bbox.start[_X] = p[i].x;
		if (p[i].y<bbox.start[_Y]) bbox.start[_Y] = p[i].y;
		if (p[i].z<bbox.start[_Z]) bbox.start[_Z] = p[i].z;

		if (p[i].x>bbox.end[_X]) bbox.end[_X] = p[i].x;
		if (p[i].y>bbox.end[_Y]) bbox.end[_Y] = p[i].y;
		if (p[i].z>bbox.end[_Z]) bbox.end[_Z] = p[i].z;
	}

	for (i=0; i<3; i++)
		if (fabs(bbox.start[i]-bbox.end[i])<NEAR_ZERO && autocorrect) {
			float dir = bbox.end[i]-bbox.start[i];
			
			bbox.end[i] += SGN(dir)*MIN_SIZE;
			bbox.start[i] -= SGN(dir)*MIN_SIZE;
		}

	bbox.active = true;

	return bbox;
}


/******************************SPHERE CLASS*********************************************/


void SPHERE::GenerateShape() {
	if (!initialized) {
		vbuff3 = new CVector3[(smoothness+1)*(smoothness+1)];
		vbuff2 = new CVector2[(smoothness+1)*(smoothness+1)];
		tbuff  = new Triangle[2*smoothness*smoothness];
		initialized = true;
	}

	float		x,y,z;

	CVector3	center;
	CVector3	radius;

	center = (p[0]+p[1])/2;
	radius = CVector3(fabs(p[0].x-p[1].x)/2, fabs(p[0].y-p[1].y)/2, fabs(p[0].z-p[1].z)/2);

	numpoints = numtriangles = 0;

	for (int ii = 0; ii<=smoothness; ii++) {
		for (int jj=0; jj<=smoothness; jj++) {
	
			x=0; y=1; z=0;

			rot((-180.0f*ii)/smoothness, x, y);
			rot((-360.0f*jj)/smoothness, x, z);

			x*=radius.x;
			y*=radius.y;
			z*=radius.z;

			vbuff3[numpoints] = center + CVector3(x,y,z);
			vbuff2[numpoints] = CVector2(jj/(float)smoothness,
										 (smoothness-ii)/(float)smoothness);
			numpoints++;			
		}		
	}
	int idx;

	for (ii=0; ii<smoothness; ii++) 
		for (int jj=0; jj<smoothness; jj++) {
		
			idx = ii*(smoothness+1) + jj;
			
			tbuff[numtriangles].p1 = idx;
			tbuff[numtriangles].p2 = idx+1;
			tbuff[numtriangles].p3 = idx+smoothness+1;
			numtriangles++;

			tbuff[numtriangles].p1 = idx+1;
			tbuff[numtriangles].p2 = idx+smoothness+2;
			tbuff[numtriangles].p3 = idx+smoothness+1;
			numtriangles++;
	}
}

void SPHERE::Init(CVector3 s, CVector3 e, int sm) {
	smoothness	= sm<3 ? 3:sm;

	p[0] = s;
	p[1] = e;

	if (p==NULL) p=new CVector3[NUM_CONTROLS[type]];
	vbuff3 = new CVector3[(smoothness+1)*(smoothness+1)];
	vbuff2 = new CVector2[(smoothness+1)*(smoothness+1)];
	tbuff  = new Triangle[2*smoothness*smoothness];
	initialized = true;
}


SPHERE::SPHERE() {
	type = ST_SPHERE;

	numpoints = 0;
	numtriangles = 0;

	vbuff3 = NULL;
	vbuff2 = NULL;
	tbuff = NULL;
	initialized = false;

	p = new CVector3[NUM_CONTROLS[type]];
}


/******************************NURB CLASS*********************************************/


CVector3 NURB::Bezier(CVector3 p1, CVector3 p2, CVector3 p3, float t) {
	float  c = (1-t);

	CVector3 pointOnCurve;

	pointOnCurve.x = (p1.x*c*c) + (p2.x*2*t*c) + (p3.x*t*t);
	pointOnCurve.y = (p1.y*c*c) + (p2.y*2*t*c) + (p3.y*t*t);
	pointOnCurve.z = (p1.z*c*c) + (p2.z*2*t*c) + (p3.z*t*t);

	return pointOnCurve;
}


void NURB::GenerateShape() {
	if (!initialized) {
		tbuff  = new Triangle[2*smoothness*smoothness];
		vbuff3 = new CVector3[(smoothness+1)*(smoothness+1)];
		vbuff2 = new CVector2[(smoothness+1)*(smoothness+1)];
		initialized = true;
	}

	CVector3	startPoint, 
				controlPoint,
				endPoint;
	CVector3	p1, p2, p3, p4;
	float		tv,	th;

	numpoints = 0;
	numtriangles = 0;
	
	for(int vv=0; vv<=smoothness; vv++)
		for (int hh=0; hh<=smoothness; hh++) {
			th = hh*1.0/smoothness;
			tv = vv*1.0/smoothness;

			startPoint	= Bezier(p[0], p[3], p[6], tv);
			controlPoint= Bezier(p[1], p[4], p[7], tv);
			endPoint	= Bezier(p[2], p[5], p[8], tv);

			vbuff3[numpoints] = Bezier(startPoint, controlPoint, endPoint, th);
			vbuff2[numpoints] = CVector2(th,tv);
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


//---------------------------------------------
//void NURB::Init(CVector3 *point, int sm)
//
// - has to be initialized with 9 control points 
//		6---7---8
//		|	|	|
//      3	4	5  
//		|	|	|
//      0---1---2
// endpoints: 0,2,6,8
// curve controls: 1,3,4,5,7
//---------------------------------------------
void NURB::Init(CVector3 *point, int sm) {
	if (point==NULL) return;	

	smoothness	= sm<3 ? 3:sm;	

	if (p==NULL) p=new CVector3[NUM_CONTROLS[type]];

	tbuff  = new Triangle[2*smoothness*smoothness];
	vbuff3 = new CVector3[(smoothness+1)*(smoothness+1)];
	vbuff2 = new CVector2[(smoothness+1)*(smoothness+1)];
	initialized = true;

	memcpy(p, point, NUM_CONTROLS[type]*sizeof(CVector3));	
}



NURB::NURB() {
	type = ST_NURB;

	numpoints = 0;
	numtriangles = 0;
	smoothness = 3;
	
	vbuff3 = NULL;
	vbuff2 = NULL;
	tbuff = NULL;
	initialized = false;

	p = new CVector3[NUM_CONTROLS[type]];
}


/******************************BOX CLASS*********************************************/



void BOX::Init(CVector3 s, CVector3 e) {
	vbuff3 = new CVector3[24];
	vbuff2 = new CVector2[24];
	tbuff = new Triangle[12];
	initialized = true;
	
	if (p==NULL) p=new CVector3[NUM_CONTROLS[type]];

	p[0] = CVector3(s.x, s.y, e.z);
	p[1] = s;
	p[2] = CVector3(e.x, s.y, s.z);
	p[3] = CVector3(e.x, s.y, e.z);
	p[4] = CVector3(s.x, e.y, e.z);
	p[5] = e;
	p[6] = CVector3(e.x, e.y, s.z);
	p[7] = CVector3(s.x, e.y, s.z);
}


void BOX::GenerateShape() {
	if (!initialized) {
		vbuff3 = new CVector3[24];
		vbuff2 = new CVector2[24];
		tbuff = new Triangle[12];
		initialized = true;
	}

	numpoints = numtriangles = 0;

	int nr = 0;

	// BOTTOM
	tbuff[numtriangles].p1 = nr;
	tbuff[numtriangles].p2 = nr+1;
	tbuff[numtriangles].p3 = nr+2;
	numtriangles++;
	tbuff[numtriangles].p1 = nr;
	tbuff[numtriangles].p2 = nr+2;
	tbuff[numtriangles].p3 = nr+3;
	numtriangles++;
	vbuff3[nr] = p[3];
	vbuff2[nr] = CVector2(0,1);
	nr++;
	vbuff3[nr] = p[2];
	vbuff2[nr] = CVector2(0,0);
	nr++;
	vbuff3[nr] = p[1];
	vbuff2[nr] = CVector2(1,0);
	nr++;
	vbuff3[nr] = p[0];
	vbuff2[nr] = CVector2(1,1);
	nr++;
	
	// RIGHT
	tbuff[numtriangles].p1 = nr;
	tbuff[numtriangles].p2 = nr+1;
	tbuff[numtriangles].p3 = nr+2;
	numtriangles++;
	tbuff[numtriangles].p1 = nr;
	tbuff[numtriangles].p2 = nr+2;
	tbuff[numtriangles].p3 = nr+3;
	numtriangles++;
	vbuff3[nr] = p[5];
	vbuff2[nr] = CVector2(0,1);
	nr++;
	vbuff3[nr] = p[6];
	vbuff2[nr] = CVector2(1,1);
	nr++;
	vbuff3[nr] = p[2];
	vbuff2[nr] = CVector2(1,0);
	nr++;
	vbuff3[nr] = p[3];
	vbuff2[nr] = CVector2(0,0);
	nr++;

	// TOP
	tbuff[numtriangles].p1 = nr;
	tbuff[numtriangles].p2 = nr+1;
	tbuff[numtriangles].p3 = nr+2;
	numtriangles++;
	tbuff[numtriangles].p1 = nr;
	tbuff[numtriangles].p2 = nr+2;
	tbuff[numtriangles].p3 = nr+3;
	numtriangles++;
	vbuff3[nr] = p[4];
	vbuff2[nr] = CVector2(0,0);
	nr++;
	vbuff3[nr] = p[7];
	vbuff2[nr] = CVector2(0,1);
	nr++;
	vbuff3[nr] = p[6];
	vbuff2[nr] = CVector2(1,1);
	nr++;
	vbuff3[nr] = p[5];
	vbuff2[nr] = CVector2(1,0);
	nr++;

	// LEFT
	tbuff[numtriangles].p1 = nr;
	tbuff[numtriangles].p2 = nr+1;
	tbuff[numtriangles].p3 = nr+2;
	numtriangles++;
	tbuff[numtriangles].p1 = nr;
	tbuff[numtriangles].p2 = nr+2;
	tbuff[numtriangles].p3 = nr+3;
	numtriangles++;
	vbuff3[nr] = p[0];
	vbuff2[nr] = CVector2(1,0);
	nr++;
	vbuff3[nr] = p[1];
	vbuff2[nr] = CVector2(0,0);
	nr++;
	vbuff3[nr] = p[7];
	vbuff2[nr] = CVector2(0,1);
	nr++;
	vbuff3[nr] = p[4];
	vbuff2[nr] = CVector2(1,1);
	nr++;

	// FRONT
	tbuff[numtriangles].p1 = nr;
	tbuff[numtriangles].p2 = nr+1;
	tbuff[numtriangles].p3 = nr+2;
	numtriangles++;
	tbuff[numtriangles].p1 = nr;
	tbuff[numtriangles].p2 = nr+2;
	tbuff[numtriangles].p3 = nr+3;
	numtriangles++;
	vbuff3[nr] = p[0];
	vbuff2[nr] = CVector2(0,0);
	nr++;
	vbuff3[nr] = p[4];
	vbuff2[nr] = CVector2(0,1);
	nr++;
	vbuff3[nr] = p[5];
	vbuff2[nr] = CVector2(1,1);
	nr++;
	vbuff3[nr] = p[3];
	vbuff2[nr] = CVector2(1,0);
	nr++;

	// BACK
	tbuff[numtriangles].p1 = nr;
	tbuff[numtriangles].p2 = nr+1;
	tbuff[numtriangles].p3 = nr+2;
	numtriangles++;
	tbuff[numtriangles].p1 = nr;
	tbuff[numtriangles].p2 = nr+2;
	tbuff[numtriangles].p3 = nr+3;
	numtriangles++;
	vbuff3[nr] = p[7];
	vbuff2[nr] = CVector2(1,1);
	nr++;
	vbuff3[nr] = p[1];
	vbuff2[nr] = CVector2(1,0);
	nr++;
	vbuff3[nr] = p[2];
	vbuff2[nr] = CVector2(0,0);
	nr++;
	vbuff3[nr] = p[6];
	vbuff2[nr] = CVector2(0,1);
	nr++;

	numpoints = nr;
}


BOX::BOX() {
	type = ST_BOX;

	numpoints = 0;
	numtriangles = 0;

	vbuff3 = NULL;
	vbuff2 = NULL;
	tbuff = NULL;
	initialized = false;

	p=new CVector3[NUM_CONTROLS[type]];
}



/******************************NPLANE CLASS*********************************************/



void NPLANE::GenerateShape() {
	if (!initialized) {
		vbuff3 = new CVector3[(smoothness+1)*(smoothness+1)];
		vbuff2 = new CVector2[(smoothness+1)*(smoothness+1)];
		tbuff = new Triangle[2*smoothness*smoothness];
		initialized = true;
	}

	numpoints = numtriangles = 0;

	m_nurb.Init(p, smoothness);
	m_nurb.GenerateShape();

	append_ptr( (byte *)vbuff3, (byte *)m_nurb.GetVertexList(), numpoints, m_nurb.GetNumPoints(), sizeof(CVector3));
	numpoints = append_ptr( (byte *)vbuff2, (byte *)m_nurb.GetTexCoordList(), numpoints, m_nurb.GetNumPoints(), sizeof(CVector2));
	numtriangles = append_ptr( (byte *)tbuff, (byte *)m_nurb.GetTriangleList(),	numtriangles, m_nurb.GetNumTriangles(), sizeof(Triangle));

	m_nurb.FreeMemory();
}

void NPLANE::Init(CVector3 s, CVector3 e, int sm, int type) {

	smoothness = sm<3 ? 3:sm;

	if (p==NULL) p=new CVector3[NUM_CONTROLS[type]];

	vbuff3 = new CVector3[(smoothness+1)*(smoothness+1)];
	vbuff2 = new CVector2[(smoothness+1)*(smoothness+1)];
	tbuff = new Triangle[2*smoothness*smoothness];
	initialized = true;

	CVector3 v[8];

	v[0] = CVector3(s.x, s.y, e.z);
	v[1] = s;
	v[2] = CVector3(e.x, s.y, s.z);
	v[3] = CVector3(e.x, s.y, e.z);
	v[4] = CVector3(s.x, e.y, e.z);
	v[5] = e;
	v[6] = CVector3(e.x, e.y, s.z);
	v[7] = CVector3(s.x, e.y, s.z);

	switch(type) {
		case _X:			
			p[0] = (v[0]+v[3])/2;
			p[1] = (v[0]+v[2])/2;
			p[2] = (v[1]+v[2])/2;
			p[3] = (v[3]+v[4])/2;
			p[4] = (v[2]+v[4])/2;
			p[5] = (v[2]+v[7])/2;
			p[6] = (v[4]+v[5])/2;
			p[7] = (v[4]+v[6])/2;
			p[8] = (v[7]+v[6])/2;			
			break;
		case _Y:
			p[0] = (v[0]+v[4])/2;
			p[1] = (v[0]+v[5])/2;
			p[2] = (v[3]+v[5])/2;
			p[3] = (v[0]+v[7])/2;
			p[4] = (v[0]+v[6])/2;
			p[5] = (v[3]+v[6])/2;
			p[6] = (v[1]+v[7])/2;
			p[7] = (v[1]+v[6])/2;
			p[8] = (v[2]+v[6])/2;
			break;
		case _Z:
			p[0] = (v[0]+v[1])/2;
			p[1] = (v[0]+v[2])/2;
			p[2] = (v[3]+v[2])/2;
			p[3] = (v[0]+v[7])/2;
			p[4] = (v[0]+v[6])/2;
			p[5] = (v[3]+v[6])/2;
			p[6] = (v[4]+v[7])/2;
			p[7] = (v[4]+v[6])/2;
			p[8] = (v[5]+v[6])/2;
			break;
	}
}

NPLANE::NPLANE() {
	type = ST_NPLANE;

	numpoints = 0;
	numtriangles = 0;

	vbuff3 = NULL;
	vbuff2 = NULL;
	tbuff = NULL;
	initialized = false;

	p=new CVector3[NUM_CONTROLS[type]];
}



/******************************NCONE CLASS*********************************************/


NCONE::NCONE() {
	type = ST_NCONE;

	numpoints = 0;
	numtriangles = 0;

	vbuff3 = NULL;
	vbuff2 = NULL;
	tbuff = NULL;
	initialized = false;

	p=new CVector3[NUM_CONTROLS[type]];
}


void NCONE::Init(CVector3 s, CVector3 e, int sm) {
	smoothness = sm<3 ? 3:sm;

	if (p==NULL) p=new CVector3[NUM_CONTROLS[type]];
	vbuff3 = new CVector3[4*(smoothness+1)*(smoothness+1)];
	vbuff2 = new CVector2[4*(smoothness+1)*(smoothness+1)];
	tbuff  = new Triangle[4*(2*smoothness*smoothness)];
	initialized = true;

	CVector3 v[8];
	v[0] = s;
	v[1] = CVector3(e.x, s.y, s.z);
	v[2] = CVector3(e.x, s.y, e.z);
	v[3] = CVector3(s.x, s.y, e.z);
	v[4] = CVector3(s.x, e.y, e.z);
	v[5] = e;
	v[6] = CVector3(e.x, e.y, s.z);
	v[7] = CVector3(s.x, e.y, s.z);

	p[0] = (v[0] + v[3])/2;
	p[1] =  v[0];
	p[2] = (v[0] + v[1])/2;
	p[3] =  v[1];
	p[4] = (v[1] + v[2])/2;
	p[5] =  v[2];
	p[6] = (v[2] + v[3])/2;
	p[7] =  v[3];
	p[8] = (v[0] + v[4])/2;
	p[9] = (v[0] + v[7])/2;
	p[10]= (v[0] + v[6])/2;
	p[11]= (v[1] + v[6])/2;
	p[12]= (v[1] + v[5])/2;
	p[13]= (v[2] + v[5])/2;
	p[14]= (v[2] + v[4])/2;
	p[15]= (v[3] + v[4])/2;
	p[16]= (v[4]+v[6])/2;
}


void NCONE::GenerateShape() {
	if (!initialized) {
		vbuff3 = new CVector3[4*(smoothness+1)*(smoothness+1)];
		vbuff2 = new CVector2[4*(smoothness+1)*(smoothness+1)];
		tbuff  = new Triangle[4*(2*smoothness*smoothness)];
		initialized = true;
	}

	CVector3 v[9];

	v[0] = p[0];
	v[1] = p[1];
	v[2] = p[2];
	v[3] = p[8];
	v[4] = p[9];
	v[5] = p[10];
	v[6] = p[16];
	v[7] = p[16];
	v[8] = p[16];
	slice[0].Init(v, smoothness);	
	v[0] = p[2];
	v[1] = p[3];
	v[2] = p[4];
	v[3] = p[10];
	v[4] = p[11];
	v[5] = p[12];
	v[6] = p[16];
	v[7] = p[16];
	v[8] = p[16];
	slice[1].Init(v, smoothness);
	v[0] = p[4];
	v[1] = p[5];
	v[2] = p[6];
	v[3] = p[12];
	v[4] = p[13];
	v[5] = p[14];
	v[6] = p[16];
	v[7] = p[16];
	v[8] = p[16];
	slice[2].Init(v, smoothness);
	v[0] = p[6];
	v[1] = p[7];
	v[2] = p[0];
	v[3] = p[14];
	v[4] = p[15];
	v[5] = p[8];
	v[6] = p[16];
	v[7] = p[16];
	v[8] = p[16];	
	slice[3].Init(v, smoothness);

	numpoints = numtriangles = 0;

	CVector2 *ptr2;
	CVector3 *ptr3;
	Triangle *ptrt;
		
	for (int i=0; i<4; i++) {
		slice[i].GenerateShape();
		
		ptr2 = slice[i].GetTexCoordList();
		ptr3 = slice[i].GetVertexList();
		ptrt = slice[i].GetTriangleList();

		for (int jj=0; jj<slice[i].GetNumTriangles(); jj++) {
			tbuff[numtriangles].p1 = ptrt[jj].p1 + numpoints;
			tbuff[numtriangles].p2 = ptrt[jj].p2 + numpoints;
			tbuff[numtriangles].p3 = ptrt[jj].p3 + numpoints;
			numtriangles++;
		}

		for (jj=0; jj<slice[i].GetNumPoints(); jj++) {
			ptr2[jj].x = i*0.25f + ptr2[jj].x * 0.25f;
			vbuff2[numpoints] = ptr2[jj];
			vbuff3[numpoints] = ptr3[jj];
			numpoints++;
		}	
	}
}


/******************************NCILINDER CLASS*********************************************/


NCILINDER::NCILINDER() {
	type = ST_NCILINDER;

	numpoints = 0;
	numtriangles = 0;

	vbuff3 = NULL;
	vbuff2 = NULL;
	tbuff = NULL;
	initialized = false;

	p=new CVector3[NUM_CONTROLS[type]];
}


void NCILINDER::Init(CVector3 s, CVector3 e, int sm) {
	smoothness = sm<3 ? 3:sm;

	if (p==NULL) p=new CVector3[NUM_CONTROLS[type]];
	vbuff3 = new CVector3[4*(smoothness+1)*(smoothness+1)];
	vbuff2 = new CVector2[4*(smoothness+1)*(smoothness+1)];
	tbuff  = new Triangle[4*(2*smoothness*smoothness)];
	initialized = true;

	CVector3 v[8];
	v[0] = s;
	v[1] = CVector3(e.x, s.y, s.z);
	v[2] = CVector3(e.x, s.y, e.z);
	v[3] = CVector3(s.x, s.y, e.z);
	v[4] = CVector3(s.x, e.y, e.z);
	v[5] = e;
	v[6] = CVector3(e.x, e.y, s.z);
	v[7] = CVector3(s.x, e.y, s.z);

	p[0] = (v[0] + v[3])/2;
	p[1] =  v[0];
	p[2] = (v[0] + v[1])/2;
	p[3] =  v[1];
	p[4] = (v[1] + v[2])/2;
	p[5] =  v[2];
	p[6] = (v[2] + v[3])/2;
	p[7] =  v[3];
	p[8] = (v[0] + v[4])/2;
	p[9] = (v[0] + v[7])/2;
	p[10]= (v[0] + v[6])/2;
	p[11]= (v[1] + v[6])/2;
	p[12]= (v[1] + v[5])/2;
	p[13]= (v[2] + v[5])/2;
	p[14]= (v[2] + v[4])/2;
	p[15]= (v[3] + v[4])/2;
	p[16]= (v[4] + v[7])/2;
	p[17]= v[7];
	p[18]= (v[7] + v[6])/2;
	p[19]= v[6];
	p[20]= (v[6] + v[5])/2;
	p[21]= v[5];
	p[22]= (v[5] + v[4])/2;
	p[23]= v[4];

}


void NCILINDER::GenerateShape() {
	if (!initialized) {
		vbuff3 = new CVector3[4*(smoothness+1)*(smoothness+1)];
		vbuff2 = new CVector2[4*(smoothness+1)*(smoothness+1)];
		tbuff  = new Triangle[4*(2*smoothness*smoothness)];
		initialized = true;
	}

	CVector3 v[9];

	v[0] = p[0];
	v[1] = p[1];
	v[2] = p[2];
	v[3] = p[8];
	v[4] = p[9];
	v[5] = p[10];
	v[6] = p[16];
	v[7] = p[17];
	v[8] = p[18];
	slice[0].Init(v, smoothness);	
	v[0] = p[2];
	v[1] = p[3];
	v[2] = p[4];
	v[3] = p[10];
	v[4] = p[11];
	v[5] = p[12];
	v[6] = p[18];
	v[7] = p[19];
	v[8] = p[20];
	slice[1].Init(v, smoothness);
	v[0] = p[4];
	v[1] = p[5];
	v[2] = p[6];
	v[3] = p[12];
	v[4] = p[13];
	v[5] = p[14];
	v[6] = p[20];
	v[7] = p[21];
	v[8] = p[22];
	slice[2].Init(v, smoothness);
	v[0] = p[6];
	v[1] = p[7];
	v[2] = p[0];
	v[3] = p[14];
	v[4] = p[15];
	v[5] = p[8];
	v[6] = p[22];
	v[7] = p[23];
	v[8] = p[16];	
	slice[3].Init(v, smoothness);

	numpoints = numtriangles = 0;

	CVector2 *ptr2;
	CVector3 *ptr3;
	Triangle *ptrt;
		
	for (int i=0; i<4; i++) {
		slice[i].GenerateShape();
		
		ptr2 = slice[i].GetTexCoordList();
		ptr3 = slice[i].GetVertexList();
		ptrt = slice[i].GetTriangleList();

		for (int jj=0; jj<slice[i].GetNumTriangles(); jj++) {
			tbuff[numtriangles].p1 = ptrt[jj].p1 + numpoints;
			tbuff[numtriangles].p2 = ptrt[jj].p2 + numpoints;
			tbuff[numtriangles].p3 = ptrt[jj].p3 + numpoints;
			numtriangles++;
		}

		for (jj=0; jj<slice[i].GetNumPoints(); jj++) {
			ptr2[jj].x = 1 - i*0.25f - ptr2[jj].x * 0.25f;
			ptr2[jj].x*= 2.0f;		// because it is 2 times more wide
			vbuff2[numpoints] = ptr2[jj];
			vbuff3[numpoints] = ptr3[jj];
			numpoints++;
		}
	}
}


/******************************ENDCAP CLASS*********************************************/


ENDCAP::ENDCAP() {
	type = ST_ENDCAP;

	numpoints = 0;
	numtriangles = 0;

	vbuff3 = NULL;
	vbuff2 = NULL;
	tbuff = NULL;
	initialized = false;

	p=new CVector3[NUM_CONTROLS[type]];
}


void ENDCAP::Init(CVector3 s, CVector3 e, int sm) {
	smoothness = sm<3 ? 3:sm;

	if (p==NULL) p=new CVector3[NUM_CONTROLS[type]];
	vbuff3 = new CVector3[2*(smoothness+1)*(smoothness+1)];
	vbuff2 = new CVector2[2*(smoothness+1)*(smoothness+1)];
	tbuff  = new Triangle[2*(2*smoothness*smoothness)];
	initialized = true;

	CVector3 v[8];
	v[0] = s;
	v[1] = CVector3(e.x, s.y, s.z);
	v[2] = CVector3(e.x, s.y, e.z);
	v[3] = CVector3(s.x, s.y, e.z);
	v[4] = CVector3(s.x, e.y, e.z);
	v[5] = e;
	v[6] = CVector3(e.x, e.y, s.z);
	v[7] = CVector3(s.x, e.y, s.z);

	p[0] = (v[0] + v[3])/2;
	p[1] =  v[0];
	p[2] = (v[0] + v[1])/2;
	p[3] =  v[1];
	p[4] = (v[1] + v[2])/2;
	p[5] = (v[0] + v[4])/2;
	p[6] = (v[0] + v[7])/2;
	p[7]= (v[0] + v[6])/2;
	p[8]= (v[1] + v[6])/2;
	p[9]= (v[1] + v[5])/2;
	p[10]= (v[4] + v[7])/2;
	p[11]= v[7];
	p[12]= (v[7] + v[6])/2;
	p[13]= v[6];
	p[14]= (v[6] + v[5])/2;
}


void ENDCAP::GenerateShape() {
	if (!initialized) {
		vbuff3 = new CVector3[2*(smoothness+1)*(smoothness+1)];
		vbuff2 = new CVector2[2*(smoothness+1)*(smoothness+1)];
		tbuff  = new Triangle[2*(2*smoothness*smoothness)];
		initialized = true;
	}

	CVector3 v[9];

	v[0] = p[0];
	v[1] = p[1];
	v[2] = p[2];
	v[3] = p[5];
	v[4] = p[6];
	v[5] = p[7];
	v[6] = p[10];
	v[7] = p[11];
	v[8] = p[12];
	slice[0].Init(v, smoothness);	
	v[0] = p[2];
	v[1] = p[3];
	v[2] = p[4];
	v[3] = p[7];
	v[4] = p[8];
	v[5] = p[9];
	v[6] = p[12];
	v[7] = p[13];
	v[8] = p[14];
	slice[1].Init(v, smoothness);
	
	numpoints = numtriangles = 0;

	CVector2 *ptr2;
	CVector3 *ptr3;
	Triangle *ptrt;
		
	for (int i=0; i<2; i++) {
		slice[i].GenerateShape();
		
		ptr2 = slice[i].GetTexCoordList();
		ptr3 = slice[i].GetVertexList();
		ptrt = slice[i].GetTriangleList();

		for (int jj=0; jj<slice[i].GetNumTriangles(); jj++) {
			tbuff[numtriangles].p1 = ptrt[jj].p1 + numpoints;
			tbuff[numtriangles].p2 = ptrt[jj].p2 + numpoints;
			tbuff[numtriangles].p3 = ptrt[jj].p3 + numpoints;
			numtriangles++;
		}

		for (jj=0; jj<slice[i].GetNumPoints(); jj++) {
			ptr2[jj].x = i*0.5f - ptr2[jj].x * 0.5f;
			vbuff2[numpoints] = ptr2[jj];
			vbuff3[numpoints] = ptr3[jj];
			numpoints++;
		}	
	}
}


/******************************BEVEL CLASS*********************************************/


void BEVEL::GenerateShape() {
	if (!initialized) {
		vbuff3 = new CVector3[(smoothness+1)*(smoothness+1)];
		vbuff2 = new CVector2[(smoothness+1)*(smoothness+1)];
		tbuff = new Triangle[2*smoothness*smoothness];
		initialized = true;
	}

	numpoints = numtriangles = 0;

	m_nurb.Init(p, smoothness);
	m_nurb.GenerateShape();

	append_ptr( (byte *)vbuff3, (byte *)m_nurb.GetVertexList(), numpoints, m_nurb.GetNumPoints(), sizeof(CVector3));
	numpoints = append_ptr( (byte *)vbuff2, (byte *)m_nurb.GetTexCoordList(), numpoints, m_nurb.GetNumPoints(), sizeof(CVector2));
	numtriangles = append_ptr( (byte *)tbuff, (byte *)m_nurb.GetTriangleList(),	numtriangles, m_nurb.GetNumTriangles(), sizeof(Triangle));
}

void BEVEL::Init(CVector3 s, CVector3 e, int sm) {

	smoothness = sm<3 ? 3:sm;

	if (p==NULL) p=new CVector3[NUM_CONTROLS[type]];

	vbuff3 = new CVector3[(smoothness+1)*(smoothness+1)];
	vbuff2 = new CVector2[(smoothness+1)*(smoothness+1)];
	tbuff = new Triangle[2*smoothness*smoothness];
	initialized = true;

	CVector3 v[8];

	v[0] = CVector3(s.x, s.y, e.z);
	v[1] = s;
	v[2] = CVector3(e.x, s.y, s.z);
	v[3] = CVector3(e.x, s.y, e.z);
	v[4] = CVector3(s.x, e.y, e.z);
	v[5] = e;
	v[6] = CVector3(e.x, e.y, s.z);
	v[7] = CVector3(s.x, e.y, s.z);
	
	p[0] = v[0];
	p[1] = v[1];
	p[2] = v[2];
	p[3] = (v[0]+v[4])/2;
	p[4] = (v[1]+v[7])/2;
	p[5] = (v[2]+v[6])/2;
	p[6] = v[4];
	p[7] = v[7];
	p[8] = v[6];
}

BEVEL::BEVEL() {
	type = ST_BEVEL;

	numpoints = 0;
	numtriangles = 0;

	vbuff3 = NULL;
	vbuff2 = NULL;
	tbuff = NULL;
	initialized = false;

	p=new CVector3[NUM_CONTROLS[type]];
}


/******************************BEZIERMESH CLASS*********************************************/


BEZIERMESH::BEZIERMESH() {
	type = ST_BEZIERMESH;

	numpoints = 0;
	numtriangles = 0;

	vbuff3 = NULL;
	vbuff2 = NULL;
	tbuff = NULL;
	initialized = false;

	p=new CVector3[NUM_CONTROLS[type]];
}

CVector3 BEZIERMESH::Bezier3(CVector3 p1, CVector3 p2, CVector3 p3, float t) {
	float  c = (1-t);

	CVector3 pointOnCurve;

	pointOnCurve.x = (p1.x*c*c) + (p2.x*2*t*c) + (p3.x*t*t);
	pointOnCurve.y = (p1.y*c*c) + (p2.y*2*t*c) + (p3.y*t*t);
	pointOnCurve.z = (p1.z*c*c) + (p2.z*2*t*c) + (p3.z*t*t);

	return pointOnCurve;
}

CVector2 BEZIERMESH::Bezier2(CVector2 p1, CVector2 p2, CVector2 p3, float t) {
	float  c = (1-t);

	CVector2 pointOnCurve;

	pointOnCurve.x = (p1.x*c*c) + (p2.x*2*t*c) + (p3.x*t*t);
	pointOnCurve.y = (p1.y*c*c) + (p2.y*2*t*c) + (p3.y*t*t);

	return pointOnCurve;
}


void BEZIERMESH::Init(CVector3 *point, int sm) {
	if (point==NULL) return;

	smoothness = sm<3 ? 3:sm;

	if (p==NULL) p=new CVector3[NUM_CONTROLS[type]];

	vbuff3 = new CVector3[1+smoothness+1];
	vbuff2 = new CVector2[1+smoothness+1];
	tbuff  = new Triangle[smoothness];
	initialized = true;

	memcpy(p, point, 4*sizeof(CVector3));
}

void BEZIERMESH::GenerateShape(bool inverted) {
	if (!initialized) {
		vbuff3 = new CVector3[1+smoothness+1];
		vbuff2 = new CVector2[1+smoothness+1];
		tbuff  = new Triangle[smoothness];
		initialized = true;
	}

	numtriangles = numpoints = 0;

	float		t;

	for (int i=0; i<=smoothness; i++) {
		t = (float)i/(float)smoothness;
		vbuff3[numpoints] = Bezier3(p[0], p[1], p[2], t);
		if (inverted) vbuff2[numpoints] = Bezier2(CVector2(0,0), CVector2(0,1), CVector2(1,1), t);
		else vbuff2[numpoints] = Bezier2(CVector2(0,0), CVector2(1,0), CVector2(1,1), t);
		numpoints++;
	}

	vbuff3[numpoints] = p[3];
	vbuff2[numpoints] = CVector2(0,1);
	numpoints++;

	for (i=0; i<smoothness; i++) {
		tbuff[numtriangles].p1 = i;
		tbuff[numtriangles].p2 = i+1;
		tbuff[numtriangles].p3 = numpoints-1;
		numtriangles++;
	}
}


/******************************BCAP CLASS*********************************************/


BCAP::BCAP() {
	type = ST_BCAP;

	numpoints = 0;
	numtriangles = 0;

	vbuff3 = NULL;
	vbuff2 = NULL;
	tbuff = NULL;
	initialized = false;

	p=new CVector3[NUM_CONTROLS[type]];
}

void BCAP::Init(SHAPE nurb, bool lowhigh) {
	// avoids only from illegal memory reference
	if (nurb.GetType()!=ST_BEVEL) return;

	CVector3 *tmp = nurb.GetControlPointList();
	smoothness = nurb.GetSmoothness();

	if (p==NULL) p=new CVector3[NUM_CONTROLS[type]];

	vbuff3 = new CVector3[1+smoothness+1];
	vbuff2 = new CVector2[1+smoothness+1];
	tbuff  = new Triangle[smoothness];
	initialized = true;


	if (lowhigh) {
		p[0] = tmp[0];
		p[1] = tmp[1];
		p[2] = tmp[2];
		p[3] = tmp[1];
	} else {
		p[0] = tmp[6];
		p[1] = tmp[7];
		p[2] = tmp[8];
		p[3] = tmp[7];
	}
}

void BCAP::GenerateShape() {
	if (!initialized) {
		vbuff3 = new CVector3[1+smoothness+1];
		vbuff2 = new CVector2[1+smoothness+1];
		tbuff  = new Triangle[smoothness];
		initialized = true;
	}

	m_bm.Init(p, smoothness);
	m_bm.GenerateShape(true);

	numpoints = numtriangles = 0;

	append_ptr((byte *)vbuff3, (byte *)m_bm.GetVertexList(), numpoints, m_bm.GetNumPoints(), sizeof(CVector3));
	numpoints=append_ptr((byte *)vbuff2, (byte *)m_bm.GetTexCoordList(), numpoints, m_bm.GetNumPoints(), sizeof(CVector2));
	numtriangles=append_ptr((byte *)tbuff, (byte *)m_bm.GetTriangleList(), numtriangles, m_bm.GetNumTriangles(), sizeof(Triangle));
}

/******************************ECAP CLASS*********************************************/

ECAP::ECAP() {
	type = ST_ECAP;

	numpoints = 0;
	numtriangles = 0;

	vbuff3 = NULL;
	vbuff2 = NULL;
	tbuff = NULL;
	initialized = false;

	p=new CVector3[NUM_CONTROLS[type]];
}

void ECAP::Init(SHAPE endcap, bool lowhigh) {
	// avoids only from illegal memory reference
	if (endcap.GetType()!=ST_ENDCAP) return;

	smoothness = endcap.GetSmoothness();
	if (p==NULL) p=new CVector3[NUM_CONTROLS[type]];

	vbuff3 = new CVector3[2*(1+smoothness+1)];
	vbuff2 = new CVector2[2*(1+smoothness+1)];
	tbuff  = new Triangle[2*smoothness];
	initialized = true;

	CVector3 *tmp = endcap.GetControlPointList();

	if (lowhigh) {
		p[0] = tmp[0];
		p[1] = tmp[1];
		p[2] = tmp[2];
		p[3] = tmp[3];
		p[4] = tmp[4];
	} else {
		p[0] = tmp[10];
		p[1] = tmp[11];
		p[2] = tmp[12];
		p[3] = tmp[13];
		p[4] = tmp[14];
	}
}

void ECAP::GenerateShape() {
	if (!initialized) {
		vbuff3 = new CVector3[2*(1+smoothness+1)];
		vbuff2 = new CVector2[2*(1+smoothness+1)];
		tbuff  = new Triangle[2*smoothness];
		initialized = true;
	}

	numtriangles = numpoints = 0;

	CVector3 v[4];

	v[0] = p[0];
	v[1] = p[1];
	v[2] = p[2];
	v[3] = p[1];
	
	m_bm[0].Init(v, smoothness);
	m_bm[0].GenerateShape(true);

	append_ptr((byte *)vbuff3, (byte *)m_bm[0].GetVertexList(), numpoints, m_bm[0].GetNumPoints(), sizeof(CVector3));
	numpoints=append_ptr((byte *)vbuff2, (byte *)m_bm[0].GetTexCoordList(), numpoints, m_bm[0].GetNumPoints(), sizeof(CVector2));
	numtriangles=append_ptr((byte *)tbuff, (byte *)m_bm[0].GetTriangleList(), numtriangles, m_bm[0].GetNumTriangles(), sizeof(Triangle));

	v[0] = p[4];
	v[1] = p[3];
	v[2] = p[2];
	v[3] = p[3];
	
	m_bm[0].Init(v, smoothness);
	m_bm[0].GenerateShape(true);

	Triangle *tbp = m_bm[0].GetTriangleList();

	for (int i=0; i<m_bm[0].GetNumTriangles(); i++) {
		tbuff[numtriangles].p1 = tbp[i].p1 + numpoints;
		tbuff[numtriangles].p2 = tbp[i].p2 + numpoints;
		tbuff[numtriangles].p3 = tbp[i].p3 + numpoints;
		numtriangles++;
	}

	append_ptr((byte *)vbuff3, (byte *)m_bm[0].GetVertexList(), numpoints, m_bm[0].GetNumPoints(), sizeof(CVector3));
	numpoints=append_ptr((byte *)vbuff2, (byte *)m_bm[0].GetTexCoordList(), numpoints, m_bm[0].GetNumPoints(), sizeof(CVector2));	
}


/************************************************IECAP CLASS*********************************************/

IECAP::IECAP() {
	type = ST_IECAP;

	numpoints = 0;
	numtriangles = 0;

	vbuff3 = NULL;
	vbuff2 = NULL;
	tbuff = NULL;
	initialized = false;

	p=new CVector3[NUM_CONTROLS[type]];
}

void IECAP::Init(SHAPE endcap, bool lowhigh) {
	// avoids only from illegal memory reference
	if (endcap.GetType()!=ST_ENDCAP) return;

	smoothness = endcap.GetSmoothness();
	if (p==NULL) p=new CVector3[NUM_CONTROLS[type]];

	vbuff3 = new CVector3[2*(1+smoothness+1)];
	vbuff2 = new CVector2[2*(1+smoothness+1)];
	tbuff  = new Triangle[2*smoothness];
	initialized = true;

	CVector3 *tmp = endcap.GetControlPointList();

	if (lowhigh) {
		p[0] = tmp[0];
		p[1] = tmp[1];
		p[2] = tmp[2];
		p[3] = tmp[3];
		p[4] = tmp[4];
	} else {
		p[0] = tmp[10];
		p[1] = tmp[11];
		p[2] = tmp[12];
		p[3] = tmp[13];
		p[4] = tmp[14];
	}
}

void IECAP::GenerateShape() {
	if (!initialized) {
		vbuff3 = new CVector3[2*(1+smoothness+1)];
		vbuff2 = new CVector2[2*(1+smoothness+1)];
		tbuff  = new Triangle[2*smoothness];
		initialized = true;
	}

	numtriangles = numpoints = 0;

	CVector3 pp;
	CVector3 v[4];

	v[0] = p[0];
	v[1] = p[1];
	v[2] = p[2];
	v[3] = (p[0]+p[4])/2;
	
	m_bm[0].Init(v, smoothness);
	m_bm[0].GenerateShape(false);

	append_ptr((byte *)vbuff3, (byte *)m_bm[0].GetVertexList(), numpoints, m_bm[0].GetNumPoints(), sizeof(CVector3));
	numpoints=append_ptr((byte *)vbuff2, (byte *)m_bm[0].GetTexCoordList(), numpoints, m_bm[0].GetNumPoints(), sizeof(CVector2));
	numtriangles=append_ptr((byte *)tbuff, (byte *)m_bm[0].GetTriangleList(), numtriangles, m_bm[0].GetNumTriangles(), sizeof(Triangle));

	v[0] = p[4];
	v[1] = p[3];
	v[2] = p[2];
	v[3] = (p[0]+p[4])/2;
	
	m_bm[0].Init(v, smoothness);
	m_bm[0].GenerateShape(false);

	Triangle *tbp = m_bm[0].GetTriangleList();

	for (int i=0; i<m_bm[0].GetNumTriangles(); i++) {
		tbuff[numtriangles].p1 = tbp[i].p1 + numpoints;
		tbuff[numtriangles].p2 = tbp[i].p2 + numpoints;
		tbuff[numtriangles].p3 = tbp[i].p3 + numpoints;
		numtriangles++;
	}

	append_ptr((byte *)vbuff3, (byte *)m_bm[0].GetVertexList(), numpoints, m_bm[0].GetNumPoints(), sizeof(CVector3));
	numpoints=append_ptr((byte *)vbuff2, (byte *)m_bm[0].GetTexCoordList(), numpoints, m_bm[0].GetNumPoints(), sizeof(CVector2));	
}

/******************************ICAP CLASS*********************************************/


IBCAP::IBCAP() {
	type = ST_IBCAP;

	numpoints = 0;
	numtriangles = 0;

	vbuff3 = NULL;
	vbuff2 = NULL;
	tbuff = NULL;
	initialized = false;

	p=new CVector3[NUM_CONTROLS[type]];
}

void IBCAP::Init(SHAPE nurb, bool lowhigh) {
	// avoids only from illegal memory reference
	if (nurb.GetType()!=ST_BEVEL) return;

	CVector3 *tmp = nurb.GetControlPointList();
	smoothness = nurb.GetSmoothness();

	if (p==NULL) p=new CVector3[NUM_CONTROLS[type]];

	vbuff3 = new CVector3[1+smoothness+1];
	vbuff2 = new CVector2[1+smoothness+1];
	tbuff  = new Triangle[smoothness];
	initialized = true;


	if (lowhigh) {
		p[0] = tmp[0];
		p[1] = tmp[1];
		p[2] = tmp[2];		
	} else {
		p[0] = tmp[6];
		p[1] = tmp[7];
		p[2] = tmp[8];
	}

	CVector3 pp = (p[0] + p[2])/2;
	p[3] = p[1] + Normalize(pp-p[1]) * Magnitude(pp-p[1])*2;
}

void IBCAP::GenerateShape() {
	if (!initialized) {
		vbuff3 = new CVector3[1+smoothness+1];
		vbuff2 = new CVector2[1+smoothness+1];
		tbuff  = new Triangle[smoothness];
		initialized = true;
	}

	m_bm.Init(p, smoothness);
	m_bm.GenerateShape(false);

	numpoints = numtriangles = 0;

	append_ptr((byte *)vbuff3, (byte *)m_bm.GetVertexList(), numpoints, m_bm.GetNumPoints(), sizeof(CVector3));
	numpoints=append_ptr((byte *)vbuff2, (byte *)m_bm.GetTexCoordList(), numpoints, m_bm.GetNumPoints(), sizeof(CVector2));
	numtriangles=append_ptr((byte *)tbuff, (byte *)m_bm.GetTriangleList(), numtriangles, m_bm.GetNumTriangles(), sizeof(Triangle));
}
