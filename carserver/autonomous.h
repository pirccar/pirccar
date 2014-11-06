#pragma once

#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <vector>
#include <pthread.h>
#include <math.h>
#include "pca9685.h"
#include "algebra.h"

struct Line
{
	Vector start;
	Vector end;
};

struct ManhattanSquare
{
	int g;
	int h;
	int f;
	Vector pos;
	Vector parent;
};

class Autonomous
{
public:
	Autonomous();
	
	void update();
	void toggleAutonomous();
	void addGotoTarget(Vector target);
	
	
private:
	void updatePosition();
	void manhattanCalculation();
	void calculateSpeed(float distance);
	void turnAround();
	void internalTurnAround();
	bool rectangleContains(float x, float y, int width, int height, Vector target);
	bool lineIntersect(Vector p0Start, Vector p0End, Vector p1Start, Vector p1End);
	float clamp(float value, float min, float max);
	float getDT();
	
	Vector position;
	float speed;
	float rotRad;
	int acceleration;
	int accelerationLimit;
	int steering;
	int reverse;
	
	std::vector<Line> virtualWorldLines;
	std::vector<Vector> gotoTargets;
	std::vector<Vector> manhattanTargets;
	bool autonomous;
	bool turn;
	bool turnAroundLeft;
	bool stop;
	bool recalculateManhattan;
	float travelDistance;
	float travelDistanceLimit;
	int scanSteps;
	int scanIndex;
	bool collisionPathDetected;
	bool scanning;
	int collLeft;
	int collRight;
	Vector lineStart;
	Vector lineEnd;
	
	struct timeval previousTime;
};