#define _USE_MATH_DEFINES // To get M_PI defined
#include <math.h>
#include <stdio.h>
#include "algebra.h"

bool Vector::operator==(const Vector &other) const
{
	if(this->x == other.x && this->y == other.y && this->z == other.z)
		return true;
	return false;
}

bool Vector::operator!=(const Vector &other) const
{
	if(this->x != other.x && this->y != other.y && this->z != other.z)
		return true;
	return false;
}

Vector CrossProduct(Vector a, Vector b) {
	Vector v( a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
	return v;
}

float DotProduct(Vector a, Vector b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vector Subtract(Vector a, Vector b) {
	Vector v ( a.x-b.x, a.y-b.y, a.z-b.z );
	return v;
}    

Vector Add(Vector a, Vector b) {
	Vector v ( a.x+b.x, a.y+b.y, a.z+b.z );
	return v;
}    

float Length(Vector a) {
	return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
}

Vector Normalize(Vector a) {
	float len = Length(a);
	Vector v ( a.x/len, a.y/len, a.z/len );

	float len2 = Length(v);
	return v;
}

Vector ScalarVecMul(float t, Vector a) {
	Vector b ( t*a.x, t*a.y, t*a.z );
	return b;
}

HomVector MatVecMul(Matrix a, Vector b) {
	HomVector h;
	h.x = b.x*a.e[0] + b.y*a.e[4] + b.z*a.e[8] + a.e[12];
	h.y = b.x*a.e[1] + b.y*a.e[5] + b.z*a.e[9] + a.e[13];
	h.z = b.x*a.e[2] + b.y*a.e[6] + b.z*a.e[10] + a.e[14];
	h.w = b.x*a.e[3] + b.y*a.e[7] + b.z*a.e[11] + a.e[15];
	return h;
}

Vector Homogenize(HomVector h) {
	Vector a;
	if (h.w == 0.0) {
		fprintf(stderr, "Homogenize: w = 0\n");
		a.x = a.y = a.z = 9999999;
		return a;
	}
	a.x = h.x / h.w;
	a.y = h.y / h.w;
	a.z = h.z / h.w;
	return a;
}

Matrix MatMatMul(Matrix a, Matrix b) {
	Matrix c;
	int i, j, k;
	for (i = 0; i < 4; i++) 
	{
		for (j = 0; j < 4; j++) 
		{
			c.e[j*4+i] = 0.0;
			for (k = 0; k < 4; k++)
				c.e[j*4+i] += a.e[k*4+i] * b.e[j*4+k];
		}
	}
	return c;
}

void PrintVector(char *name, Vector a) {
	printf("%s: %6.5lf %6.5lf %6.5lf\n", name, a.x, a.y, a.z);
}

void PrintHomVector(char *name, HomVector a) {
	printf("%s: %6.5lf %6.5lf %6.5lf %6.5lf\n", name, a.x, a.y, a.z, a.w);
}

void PrintMatrix(char *name, Matrix a) { 
	int i,j;

	printf("%s:\n", name);
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			printf("%6.5lf ", a.e[j*4+i]);
		}
		printf("\n");
	}
}

//sorry for the long functions ;)

Matrix Identity()
{
	Matrix m;

	m.e[0] = 1;
	m.e[1] = 0;
	m.e[2] = 0;
	m.e[3] = 0;

	m.e[4] = 0;
	m.e[5] = 1;
	m.e[6] = 0;
	m.e[7] = 0;

	m.e[8] = 0;
	m.e[9] = 0;
	m.e[10] = 1;
	m.e[11] = 0;

	m.e[12] = 0;
	m.e[13] = 0;
	m.e[14] = 0;
	m.e[15] = 1;

	return m;
}

Matrix3 Identity3()
{
	Matrix3 m;

	m.e[0] = 1;
	m.e[1] = 0;
	m.e[2] = 0;

	m.e[3] = 0;
	m.e[4] = 1;
	m.e[5] = 0;

	m.e[6] = 0;
	m.e[7] = 0;
	m.e[8] = 1;

	return m;
}

