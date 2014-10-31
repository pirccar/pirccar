#include "autonomous.h"

Autonomous::Autonomous()
{
	autonomous = false;
	turn = false;
	turnAroundLeft = false;
	stop = false;
	recalculateManhattan = false;
	travelDistance = 0.0f;
	travelDistanceLimit = 0.0f;
	scanSteps = 60;
	scanIndex = 0;
	collisionPathDetected = false;
	scanning = false;
	collLeft = 0;
	collRight = 0;
	speed = 0;
	rotRad = 0;
	acceleration = 0;
	accelerationLimit = 7;
	lineStart = Vector();
	lineEnd = Vector();
}

void Autonomous::update()
{
	//TODO add a timer
	
	if(gotoTargets.size() == 0)
	{
		acceleration = 0;
		return;
	}
	else if(stop)
	{
		acceleration = 0;
		if(speed == 0)
		{
			stop = false;
			reverse = 1; //NOTE THIS IS NOT THE CORRECT VALUE #"!#"!!#
		}
	}
	else if(turn)
	{
		internalTurnAround();
	}
	else if(false) //TODO IMPLEMENT SCAN METHOD
	{
	
	}
	else
	{
		
		if(false)//TODO CHECK PASSIVE SENSORS
		{
			collisionPathDetected = true;
			travelDistanceLimit = 0;
			recalculateManhattan = true;
			return;
		}
		
		Vector dir = Vector((float)cos(rotRad), (float)sin(rotRad), 0);
		Vector tarDir = Subtract(gotoTargets[gotoTargets.size() - 1], position);
		float distance = Distance(position, gotoTargets[gotoTargets.size() - 1]);
		
		if(manhattanTargets.size() > 0 && distance > 100)
			tarDir = Subtract(manhattanTargets[manhattanTargets.size() -1], position);
			
		tarDir = Normalize(tarDir);
		
		float v1 = (float)atan2(tarDir.y, tarDir.x);
		float v2 = (float)atan2(dir.y, dir.x);
		float angleToTarget = v1 - v2;
		
		while(angleToTarget > PI_F || angleToTarget < -PI_F)
		{
			if(angleToTarget > PI_F)
			{
				float removeFactor = angleToTarget - (float)PI_F;
				angleToTarget = (float)-PI_F + removeFactor;
			}
			else if(angleToTarget < -PI_F)
			{
				float removeFactor = angleToTarget + (float)PI_F;
				angleToTarget = (float)PI_F + removeFactor;
			}
		}
		
		steering = (int)RadianToDegree(angleToTarget);
		steering = -(int)clamp(steering, -48, 48);
		
		if(manhattanTargets.size() > 0 && distance > 100)
			distance = Distance(position, manhattanTargets[manhattanTargets.size() - 1]);
			
		calculateSpeed(distance < travelDistanceLimit - travelDistance ? distance : travelDistanceLimit - travelDistance);
		if(distance <= 15.0f && manhattanTargets.size() <= 1)
		{
			gotoTargets.pop_back();
			travelDistanceLimit = 0;
		}
		else if(distance <= 30.0f && manhattanTargets.size() >= 1)
			manhattanTargets.pop_back();
	}
	
	updatePosition();
}

void Autonomous::updatePosition()
{
	//TODO NEED TIMER
	float x = (50 / fabs(atan(steering)) + 25);
	float r = (float)sqrt(x * x + 25 * 25);
	float theta = speed * reverse / r; //NEED TIMER AND CALCULATION OF REVERSE VALUE!!!
	
	if(steering > 0.0f)
		theta = -theta;
	
	rotRad += theta;
	if(rotRad > PI_F * 2)
		rotRad = rotRad - PI_F * 2;
	else if(rotRad < 0)
		rotRad = PI_F * 2 + rotRad;
	
	Vector addVector = Vector((float)cos(rotRad), (float)sin(rotRad), 0);
	position = Add(position, ScalarVecMul(speed * reverse, addVector)); //NEED TIMER + REVERSE FIX
	
	travelDistance += speed; // add timer
	
}

void Autonomous::toggleAutonomous()
{
	autonomous = !autonomous;
}

void Autonomous::addGotoTarget(Vector target)
{
	gotoTargets.push_back(target);
}

