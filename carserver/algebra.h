#ifndef _ALGEBRA_H_
#define _ALGEBRA_H_

//typedef struct { double x, y, z; } Vector;
//typedef struct { double x, y;} Vector2;

struct HomVector
{
	float x;
	float y;
	float z;
	float w;

	HomVector()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		w = 0.0f;
	}

	HomVector(float x, float y, float z, float w)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

};

//typedef struct { double x, y, z, w; } HomVector;

struct Vector
{
	float x;
	float y;
	float z;

	Vector()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

	Vector(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}
	bool operator==(const Vector &other) const;
	bool operator!=(const Vector &other) const;
};

struct Vector2
{
	float x;
	float y;

	Vector2()
	{
	}

	Vector2(float x, float y)
	{
		this->x = x;
		this->y = y;
	}
};

/* Column-major order are used for the matrices here to be comnpatible with OpenGL.
** The indices used to access elements in the matrices are shown below.
**  _                _
** |                  |
** |   0   4   8  12  |
** |                  |
** |   1   5   9  13  |
** |                  |
** |   2   6  10  14  |
** |                  |
** |   3   7  11  15  |
** |_                _|
*/
typedef struct matrix { float e[16]; } Matrix;
typedef struct matrix3 { float e[9]; } Matrix3;


const float  PI_F=3.14159265358979f;
Vector Add(Vector a, Vector b);
Vector Subtract(Vector a, Vector b);
Vector CrossProduct(Vector a, Vector b);
float DotProduct(Vector a, Vector b);
float Length(Vector a);
Vector Normalize(Vector a);
Vector ScalarVecMul(float t, Vector a);
HomVector MatVecMul(Matrix a, Vector b);
Vector Homogenize(HomVector a);
Matrix MatMatMul(Matrix a, Matrix b);
void PrintMatrix(char *name, Matrix m);
void PrintVector(char *name, Vector v);
void PrintHomVector(char *name, HomVector h);

Matrix Identity();
Matrix3 Identity3();
Matrix Translate(float x, float y, float z);
Matrix Translate(Vector vec);
Matrix RotationX(float x);
Matrix RotationY(float y);
Matrix RotationZ(float z);
Matrix Scale(float x, float y, float z);
Matrix Perspective(float fov, float aspect, float near, float far);
Matrix Orthogonal(float left, float right, float bottom, float top, float near, float far);
Matrix LookAt(Vector eye, Vector tar, Vector up);
Matrix Transpose(Matrix a);
Matrix3 Mat4ToMat3(Matrix mat);

Vector BinaryColorToGLSLColor(Vector vec);

float Distance(Vector a, Vector b);
float HomoLength(HomVector a);
HomVector HomoNormalize(HomVector a);
float PlaneDistance(HomVector a, Vector b);
float DegreeToRadian(float degree);
float RadianToDegree(float radian);
float AngleBetweenVectors(Vector a, Vector b);

float Determinant(const Matrix);
bool Inverse(Matrix& i, const Matrix m);

#endif