Matrix Translate(float x, float y, float z)
{
	Matrix m;

	m.e[0] = 1;
	m.e[1] = 0;
	m.e[2] = 0;
	m.e[3] = 0;

	m.e[4] = 0;
	m.e[5] = 1;
	m.e[6] = 0;
	m.e[7] = 0;

	m.e[8] = 0;
	m.e[9] = 0;
	m.e[10] = 1;
	m.e[11] = 0;

	m.e[12] = x;
	m.e[13] = y;
	m.e[14] = z;
	m.e[15] = 1;

	return m;
}

Matrix Translate(Vector vec)
{
	Matrix m;

	m.e[0] = 1;
	m.e[1] = 0;
	m.e[2] = 0;
	m.e[3] = 0;

	m.e[4] = 0;
	m.e[5] = 1;
	m.e[6] = 0;
	m.e[7] = 0;

	m.e[8] = 0;
	m.e[9] = 0;
	m.e[10] = 1;
	m.e[11] = 0;

	m.e[12] = vec.x;
	m.e[13] = vec.y;
	m.e[14] = vec.z;
	m.e[15] = 1;

	return m;
}

Matrix RotationX(float x)
{
	Matrix m;

	m.e[0] = 1;
	m.e[1] = 0;
	m.e[2] = 0;
	m.e[3] = 0;

	m.e[4] = 0;
	m.e[5] = cos(x);
	m.e[6] = sin(x);
	m.e[7] = 0;

	m.e[8] = 0;
	m.e[9] = -sin(x);
	m.e[10] = cos(x);
	m.e[11] = 0;

	m.e[12] = 0;
	m.e[13] = 0;
	m.e[14] = 0;
	m.e[15] = 1;

	return m;
}

Matrix RotationY(float y)
{
	Matrix m;

	m.e[0] = cos(y);
	m.e[1] = 0;
	m.e[2] = -sin(y);
	m.e[3] = 0;

	m.e[4] = 0;
	m.e[5] = 1;
	m.e[6] = 0;
	m.e[7] = 0;

	m.e[8] = sin(y);
	m.e[9] = 0;
	m.e[10] = cos(y);
	m.e[11] = 0;

	m.e[12] = 0;
	m.e[13] = 0;
	m.e[14] = 0;
	m.e[15] = 1;

	return m;
}

Matrix RotationZ(float z)
{
	Matrix m;

	m.e[0] = cos(z);
	m.e[1] = sin(z);
	m.e[2] = 0;
	m.e[3] = 0;

	m.e[4] = -sin(z);
	m.e[5] = cos(z);
	m.e[6] = 0;
	m.e[7] = 0;

	m.e[8] = 0;
	m.e[9] = 0;
	m.e[10] = 1;
	m.e[11] = 0;

	m.e[12] = 0;
	m.e[13] = 0;
	m.e[14] = 0;
	m.e[15] = 1;

	return m;
}

Matrix Scale(float x, float y, float z)
{
	Matrix m;

	m.e[0] = x;
	m.e[1] = 0;
	m.e[2] = 0;
	m.e[3] = 0;

	m.e[4] = 0;
	m.e[5] = y;
	m.e[6] = 0;
	m.e[7] = 0;

	m.e[8] = 0;
	m.e[9] = 0;
	m.e[10] = z;
	m.e[11] = 0;

	m.e[12] = 0;
	m.e[13] = 0;
	m.e[14] = 0;
	m.e[15] = 1;

	return m;
}

Matrix Perspective(float fov, float aspect, float near, float far)
{
	Matrix m;
	
	m.e[0] = (1/tan((fov*M_PI/360)))/aspect;
	m.e[1] = 0;
	m.e[2] = 0;
	m.e[3] = 0;

	m.e[4] = 0;
	m.e[5] = 1/tan((fov*M_PI/360));
	m.e[6] = 0;
	m.e[7] = 0;

	m.e[8] = 0;
	m.e[9] = 0;
	m.e[10] = (far+near)/(near-far);
	m.e[11] = -1;

	m.e[12] = 0;
	m.e[13] = 0;
	m.e[14] = ((2*(far*near))/(near-far));
	m.e[15] = 0;

	return m;
}

