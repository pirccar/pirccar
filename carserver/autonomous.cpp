#include "autonomous.h"

Autonomous::Autonomous()
{
	autonomous = false;
	turnAround = false;
	turnAroundLeft = false;
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
	lineStart = Vector();
	lineEnd = Vector();
}

void Autonomous::update()
{

}

void Autonomous::updatePosition()
{

}

void Autonomous::toggleAutonomous()
{
	autonomous = !autonomous;
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