void Autonomous::manhattanCalculation()
{
	if(gotoTargets.size() > 0)
	{
		manhattanTargets.clear();
		Vector finalDestination = gotoTargets[0];
		Vector lastPos = position;
		Vector currentMPos = position;
		std::vector<ManhattanSquare> openList;
		std::vector<ManhattanSquare> closedList;
		ManhattanSquare first = ManhattanSquare();
		int index = -1;
		first.pos = currentMPos;
		int manhattanDistance = 100;
		closedList.push_back(first);
		bool done = false;
		
		while(!done)
		{
			if(rectangleContains(currentMPos.x - manhattanDistance / 2, currentMPos.y - manhattanDistance /2, manhattanDistance, manhattanDistance, finalDestination))
			{
				bool foundHome = false;
				recalculateManhattan = false;
				Vector parent = Vector();
				for(int i = 0; i < closedList.size(); i++)
				{
					if(closedList[i].pos == currentMPos)
					{
						manhattanTargets.push_back(closedList[i].pos);
						parent = closedList[i].parent;
						break;
					}
				}
				
				while(!foundHome)
				{
					for(int i = 0; i < closedList.size(); i++)
					{
						if(closedList[i].pos == parent)
						{
							if(rectangleContains(closedList[i].pos.x - manhattanDistance / 2, closedList[i].pos.y - manhattanDistance / 2, manhattanDistance, manhattanDistance, position))
							{
								foundHome = true;
								break;
							}
							
							manhattanTargets.push_back(closedList[i].pos);
							parent = closedList[i].parent;
							break;
						}
					}
					done = true;
					break;
				}
			}
			else
			{
				Vector* directions = new Vector[4];
				directions[0] = Vector(currentMPos.x, currentMPos.y - manhattanDistance, 0);
				directions[1] = Vector(currentMPos.x + manhattanDistance, currentMPos.y, 0);
				directions[2] = Vector(currentMPos.x, currentMPos.y + manhattanDistance, 0);
				directions[3] = Vector(currentMPos.x - manhattanDistance, currentMPos.y, 0);
				
				bool* colls = new bool[4];
				for(int i = 0; i < 4; i++)
				{
					colls[i] = false;
				}
				
				Vector manhattanDistanceVector = Vector(manhattanDistance / 2, manhattanDistance /2, 0);
				
				for(int i = 0; i < virtualWorldLines.size(); i++)
				{
					for(int j = 0; j < 4; j++)
					{
						if(lineIntersect(virtualWorldLines[i].start, virtualWorldLines[i].end, currentMPos, directions[j]))
						{
							colls[j] = true;
						}
					}
				}
				
				for(int i = 0; i < 4; i++)
				{
					if(!colls[i])
					{
						ManhattanSquare s;
						s.pos = directions[i];
						s.parent = currentMPos;
						Vector newDir = Subtract(s.pos, s.parent);
						Vector lastDir = Subtract(s.parent, lastPos);
						s.g = closedList[closedList.size() - 1].g + (newDir != lastDir ? manhattanDistance : 0);
						s.h = (int)(fabs(finalDestination.x - directions[i].x) + fabs(finalDestination.y - directions[i].y));
						s.f = s.g + s.h;
						bool inList = false;
						bool inClosed = false;
						
						for(int j = 0; j < closedList.size(); j++)
						{
							if(closedList[j].pos == s.pos)
							{
								inClosed = true;
								break;
							}
						}
						
						if(inClosed)
							continue;
							
						for(int j = 0; j < openList.size(); j++)
						{
							if(openList[j].pos == s.pos)
							{
								ManhattanSquare temp = openList[j];
								temp.g = s.g;
								temp.parent = s.parent;
								temp.f = temp.g + temp.h;
								if(temp.f < openList[j].f)
									openList[j] = temp;
								inList = true;
								break;
							}
						}
						
						for(int j = 0; j < virtualWorldLines.size(); j++)
						{
							Vector start = Vector(s.pos.x - manhattanDistanceVector.x, s.pos.y - manhattanDistanceVector.y, 0);
							Vector end = Vector(s.pos.x + manhattanDistanceVector.x, s.pos.y + manhattanDistanceVector.y, 0);
							if(lineIntersect(virtualWorldLines[j].start, virtualWorldLines[j].end, start, end))
							{
								colls[i] = true;
								inList = true;
								break;
							}
						}
						
						if(!inList)
							openList.push_back(s);
					}
				}
				
				int lowest = 999999;
				for(int i = 0; i < openList.size(); i++)
				{
					if(openList[i].f < lowest)
					{
						lowest = openList[i].f;
						index = i;
					}
				}
				lastPos = currentMPos;
				currentMPos = openList[index].pos;
				closedList.push_back(openList[index]);
				openList.erase(openList.begin() + index);
				
				delete[] directions;
				delete[] colls;
			}
		}
	}
}

void Autonomous::calculateSpeed(float distance)
{
	if(distance <= 0)
	{
		acceleration = 0;
		return;
	}
	float calcSpeed = speed;
	
	if(calcSpeed == 0)
		calcSpeed = 1;
	
	acceleration = (int)((distance * 4.35f) / calcSpeed);
	acceleration = (int)clamp(acceleration, 1, accelerationLimit);
}

void Autonomous::turnAround()
{
	turn = true;
	stop = true;
	travelDistance = 0;
	turnAroundLeft = !turnAroundLeft;
	collisionPathDetected = false;
}
void Autonomous::internalTurnAround()
{
	if(turn)
	{
		steering = turnAroundLeft ? -48 : 48;
		reverse = -1; //NOTE THIS IS NOT THE CORRECT VALUE!"#!"#!"#!"
		float dist = 200.0f;
		acceleration = 5;
		
		if(travelDistance >= dist || travelDistance >= travelDistanceLimit) //TODO add checkReverseSensors
		{
			stop = true;
			acceleration = 0;
			steering = 0;
			if(speed == 0)
				reverse = 1;
			turn = false;
			travelDistance = travelDistanceLimit;
		}
	}
}

bool Autonomous::rectangleContains(float x, float y, int width, int height, Vector target)
{
	if(target.x < x)
		return false;
	else if( target.x > x + width)
		return false;
	else if ( target.y < y)
		return false;
	else if(target.y > y + height)
		return false;
		
	return true;
}

bool Autonomous::lineIntersect(Vector p0Start, Vector p0End, Vector p1Start, Vector p1End)
{
	float s0x, s0y, s1x, s1y;
	s0x =p0End.x - p0Start.x;
	s0y = p0End.y - p0Start.y;
	s1x = p1End.x - p1Start.x;
	s1y = p1End.y - p1Start.y;
	
	float s, t;
	s = (-s0y * (p0Start.x - p1Start.x) + s0x * (p0Start.y - p1Start.y)) / (-s1x * s0y + s0x * s1y);
	t = ( s1x * (p0Start.y - p1Start.x) - s1y * (p0Start.x - p1Start.x)) / (-s1x * s0y + s0x * s1y);
	
	if(s >= 0 && s <= 1 && t >= 0 && t <= 1)
	{
		return true;
	}
	
	return false;
}

float Autonomous::clamp(float value, float min, float max)
{
	if(value < min)
		value = min;
	else if(value > max)
		value = max;
	
	return value;
}