Matrix Orthogonal(float left, float right, float bottom, float top, float near, float far)
{
	Matrix m;
	
	m.e[0] = 2/(right - left);
	m.e[1] = 0;
	m.e[2] = 0;
	m.e[3] = 0;

	m.e[4] = 0;
	m.e[5] = 2/(top-bottom);
	m.e[6] = 0;
	m.e[7] = 0;

	m.e[8] = 0;
	m.e[9] = 0;
	m.e[10] = -2/(far-near);
	m.e[11] = 0;

	m.e[12] = -(right+left)/(right-left);
	m.e[13] = -(top+bottom)/(top-bottom);
	m.e[14] = -(far+near)/(far-near);
	m.e[15] = 1;

	return m;
}

Matrix LookAt(Vector eye, Vector tar, Vector up)
{
	Matrix m1;

	/*Vector z = Normalize(Subtract(eye, tar));
	Vector x = Normalize(CrossProduct(up, z));
	Vector y = CrossProduct(z, x);*/

	/*m1.e[0] = x.x;
	m1.e[1] = x.y;
	m1.e[2] = x.z;
	m1.e[3] = -DotProduct(x, eye);

	m1.e[4] = y.x;
	m1.e[5] = y.y;
	m1.e[6] = y.z;
	m1.e[7] = -DotProduct(y, eye);

	m1.e[8] = z.x;
	m1.e[9] = z.y;
	m1.e[10] = z.z;
	m1.e[11] = -DotProduct(z, eye);

	m1.e[12] = 0;
	m1.e[13] = 0;
	m1.e[14] = 0;
	m1.e[15] = 1;*/

	Vector z = Normalize(Subtract(tar, eye));
	Vector x = Normalize(CrossProduct(z, up));
	Vector y = CrossProduct(x, z);

	m1.e[0] = x.x;
	m1.e[1] = y.x;
	m1.e[2] = -z.x;
	m1.e[3] = 0;

	m1.e[4] = x.y;
	m1.e[5] = y.y;
	m1.e[6] = -z.y;
	m1.e[7] = 0;

	m1.e[8] = x.z;
	m1.e[9] = y.z;
	m1.e[10] = -z.z;
	m1.e[11] = 0;

	m1.e[12] = -DotProduct(x, eye);
	m1.e[13] = -DotProduct(y, eye);
	m1.e[14] = DotProduct(z, eye);
	m1.e[15] = 1;

	return m1;
}

Matrix Transpose(Matrix a)
{
	Matrix m;

	m.e[0] = a.e[0];
	m.e[1] = a.e[4];
	m.e[2] = a.e[8];
	m.e[3] = a.e[12];

	m.e[4] = a.e[1];
	m.e[5] = a.e[5];
	m.e[6] = a.e[9];
	m.e[7] = a.e[13];

	m.e[8] = a.e[2];
	m.e[9] = a.e[6];
	m.e[10] = a.e[10];
	m.e[11] = a.e[14];

	m.e[12] = a.e[3];
	m.e[13] = a.e[7];
	m.e[14] = a.e[11];
	m.e[15] = a.e[15];

	return m;
}

Matrix3 Mat4ToMat3(Matrix mat)
{
	Matrix3 m;

	m.e[0] = mat.e[0];
	m.e[1] = mat.e[1];
	m.e[2] = mat.e[2];

	m.e[3] = mat.e[4];
	m.e[4] = mat.e[5];
	m.e[5] = mat.e[6];

	m.e[6] = mat.e[8];
	m.e[7] = mat.e[9];
	m.e[8] = mat.e[10];

	return m;
}

Vector BinaryColorToGLSLColor(Vector vec)
{
	return Vector(vec.x/255, vec.y / 255, vec.z / 255);
}

float Distance(Vector a, Vector b)
{
	return sqrt((b.x-a.x)*(b.x-a.x)+(b.y-a.y)*(b.y-a.y)+(b.z-a.z)*(b.z-a.z));
}

