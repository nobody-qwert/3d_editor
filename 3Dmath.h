#ifndef __3DMATH_H
#define __3DMATH_H


#include "float.h"
#include "math.h"


#define		M_PI			3.1415926535897932384626433832795f
#define		piover180		0.0174532925199432957692369076848861f

#define		NEAR_ZERO		0.0001f


#define MAX(a,b)	a>b ?	a:b
#define MIN(a,b)	a<b ?	a:b

#define SGN(a)		a<0 ?  -1:1
#define DIR(a)		a<0 ?	0:1


typedef		unsigned int	UINT;


class CVector3 {

public:

	float x, y, z;

	CVector3() 
	{	x = y = z = 0.0f;	}

	CVector3(float x, float y, float z)
	{	this->x = x;	this->y = y;	this->z = z;	}
	
	CVector3 operator +(CVector3 v) 
	{	return CVector3(x + v.x, y + v.y, z + v.z);		}

	CVector3 operator -(CVector3 v)
	{	return CVector3(x - v.x, y - v.y, z - v.z);		}

	CVector3 operator *(float num)
	{	return CVector3(x * num, y * num, z * num);		}

	CVector3 operator /(float num)
	{	return CVector3(x / num, y / num, z / num);		}

	void operator +=(CVector3 v) 
	{	x += v.x; y += v.y; z += v.z;		}

	void operator -=(CVector3 v)
	{	x -= v.x; y -= v.y; z -= v.z;		}
};


class CVector2 {

public:

	float x, y;

	CVector2() 
	{	x = y = 0.0f;	}

	CVector2(float x, float y)
	{	this->x = x;	this->y = y;	}
	
	CVector2 operator +(CVector2 v) 
	{	return CVector2(x + v.x, y + v.y);		}

	CVector2 operator -(CVector2 v)
	{	return CVector2(x - v.x, y - v.y);		}

	CVector2 operator *(float num)
	{	return CVector2(x * num, y * num);		}

	CVector2 operator /(float num)
	{	return CVector2(x / num, y / num);		}
	
};


typedef struct Triangle {
	int			p1; 
	int			p2;
	int			p3;
	CVector2	lightmap[3];
	UINT	    l_ID;
} TRIANGLE;


// basic functions


float		angle(float x, float y);
void        rot(float angle, float &x, float &y);

// Functions for vector manipulation

CVector3	Cross(CVector3 v1, CVector3 v2);
CVector3	Normalize(CVector3 v);
CVector2	Normalize(CVector2 v);
CVector3	Normal(CVector3 v[3]);
double		AngleBetweenVectors(CVector3 Vector1, CVector3 Vector2);
float		Magnitude(CVector3 v);
float		Magnitude(CVector2 v);
float		Distance(CVector2 vPoint1, CVector2 vPoint2);
float		Distance(CVector3 vPoint1, CVector3 vPoint2);
float		DotProduct(CVector3 vVector1, CVector3 vVector2);
bool		InsidePolygon(CVector3 vIntersection, CVector3 Poly[], long verticeCount);
void		swap(float &a, float &b);

#endif