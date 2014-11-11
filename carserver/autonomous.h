#pragma once

#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <vector>
#include <pthread.h>
#include <math.h>
#include <pthread.h>
#include "pca9685.h"
#include "algebra.h"
#include "speedMeter.h"

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
	~Autonomous();
	void update();
	void toggleAutonomous(bool on);
	void addGotoTarget(Vector target);
	void setChannels(int throttle, int steeringF, int steeringB, int gear);
	
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
	void setServos();
	
	static void* threadMain(void* ptr);
	void homeLoop();
	
	Vector position;
	float speed;
	float rotRad;
	int acceleration;
	int accelerationLimit;
	int steering;
	int reverse;
	SpeedMeter* speedMeter;
	
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
	
	int throttleChannel;
	int steeringFChannel;
	int steeringBChannel;
	int gearChannel;
	
	struct timeval previousTime;
	pthread_t thread;
};