float Determinant(Matrix m)
{
	return
		m.e[12]*m.e[9]*m.e[6]*m.e[3]-
		m.e[8]*m.e[13]*m.e[6]*m.e[3]-
		m.e[12]*m.e[5]*m.e[10]*m.e[3]+
		m.e[4]*m.e[13]*m.e[10]*m.e[3]+
		m.e[8]*m.e[5]*m.e[14]*m.e[3]-
		m.e[4]*m.e[9]*m.e[14]*m.e[3]-
		m.e[12]*m.e[9]*m.e[2]*m.e[7]+
		m.e[8]*m.e[13]*m.e[2]*m.e[7]+
		m.e[12]*m.e[1]*m.e[10]*m.e[7]-
		m.e[0]*m.e[13]*m.e[10]*m.e[7]-
		m.e[8]*m.e[1]*m.e[14]*m.e[7]+
		m.e[0]*m.e[9]*m.e[14]*m.e[7]+
		m.e[12]*m.e[5]*m.e[2]*m.e[11]-
		m.e[4]*m.e[13]*m.e[2]*m.e[11]-
		m.e[12]*m.e[1]*m.e[6]*m.e[11]+
		m.e[0]*m.e[13]*m.e[6]*m.e[11]+
		m.e[4]*m.e[1]*m.e[14]*m.e[11]-
		m.e[0]*m.e[5]*m.e[14]*m.e[11]-
		m.e[8]*m.e[5]*m.e[2]*m.e[15]+
		m.e[4]*m.e[9]*m.e[2]*m.e[15]+
		m.e[8]*m.e[1]*m.e[6]*m.e[15]-
		m.e[0]*m.e[9]*m.e[6]*m.e[15]-
		m.e[4]*m.e[1]*m.e[10]*m.e[15]+
		m.e[0]*m.e[5]*m.e[10]*m.e[15];
}

