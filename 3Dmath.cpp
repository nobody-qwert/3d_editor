#include "3Dmath.h"


//--------------------------------------------//
//- Returns the angle of a given vector(x,y) -//
//- the angle is returned in radians ---------//
//--------------------------------------------//

float angle(float x, float y) {
  float alfa;

  if (x==0) alfa = y<0 ? -M_PI/2:M_PI/2;
  else if (y == 0) alfa = x<0 ? M_PI:0;
  else {
    alfa = atan(y/x);

    if (x<0 && y>0) {
       alfa += M_PI;
    } else if (x<0 && y<0) {
       alfa += M_PI;
    }
  }

  if (alfa < 0) alfa += 2*M_PI;

  return alfa;
}


float Distance(CVector2 vPoint1, CVector2 vPoint2)
{
	double distance = sqrt( (vPoint2.x - vPoint1.x) * (vPoint2.x - vPoint1.x) +
						    (vPoint2.y - vPoint1.y) * (vPoint2.y - vPoint1.y));
	return (float)distance;
}


// angle must be in degrees
void rot(float alfa, float &x, float &y) {
	alfa *= piover180;

	float fi = angle(x, y) + alfa;
	float length = sqrt(x*x + y*y);

	x = length * cos(fi);
	y = length * sin(fi);
}


// returns the perpendicular vector from 2 given vectors, taking the cross product
CVector3 Cross(CVector3 v1, CVector3 v2) {	
	CVector3 normal;

	normal.x = v1.y*v2.z - v2.y*v1.z;
	normal.y = v2.x*v1.z - v1.x*v2.z;
	normal.z = v1.x*v2.y - v2.x*v1.y;

	return normal;
}


// returns the length of the vector
float Magnitude(CVector3 v) {
	return (float)sqrt(v.x * v.x + v.y *  v.y + v.z * v.z);
}


// returns the length of the vector
float Magnitude(CVector2 v) {
	return (float)sqrt(v.x * v.x + v.y *  v.y);
}


// normalizes the vector
CVector3 Normalize(CVector3 v) {
	return v / Magnitude(v);
}


CVector2 Normalize(CVector2 v) {
	return v / Magnitude(v);
}

						
float DotProduct(CVector3 vVector1, CVector3 vVector2) 
{
	return ( (vVector1.x * vVector2.x) + (vVector1.y * vVector2.y) + (vVector1.z * vVector2.z) );
}

double AngleBetweenVectors(CVector3 Vector1, CVector3 Vector2)
{							
	double dotProduct = DotProduct(Vector1, Vector2);				
	double vectorsMagnitude = Magnitude(Vector1) * Magnitude(Vector2) ;
	double angle = acos( dotProduct / vectorsMagnitude );
	if(_isnan(angle))
		return 0;
	return( angle );
}

CVector3 Normal(CVector3 v[3])
{
	return Normalize(Cross(v[2] - v[0], v[1] - v[0]));
}


bool InsidePolygon(CVector3 vIntersection, CVector3 Poly[], long verticeCount)
{
	const double MATCH_FACTOR = 0.99;
	double Angle = 0.0;
	CVector3 vA, vB;
	
	for (int i = 0; i < verticeCount; i++)
	{	
		vA = Poly[i] - vIntersection;
		vB = Poly[(i + 1) % verticeCount] - vIntersection;
		Angle += AngleBetweenVectors(vA, vB);
	}
											
	if(Angle >= (MATCH_FACTOR * (2.0 * M_PI)) )
		return true;
	
	return false;
}


float Distance(CVector3 vPoint1, CVector3 vPoint2)
{
	double distance = sqrt( (vPoint2.x - vPoint1.x) * (vPoint2.x - vPoint1.x) +
						    (vPoint2.y - vPoint1.y) * (vPoint2.y - vPoint1.y) +
						    (vPoint2.z - vPoint1.z) * (vPoint2.z - vPoint1.z) );
	return (float)distance;
}

void swap(float &a, float &b) {
	float t = a;
	a = b;
	b = t;
}