bool Inverse(Matrix& i, const Matrix m)
{
	float x = Determinant(m);
	if (x==0) return false;

	i.e[0]= (-m.e[13]*m.e[10]*m.e[7] +m.e[9]*m.e[14]*m.e[7] +m.e[13]*m.e[6]*m.e[11]
	-m.e[5]*m.e[14]*m.e[11] -m.e[9]*m.e[6]*m.e[15] +m.e[5]*m.e[10]*m.e[15])/x;
	i.e[4]= ( m.e[12]*m.e[10]*m.e[7] -m.e[8]*m.e[14]*m.e[7] -m.e[12]*m.e[6]*m.e[11]
	+m.e[4]*m.e[14]*m.e[11] +m.e[8]*m.e[6]*m.e[15] -m.e[4]*m.e[10]*m.e[15])/x;
	i.e[8]= (-m.e[12]*m.e[9]* m.e[7] +m.e[8]*m.e[13]*m.e[7] +m.e[12]*m.e[5]*m.e[11]
	-m.e[4]*m.e[13]*m.e[11] -m.e[8]*m.e[5]*m.e[15] +m.e[4]*m.e[9]* m.e[15])/x;
	i.e[12]=( m.e[12]*m.e[9]* m.e[6] -m.e[8]*m.e[13]*m.e[6] -m.e[12]*m.e[5]*m.e[10]
	+m.e[4]*m.e[13]*m.e[10] +m.e[8]*m.e[5]*m.e[14] -m.e[4]*m.e[9]* m.e[14])/x;
	i.e[1]= ( m.e[13]*m.e[10]*m.e[3] -m.e[9]*m.e[14]*m.e[3] -m.e[13]*m.e[2]*m.e[11]
	+m.e[1]*m.e[14]*m.e[11] +m.e[9]*m.e[2]*m.e[15] -m.e[1]*m.e[10]*m.e[15])/x;
	i.e[5]= (-m.e[12]*m.e[10]*m.e[3] +m.e[8]*m.e[14]*m.e[3] +m.e[12]*m.e[2]*m.e[11]
	-m.e[0]*m.e[14]*m.e[11] -m.e[8]*m.e[2]*m.e[15] +m.e[0]*m.e[10]*m.e[15])/x;
	i.e[9]= ( m.e[12]*m.e[9]* m.e[3] -m.e[8]*m.e[13]*m.e[3] -m.e[12]*m.e[1]*m.e[11]
	+m.e[0]*m.e[13]*m.e[11] +m.e[8]*m.e[1]*m.e[15] -m.e[0]*m.e[9]* m.e[15])/x;
	i.e[13]=(-m.e[12]*m.e[9]* m.e[2] +m.e[8]*m.e[13]*m.e[2] +m.e[12]*m.e[1]*m.e[10]
	-m.e[0]*m.e[13]*m.e[10] -m.e[8]*m.e[1]*m.e[14] +m.e[0]*m.e[9]* m.e[14])/x;
	i.e[2]= (-m.e[13]*m.e[6]* m.e[3] +m.e[5]*m.e[14]*m.e[3] +m.e[13]*m.e[2]*m.e[7]
	-m.e[1]*m.e[14]*m.e[7] -m.e[5]*m.e[2]*m.e[15] +m.e[1]*m.e[6]* m.e[15])/x;
	i.e[6]= ( m.e[12]*m.e[6]* m.e[3] -m.e[4]*m.e[14]*m.e[3] -m.e[12]*m.e[2]*m.e[7]
	+m.e[0]*m.e[14]*m.e[7] +m.e[4]*m.e[2]*m.e[15] -m.e[0]*m.e[6]* m.e[15])/x;
	i.e[10]=(-m.e[12]*m.e[5]* m.e[3] +m.e[4]*m.e[13]*m.e[3] +m.e[12]*m.e[1]*m.e[7]
	-m.e[0]*m.e[13]*m.e[7] -m.e[4]*m.e[1]*m.e[15] +m.e[0]*m.e[5]* m.e[15])/x;
	i.e[14]=( m.e[12]*m.e[5]* m.e[2] -m.e[4]*m.e[13]*m.e[2] -m.e[12]*m.e[1]*m.e[6]
	+m.e[0]*m.e[13]*m.e[6] +m.e[4]*m.e[1]*m.e[14] -m.e[0]*m.e[5]* m.e[14])/x;
	i.e[3]= ( m.e[9]* m.e[6]* m.e[3] -m.e[5]*m.e[10]*m.e[3] -m.e[9]* m.e[2]*m.e[7]
	+m.e[1]*m.e[10]*m.e[7] +m.e[5]*m.e[2]*m.e[11] -m.e[1]*m.e[6]* m.e[11])/x;
	i.e[7]= (-m.e[8]* m.e[6]* m.e[3] +m.e[4]*m.e[10]*m.e[3] +m.e[8]* m.e[2]*m.e[7]
	-m.e[0]*m.e[10]*m.e[7] -m.e[4]*m.e[2]*m.e[11] +m.e[0]*m.e[6]* m.e[11])/x;
	i.e[11]=( m.e[8]* m.e[5]* m.e[3] -m.e[4]*m.e[9]* m.e[3] -m.e[8]* m.e[1]*m.e[7]
	+m.e[0]*m.e[9]* m.e[7] +m.e[4]*m.e[1]*m.e[11] -m.e[0]*m.e[5]* m.e[11])/x;
	i.e[15]=(-m.e[8]* m.e[5]* m.e[2] +m.e[4]*m.e[9]* m.e[2] +m.e[8]* m.e[1]*m.e[6]
	-m.e[0]*m.e[9]* m.e[6] -m.e[4]*m.e[1]*m.e[10] +m.e[0]*m.e[5]* m.e[10])/x;

	return true;
}

float HomoLength(HomVector a)
{
	return sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
}

HomVector HomoNormalize(HomVector a)
{
	float length = HomoLength(a);

	a.x /= length;
	a.y /= length;
	a.z /= length;
	a.w /= length;

	return a;
}

float PlaneDistance(HomVector a, Vector b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w;
}

float AngleBetweenVectors(Vector a, Vector b)
{
	float dot = DotProduct(a, b);

	float lengthA = Length(a);
	float lengthB = Length(b);

	float rad = dot / (lengthA * lengthB);

	rad = acos(rad);

	return rad * (180 / M_PI);
}

float DegreeToRadian(float degree)
{
	float PIOver180 = M_PI / 180;

	return degree * PIOver180;
}

float RadianToDegree(float radian)
{
	float factor = 180 / M_PI;
	
	return radian * factor;
}