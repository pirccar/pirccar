using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace CarSimulator
{
    struct ServoTiming
    {
        public Servo[] servo;
        public float distance;
    }

    struct Line
    {
        public Vector2 start;
        public Vector2 end;
    }

    struct ManhattanSquare
    {
        public int g;
        public int h;
        public int f;
        public Vector2 pos;
        public Vector2 parent;
    }

    class Car
    {
        Servo[] servos;
        Texture2D texture;
        int acceleration;
        Vector2 position;
        Vector2 direction;
        Quaternion rotation;
        float rotRad;
        float steering;
        float speed;
        int reverse;
        int accelerationLimit;
        float tireWidth;
        float tireRowSpacing;
        List<Vector2> lineList;
        float lineListDistance;

        DistanceSensor sensor;
        DistanceSensor[] passiveSensors;
        DistanceSensor[] reverseSensors;
        List<RealWorldObject> realObjects;
        List<Vector2> virtualWorldPoints;
        List<Line?> virtualWorldLines;
        List<Vector2> gotoTargets;
        List<Vector2> manhattanTargets;
        bool renderRealWorld;
        bool autonomous;
        bool turnAround;
        bool turnAroundLeft;
        bool recalculateManhattan;
        float travelDistance;
        float travelDistanceLimit;
        int scanSteps;
        int scanIndex;
        bool sensorLeft;
        bool collisionPathDetected;
        bool scanning;
        int collLeft;
        int collRight;
        Vector2? lineStart;
        Vector2? lineEnd;

        List<ServoTiming> goHomeData;
        int goHomeIndex;
        int goHomeTargetIndex;
        bool goHome;
        int prevGearValue;
        bool stop = false;

        int[] lastChannelIndex;

        //pid
        float errorSum, lastError;
        float kp = 0.006f;
        float ki = 0.002f;
        float kd = 0.006f;

        public Car(Vector2 position, Texture2D texture)
        {
            this.texture = texture;
            this.position = position;
            this.direction = new Vector2(1, 0);
            speed = 0.0f;
            steering = 0.0f;
            rotation = Quaternion.Identity;
            reverse = 1;
            accelerationLimit = 8;
            servos = new Servo[3];
            servos[0] = new Servo(0, 320, ServoType.steering);
            servos[1] = new Servo(1, 180, ServoType.throttle);
            servos[2] = new Servo(2, 0, ServoType.gear);
            lastChannelIndex = new int[3];
            tireWidth = 50.0f;
            tireRowSpacing = 50.0f;
            goHomeData = new List<ServoTiming>();
            lineList = new List<Vector2>();

            sensor = new DistanceSensor(position, rotRad, texture);
            passiveSensors = new DistanceSensor[4];
            reverseSensors = new DistanceSensor[2];
            virtualWorldPoints = new List<Vector2>();
            virtualWorldLines = new List<Line?>();
            renderRealWorld = true;
            autonomous = false;
            travelDistance = 0.0f;
            scanSteps = 60;
            scanIndex = 0;
            sensorLeft = true;
            recalculateManhattan = true;
            collisionPathDetected = false;
            gotoTargets = new List<Vector2>();
            manhattanTargets = new List<Vector2>();
            turnAround = false;
            scanning = false;

            realObjects = new List<RealWorldObject>();
            //Tripple Room
            /*
            RealWorldObject realObject = new RealWorldObject(new Vector2(550, 150), 25, 425, texture);
            realObjects.Add(realObject);
            realObject = new RealWorldObject(new Vector2(0, 0), 1600, 25, texture);
            realObjects.Add(realObject);
            realObject = new RealWorldObject(new Vector2(0, 0), 25, 1000, texture);
            realObjects.Add(realObject);
            realObject = new RealWorldObject(new Vector2(0, 550), 700, 25, texture);
            realObjects.Add(realObject);
            realObject = new RealWorldObject(new Vector2(1000, 0), 25, 550, texture);
            realObjects.Add(realObject);
            realObject = new RealWorldObject(new Vector2(900, 550), 550, 25, texture);
            realObjects.Add(realObject);
            realObject = new RealWorldObject(new Vector2(0, 975), 1600, 25, texture);
            realObjects.Add(realObject);
            */

            // office
            
            RealWorldObject realObject = new RealWorldObject(new Vector2(0, 600), 500, 25, texture);
            realObjects.Add(realObject);
            realObject = new RealWorldObject(new Vector2(0, 300), 500, 25, texture);
            realObjects.Add(realObject);
            realObject = new RealWorldObject(new Vector2(750, 600), 500, 25, texture);
            realObjects.Add(realObject);
            realObject = new RealWorldObject(new Vector2(750, 300), 500, 25, texture);
            realObjects.Add(realObject);

            realObject = new RealWorldObject(new Vector2(25, 0), 500, 25, texture);
            realObjects.Add(realObject);
            realObject = new RealWorldObject(new Vector2(750, 0), 500, 25, texture);
            realObjects.Add(realObject);
            realObject = new RealWorldObject(new Vector2(0, 0), 25, 900, texture);
            realObjects.Add(realObject);
            realObject = new RealWorldObject(new Vector2(0, 900), 1250, 25, texture);
            realObjects.Add(realObject);
            realObject = new RealWorldObject(new Vector2(1250, 0), 25, 925, texture);
            realObjects.Add(realObject);
            
            goHomeIndex = -1;
            goHome = false;

            for (int i = 0; i < passiveSensors.Length; i++)
            {
                passiveSensors[i] = new DistanceSensor(position, rotRad, texture);
            }
            passiveSensors[0].SetInternalRotation((float)-(Math.PI / 180) * 45);
            passiveSensors[1].SetInternalRotation((float)-(Math.PI / 180) * 22);
            passiveSensors[2].SetInternalRotation((float)(Math.PI / 180) * 22);
            passiveSensors[3].SetInternalRotation((float)(Math.PI / 180) * 45);

            for (int i = 0; i < reverseSensors.Length; i++)
            {
                reverseSensors[i] = new DistanceSensor(position, rotRad, texture);
            }

            reverseSensors[0].SetInternalRotation(-MathHelper.Pi + (MathHelper.Pi / 180) * 5);
            reverseSensors[1].SetInternalRotation(MathHelper.Pi - (MathHelper.Pi / 180) * 5);
        }

        public Vector2 GetPosition()
        {
            return position;
        } //get cars current position

        public float GetSpeed()
        {
            return speed;
        } //get cars current speed

        public int GetAcceleration()
        {
            return acceleration;
        }

        public float GetSteering()
        {
            return steering;
        } //get cars current wheel direction

        public float GetRotation()
        {
            return rotRad;
        } //get cars rotation

        public void Reset()
        {
            position = new Vector2(200, 300);
            speed = 0.0f;
            steering = 0.0f;
            rotation = Quaternion.Identity;
            rotRad = 0;
            servos = new Servo[3];
            servos[0] = new Servo(0, 320, ServoType.steering);
            servos[1] = new Servo(1, 180, ServoType.throttle);
            servos[2] = new Servo(2, 0, ServoType.gear);
            lastChannelIndex = new int[3];
            goHomeData = new List<ServoTiming>();
            goHomeIndex = -1;
            goHome = false;
            lineList.Clear();
            virtualWorldPoints.Clear();
            travelDistance = 0.0f;
            scanIndex = 0;
            sensorLeft = true;
            collisionPathDetected = false;
        } //reset the simulation

        public void SetServo(int channel, int value)
        {
            servos[channel].SetValue(value);
        } //set a servo to a value

        public void GoHome(int targetIndex = -1)
        {
            if (!goHome)
            {
                errorSum = 0;
                lastError = 0;
                goHomeTargetIndex = targetIndex;
            }
            goHome = true;
        } //tell the car to go home

        private void CalculateSpeed(float distance)
        {
            if (distance <= 0)
            {
                acceleration = 0;
                return;
            }
            float calcSpeed = speed;

            if (calcSpeed == 0)
                calcSpeed = 1;
            acceleration = (int)((distance * 4.35f) / calcSpeed);
            acceleration = (int)MathHelper.Clamp(acceleration, 1, accelerationLimit); 
            
        } //calculate a speed depending on distance to travel

        private void CalculateSpeedPid(float distance, GameTime gameTime)
        {
            float timeNow = (float)gameTime.ElapsedGameTime.TotalSeconds;

            errorSum += (distance * timeNow);
            float dErr = (distance - lastError) / timeNow;

            acceleration = (int)(kp * distance + ki * errorSum + kd * dErr);
            lastError = distance;
        } //speed calculation using pid controller

        private void InternalGoHome(GameTime gameTime)
        {
            if (goHomeIndex == goHomeTargetIndex) //this idicates that we have gone thru every state
            {
                acceleration = 0; //so put the acceleration to 0, this will in time stop the car
                if (speed == 0) //and if the car has stopped then we are done!
                {
                    goHome = false;
                    goHomeIndex = -1;
                    goHomeData.Clear();
                    lineList.Clear();
                }
                return;
            }

            ServoTiming temp = goHomeData[goHomeIndex]; //extract the current element from the list

            for (int i = goHomeIndex; i >= 0; i--) //find the correct state of the gear
            {
                if (goHomeData[i].servo[1] != null)
                {
                    reverse = goHomeData[i].servo[1].GetValue() > 0 ? 1 : -1;
                    break;
                }
            }

            if(reverse != prevGearValue) //if we have switched from forward to reverse then we need to stop, otherwise the wheel would spin
            {
                stop = true;
            }

            if(stop) //so we are stopping
            {
                if (speed == 0) //and if the speed is zero then we have stopped, time to move on
                    stop = false;
                acceleration = 0; //put acceleration to zero, this will stop the car
            }
            else if(temp.distance > 0) //if we are not stopping and we haven't reached our checkpoint keep driving
            {
                /*
                if (temp.distance < 50.0f) //speed calculation to not overdrive, will implement better calculations for this
                    acceleration = 1;
                else //else use a little more speed
                    acceleration = 5;*/
                CalculateSpeed(temp.distance);
                //CalculateSpeedPid(temp.distance, gameTime);
                temp.distance -= speed * (float)gameTime.ElapsedGameTime.TotalSeconds; //decrease the distance by current speed * time = distance this iteration
                goHomeData[goHomeIndex] = temp; //and put it back into our list for next iteration
            }
            else //we have reached a checkpoint!
            {
                goHomeIndex--; //decrease index so we can read next checkpoint value

                for (int i = goHomeIndex; i >= 0; i--) //find the next state of steering
                {
                    if (goHomeData[i].servo[0] != null)
                    {
                        steering = MathHelper.ToRadians(goHomeData[i].servo[0].GetValue() - 320);
                        break; //break to make sure we dont get the wrong state
                    }
                }
            }
            prevGearValue = reverse; //used to check for stopping
        } 

        private void saveStates(GameTime gameTime)
        {
            if(goHomeIndex == -1) //first state, save everything so we get a base value to work from
            {
                ServoTiming current = new ServoTiming();
                current.servo = new Servo[2];
                current.servo[0] = new Servo(servos[0]);
                current.servo[1] = new Servo(servos[2]);
                current.distance = 0;
                goHomeData.Add(current);
                goHomeIndex++;
                lastChannelIndex[0] = 0;
                lastChannelIndex[1] = 0;
                
            }
            else //Any other index than the first state
            {
                bool diff = false;
                ServoTiming current = new ServoTiming();
                current.servo = new Servo[2]; //number of servos
               
                //Check steering changed
                if (goHomeData[lastChannelIndex[0]].servo[0].GetValue() != servos[0].GetValue())
                {
                    diff = true;
                    current.servo[0] = new Servo(servos[0]); //copy the servo to our servoTiming struct
                    lastChannelIndex[0] = goHomeIndex +1; //save the last index of steering
                }
                //check gear changed
                if(goHomeData[lastChannelIndex[1]].servo[1].GetValue() != servos[2].GetValue())
                {
                    diff = true;
                    current.servo[1] = new Servo(servos[2]); //copy the servo to our servoTiming struct
                    lastChannelIndex[1] = goHomeIndex +1; //save the last index of gear
                }

                ServoTiming temp = goHomeData.ElementAt(goHomeIndex); //Extract previous state from list
                temp.distance += speed * (float)gameTime.ElapsedGameTime.TotalSeconds; //add some distance (if any)
                goHomeData[goHomeIndex] = temp; //put it back into our list

                if (diff) //if we found out that something has changed then add a new state
                {
                    current.distance = 0; //set distance for new state
                    goHomeData.Add(current);
                    goHomeIndex++;
                }
            }
        }

        private void ParseServos(GameTime gameTime)
        {
            acceleration = servos[1].GetValue() - 180;
            reverse = servos[2].GetValue() > 0 ? -1 : 1;
            steering = MathHelper.ToRadians(servos[0].GetValue() - 320);
        }

        public void ToggleRealWorld() //render objects or not
        {
            renderRealWorld = !renderRealWorld;
        }

        public void ToggleAutonomous()
        {
            autonomous = !autonomous;
        } //the car will drive by itself

        public void AddGotoTarget(Vector2 target)
        {
            gotoTargets.Add(target);
        } //add target to drive to

        private void AutonomousUpdate(GameTime gameTime)
        {
            if (gotoTargets.Count == 0) //indicates that there are not more targets to go to, car is done
            {
                acceleration = 0;
                return;
            }
            else if(stop)
            {
                acceleration = 0;
                if (speed == 0)
                {
                    stop = false;
                    reverse = 1;
                }
            }
            else if(turnAround)
            {
                InternalTurnAround();
            }
            else if (scanning || travelDistance >= travelDistanceLimit) //if the car is scanning the enviroment
            {
                acceleration = 0; //stop the car
                scanning = true;
                if (speed == 0.0f) //if the car has stopped we can start scanning
                {
                    if (scanIndex == 0) //at the first scan set some values
                    {
                        //virtualWorldLines.Clear();
                        lineStart = null;
                        lineEnd = null;
                        collLeft = 0;
                        collRight = 0;
                        
                    }
                    InternalScan(gameTime); //do one scan

                    if (scanIndex == 0 && travelDistance == 0) //last scan, check if the car will collide with anything etc
                    {
                        CheckCollision(rotRad, Vector2.Distance(position, gotoTargets[gotoTargets.Count - 1]));
                        scanning = false;
                        if (collisionPathDetected) //should indicate that the car will hit something
                            TurnAround();
                        if (Vector2.Distance(position, gotoTargets[gotoTargets.Count - 1]) > 100 && recalculateManhattan)
                            ManhattanCalculation(); //path finding.

                        //travelDistanceLimit = 300;
                    }
                }
            }
            else //if(gotoTargetIndex != -1) //the car is moving towards a target
            {
                if(CheckPassiveSensors())
                {
                    collisionPathDetected = true;
                    travelDistanceLimit = 0;
                    recalculateManhattan = true;
                    return;
                }

                Vector2 dir = new Vector2((float)Math.Cos(rotRad), (float)Math.Sin(rotRad)); //dirrection of car
                Vector2 tarDir = gotoTargets[gotoTargets.Count - 1] - position; //direction from position to current target
                float distance = Vector2.Distance(position, gotoTargets[gotoTargets.Count - 1]); //calculate distance to target

                if (manhattanTargets.Count > 0 && distance > 100) //use manhattan target instead
                    tarDir = manhattanTargets[manhattanTargets.Count -1] - position;
                
                tarDir.Normalize();

                float v1 = (float)Math.Atan2(tarDir.Y, tarDir.X);
                float v2 = (float)Math.Atan2(dir.Y, dir.X);
                float angleToTarget = v1 - v2; //the angle to the target in radians

                while(angleToTarget > Math.PI || angleToTarget < -Math.PI)
                {
                    if(angleToTarget > Math.PI)
                    {
                        float removeFactor = angleToTarget - (float)Math.PI; //calculations to ensuse proper angles from -180 to 180 degrees 
                        angleToTarget = (float)-Math.PI + removeFactor;
                    }
                    else if (angleToTarget < -Math.PI)
                    {
                        float removeFactor = angleToTarget + (float)Math.PI; //calculations to ensuse proper angles from -180 to 180 degrees 
                        angleToTarget = (float)Math.PI + removeFactor;
                    }
                }

                steering = (int)MathHelper.ToDegrees(angleToTarget); //set steering
                steering = -(int)MathHelper.Clamp(steering, -48, 48); //and clamp it, car cant turn at 180 degree angles

                if (manhattanTargets.Count > 0 && distance > 100)
                    distance = Vector2.Distance(position, manhattanTargets[manhattanTargets.Count - 1]);

                CalculateSpeed(distance < travelDistanceLimit - travelDistance ? distance : travelDistanceLimit - travelDistance); //calculate speed
                if(distance <= 15.0f && manhattanTargets.Count <= 1) //close enough to target, lets goto next target
                {
                    gotoTargets.RemoveAt(gotoTargets.Count - 1);
                    travelDistanceLimit = 0;
                }
                else if(distance <= 30.0f && manhattanTargets.Count >= 1)
                {
                    manhattanTargets.RemoveAt(manhattanTargets.Count - 1);
                }
            }
        }

        private void ManhattanCalculation() //calculate path to target
        {
            if (gotoTargets.Count > 0)
            {
                manhattanTargets.Clear();
                Vector2 finalDestination = gotoTargets[0]; //our final target destination
                Vector2 lastPos = position;
                Vector2 currentMPos = position;
                List<ManhattanSquare> openList = new List<ManhattanSquare>(); //open blocks
                List<ManhattanSquare> closedList = new List<ManhattanSquare>();//closed blocks
                ManhattanSquare first = new ManhattanSquare(); //current location
                int index = -1;
                first.pos = currentMPos;
                int manhattanDistance = 100;
                closedList.Add(first);
                bool done = false;

                while(!done) //done will be set when we have a found a manhattan square that covers the target
                {
                    //check if current square contains the target
                    Rectangle rect = new Rectangle((int)currentMPos.X - manhattanDistance / 2, (int)currentMPos.Y - manhattanDistance / 2, manhattanDistance, manhattanDistance);
                    if(rect.Contains(finalDestination))
                    {
                        //it did, now we just have to follow the parent road back to our current location
                        bool foundHome = false;
                        recalculateManhattan = false;
                        Vector2 parent = Vector2.Zero;
                        for (int i = 0; i < closedList.Count; i++) //find the square containing the target
                        {
                            if(closedList[i].pos == currentMPos)
                            {
                                manhattanTargets.Add(closedList[i].pos); //add this as a manhattan target
                                parent = closedList[i].parent; //grab target squares parent
                                break;
                            }
                        }

                        Vector2 lastDir = manhattanTargets[0] - parent;

                        while (!foundHome) //as long as we haven't found our start location
                        {
                            for (int i = 0; i < closedList.Count; i++) //check all possible location for parent
                            {
                                if(closedList[i].pos == parent) //we found the parent!
                                {
                                    //check if this square contains our current location
                                    Rectangle cRect = new Rectangle((int)closedList[i].pos.X - manhattanDistance / 2, (int)closedList[i].pos.Y - manhattanDistance/2, manhattanDistance, manhattanDistance);
                                    if (cRect.Contains(position))
                                    {
                                        foundHome = true; //it did, now to remove some redundant squares
                                        
                                        /*
                                        for (int j = manhattanTargets.Count -1; j >= 0; j -= 2) //this should be omptimized :)
                                        {
                                            manhattanTargets.RemoveAt(j);
                                        }*/
                                        
                                        break;
                                    }
                                    Vector2 dir = closedList[i].parent - closedList[i].pos;
                                    
                                    /*
                                    if (dir.X != lastDir.X && dir.Y != lastDir.Y) //if we have found a square that has a difference in x and y
                                    {
                                        manhattanTargets.Add(closedList[i].pos); //then add this square as a manhattan target
                                        lastDir = dir;
                                    }*/

                                    /*
                                    if (dir != lastDir) //if we have found a square that has a difference in x and y
                                    {
                                        manhattanTargets.Add(closedList[i].pos); //then add this square as a manhattan target
                                        lastDir = dir;
                                    }*/

                                    manhattanTargets.Add(closedList[i].pos); //then add this square as a manhattan target
                                    lastDir = dir;
                                    
                                    parent = closedList[i].parent;
                                    
                                    break;
                                }
                            }
                        }
                        done = true; //all done :)
                        break;
                    }
                    Vector2[] directions = new Vector2[4]; //calculate four possible directions, up, right, down, left from our current location
                    directions[0] = new Vector2(currentMPos.X, currentMPos.Y - manhattanDistance);
                    directions[1] = new Vector2(currentMPos.X + manhattanDistance, currentMPos.Y);
                    directions[2] = new Vector2(currentMPos.X, currentMPos.Y + manhattanDistance);
                    directions[3] = new Vector2(currentMPos.X - manhattanDistance, currentMPos.Y);

                    bool[] colls = new bool[4]; //bools to check if it touches an object
                    for (int i = 0; i < colls.Length; i++)
                    {
                        colls[i] = false;
                    }

                    Vector2 manhattanDistanceVector = new Vector2(manhattanDistance / 2, manhattanDistance / 2);
                    for (int i = 0; i < virtualWorldLines.Count; i++) //check collision against every know object
                    {
                        if (virtualWorldLines[i] == null)
                            continue;

                        for (int j = 0; j < 4; j++)
                        {
                            //if(lineIntersect(virtualWorldLines[i].Value.start, virtualWorldLines[i].Value.end, currentMPos, directions[j]) ||
                                //lineIntersect(virtualWorldLines[i].Value.start, virtualWorldLines[i].Value.end, currentMPos, directions[j] + manhattanDistanceVector))
                            if(lineIntersect(virtualWorldLines[i].Value.start, virtualWorldLines[i].Value.end, currentMPos, directions[j]))
                            {
                                colls[j] = true;
                            }
                        }
                    }

                    for (int i = 0; i < colls.Length; i++) //check if the calculated square doesn't collide with an object
                    {
                        if (!colls[i])
                        {
                            ManhattanSquare s = new ManhattanSquare(); //create a new manhattan square
                            s.pos = directions[i]; //set the position of the square
                            s.parent = currentMPos; //and set the parent
                            Vector2 newDir = s.pos - s.parent;
                            Vector2 lastDir = s.parent - lastPos;
                            s.g = closedList[closedList.Count - 1].g + (newDir != lastDir ? manhattanDistance : 0); //calculate g score
                            //s.g = closedList[closedList.Count - 1].g + manhattanDistance;
                            s.h = (int)(Math.Abs(finalDestination.X - directions[i].X) + Math.Abs(finalDestination.Y - directions[i].Y)); //calculate h score
                            s.f = s.g + s.h; //f score = g + h
                            bool inList = false; //check if this square is already known to us!
                            bool inClosed = false;

                            for (int j = 0; j < closedList.Count; j++)
                            {
                                if(closedList[j].pos == s.pos)
                                {
                                    inClosed = true;
                                    break;
                                }
                            }

                            if (inClosed)
                                continue;

                            for (int j = 0; j < openList.Count; j++)
                            {
                                if (openList[j].pos == s.pos) //it was :O
                                {
                                    ManhattanSquare temp = openList[j]; //well lets check if we found a faster way to this square
                                    temp.g = s.g;
                                    temp.parent = s.parent;
                                    temp.f = temp.g + temp.h;
                                    if (temp.f < openList[j].f) //if it was faster then change its score and parent
                                        openList[j] = temp;
                                    inList = true;
                                    break;
                                }
                            }

                            for (int j = 0; j < virtualWorldLines.Count; j++)
                            {
                                if (virtualWorldLines[j] == null)
                                    continue;
                                Vector2 start = new Vector2(s.pos.X - manhattanDistanceVector.X, s.pos.Y - manhattanDistanceVector.Y);
                                Vector2 end = new Vector2(s.pos.X + manhattanDistanceVector.X, s.pos.Y + manhattanDistanceVector.Y);
                                if (lineIntersect(virtualWorldLines[j].Value.start, virtualWorldLines[j].Value.end, start, end))
                                {
                                    colls[i] = true;
                                    inList = true;
                                    break;
                                }
                            }

                            if (!inList) //if the square wasn't known to us then lets add it
                                openList.Add(s);
                        }
                    }
                    int lowest = 99999; //time to choose the best way to go next, the square with the lowest f score!
                    for (int i = 0; i < openList.Count; i++)
                    {
                        if(openList[i].f < lowest)
                        {
                            lowest = openList[i].f; //save lowest for comparing
                            index = i; //and save the index so we know which one to use
                        }
                    }
                    lastPos = currentMPos;
                    currentMPos = openList[index].pos; //move the best location
                    closedList.Add(openList[index]);
                    openList.RemoveAt(index);
                }    
            }
               
        }

        private void InternalScan(GameTime gameTime)
        {
            if (sensorLeft) //move sensor to the left... or to the right
                sensor.SetRotationTarget((float)(Math.PI / scanSteps) * (float)scanIndex - MathHelper.PiOver2);
            else
                sensor.SetRotationTarget((float)(MathHelper.PiOver2 - (Math.PI / scanSteps) * (float)scanIndex));
            sensor.Update(gameTime); //update the sensor, it might move
            if(!sensor.Rotating()) //if the sensor isnt moving then we can measure the distance
            {
                float? sensorValue = SensorDected();

                if (sensorValue != null)
                {
                    if (scanIndex < scanSteps / 2) //check how many values are to the left and to the right of current heading
                    {
                        if (sensorLeft)
                            collLeft++;
                        else
                            collRight++;
                    }
                    else
                    {
                        if (sensorLeft)
                            collRight++;
                        else
                            collLeft++;
                    }
                }
                AddVirtualLine(sensorValue); //add a line using sensor readings
                scanIndex++;
                if (scanIndex >= scanSteps+1) //done scanning
                {
                    travelDistance = 0;
                    scanIndex = 0;
                    sensorLeft = !sensorLeft;
                }
            }
        }

        private bool CheckPassiveSensors()
        {
            bool returnValue = false;

            for (int i = 0; i < passiveSensors.Length; i++)
            {
                passiveSensors[i].SetPosition(position);
                passiveSensors[i].SetDirection(rotRad);
                for (int j = 0; j < realObjects.Count; j++)
                {
                    float? sensorValue = passiveSensors[i].Intersect(realObjects[j].GetRectangle());
                    if (sensorValue != null)
                    {
                        returnValue = sensorValue.Value < Math.Max(speed, 60);
                        if (returnValue)
                            return returnValue;
                        else
                            accelerationLimit = (int)(Math.Min(sensorValue.Value / 10.0f, 15));
                    }
                }
            }

            return returnValue;
        }

        private bool CheckReverseSensors()
        {
            bool retValue = false;

            for (int i = 0; i < reverseSensors.Length; i++)
            {
                reverseSensors[i].SetPosition(position);
                reverseSensors[i].SetDirection(rotRad);
                for (int j = 0; j < realObjects.Count; j++)
                {
                    float? sensorValue = reverseSensors[i].Intersect(realObjects[j].GetRectangle());
                    if (sensorValue != null)
                    {
                        retValue = sensorValue.Value < 50.0f;
                        if (retValue)
                            return retValue;
                    }
                }
            }

            return retValue;
        }

        private void CheckCollision(float rotation, float distance) //used to see if the car will collide with objects
        {
            Vector2 newDir = new Vector2((float)Math.Cos(rotation), (float)Math.Sin(rotation)); //calc new direction of car
            bool collision = false;
            for (int i = 0; i < virtualWorldLines.Count; i++) //check against every object
            {
                if (virtualWorldLines[i] == null)
                    continue;
                if (lineIntersect(virtualWorldLines[i].Value.start, virtualWorldLines[i].Value.end, position, position + newDir * distance))
                {
                    //if we collide with something along that path
                    collision = true;
                    break;
                }
            }
            if (collision) //if the car will collide with anything the car should try to avoid it!
            {
                travelDistanceLimit = 100.0f; //but only drive for X units, experimental value
            }
            else
            {
                //no collision at current path!
                travelDistanceLimit = 250.0f;
            }
        }

        private float? SensorDected()
        {
            float? returnValue = null;
            
            for (int i = 0; i < realObjects.Count; i++)
            {
                 float? sensorValue = sensor.Intersect(realObjects[i].GetRectangle());
                 if (returnValue == null || sensorValue < returnValue)
                     returnValue = sensorValue;
            }

            sensor.SetCollDistance(returnValue);

            return returnValue;
        }

        private void TurnAround()
        {
            turnAround = true;
            stop = true;
            travelDistance = 0;
            turnAroundLeft = !turnAroundLeft;
            collisionPathDetected = false;
        }

        private void InternalTurnAround()
        {
            if(turnAround)
            {
                steering = turnAroundLeft ? -48 : 48;
                reverse = -1;
                float dist = 200.0f;
                acceleration = 5;

                if (travelDistance >= dist || travelDistance > travelDistanceLimit || CheckReverseSensors())
                {
                    stop = true;
                    acceleration = 0;
                    steering = 0;
                    if(speed == 0)
                        reverse = 1;
                    turnAround = false;
                    travelDistance =  travelDistanceLimit;
                }

            }
        }

        private bool lineIntersect(Vector2 p0Start, Vector2 p0End, Vector2 p1Start, Vector2 p1End)
        {
            float s0x, s0y, s1x, s1y;
            s0x = p0End.X - p0Start.X;
            s0y = p0End.Y - p0Start.Y;
            s1x = p1End.X - p1Start.X;
            s1y = p1End.Y - p1Start.Y;

            float s, t;
            s = (-s0y * (p0Start.X - p1Start.X) + s0x * (p0Start.Y - p1Start.Y)) / (-s1x * s0y + s0x * s1y);
            t = ( s1x * (p0Start.Y - p1Start.Y) - s1y * (p0Start.X - p1Start.X)) / (-s1x * s0y + s0x * s1y);

            if(s >= 0 && s<= 1 && t >= 0 && t <= 1)
            {
                return true;
            }

            return false;
        }

        private void AddVirtualPoint(float sensorValue)
        {
            Vector2 collPoint = position;
            collPoint += sensor.GetDirection() * sensorValue;
            collPoint.X = (float)Math.Round(collPoint.X, 1);
            collPoint.Y = (float)Math.Round(collPoint.Y, 1);
            if (!virtualWorldPoints.Contains(collPoint))
                virtualWorldPoints.Add(collPoint);
        }

        private void AddVirtualLine(float? newValue)
        {
            if (newValue != null) //the sensor detected something
            {
                if (lineStart == null && lineEnd == null) //new line
                {
                    lineStart = position + sensor.GetDirection() * newValue.Value;
                }
                else if (lineStart != null) //indicates that we have a line start so either set new value to end
                {
                    Vector2 newPosition = position + sensor.GetDirection() * newValue.Value;
                    lineEnd = newPosition;
                    Line l = new Line();
                    l.start = lineStart.Value;
                    l.end = lineEnd.Value;
                    if(Vector2.Distance(l.start, l.end) < 50.0f)
                        virtualWorldLines.Add(l);
                    lineStart = lineEnd;
                    lineEnd = null;
                }
            }
            else
            {
                lineStart = null;
                lineEnd = null;
            }
        }

        private void UpdatePosition(GameTime gameTime)
        {
            float dt = (float)gameTime.ElapsedGameTime.TotalSeconds;
            float calculatedAcc = acceleration;
            calculatedAcc *= 5;
            if (calculatedAcc == 0) //Throttle released
            {
                if (speed < 0.5f || speed < 0) //stop car if speed = almost 0
                    speed = 0;
                else if (speed > 0) //motorbreak
                    calculatedAcc = -50;
            }
            else if (speed >= calculatedAcc)
            {
                calculatedAcc = -calculatedAcc;
            }

            speed = speed + calculatedAcc * (float)gameTime.ElapsedGameTime.TotalSeconds;

            float x = (tireRowSpacing / Math.Abs((float)Math.Atan(steering)) + tireWidth / 2);
            float r = (float)Math.Sqrt(x * x + (tireRowSpacing / 2) * (tireRowSpacing / 2));
            float theta = speed * reverse * (float)gameTime.ElapsedGameTime.TotalSeconds / r;

            if (steering > 0.0f)
                theta = -theta;

            rotation = rotation * Quaternion.CreateFromAxisAngle(new Vector3(0, 0, 1), theta);

            Vector2 right = new Vector2(1, 0);
            //Vector2 addVector = Vector2.Transform(right, rotation);
            Vector2 addVector;
           

            rotRad += theta;

            if (rotRad > Math.PI * 2)
                rotRad = rotRad - (float)Math.PI * 2;
            else if (rotRad < 0)
                rotRad = (float)Math.PI * 2 - rotRad;

            addVector.X = (float)Math.Cos(rotRad);
            addVector.Y = (float)Math.Sin(rotRad);

            position += addVector * speed * reverse * dt;

            if (autonomous)
                travelDistance += Math.Abs(speed * reverse * dt);

            lineListDistance += speed * dt;
            sensor.SetPosition(position);
            sensor.SetDirection(rotRad);
        }

        public void Update(GameTime gameTime)
        {
            if (!goHome && !autonomous)
            {
                saveStates(gameTime);
                ParseServos(gameTime);
                if(lineListDistance >= 5)
                {
                    lineListDistance -= 5;
                    lineList.Add(new Vector2(position.X, position.Y));
                }

                sensor.Update(gameTime);
                float? sensorValue = SensorDected();
                if (sensorValue != null)
                {
                    AddVirtualPoint(sensorValue.Value);
                }
            }
            else if(goHome)
            {
                InternalGoHome(gameTime);
            }
            else
            {
                saveStates(gameTime);
                AutonomousUpdate(gameTime);
                if (lineListDistance >= 5)
                {
                    lineListDistance -= 5;
                    lineList.Add(new Vector2(position.X, position.Y));
                }
            }
             
            UpdatePosition(gameTime);
            
        }

        private void DrawLine(SpriteBatch spriteBatch, Vector2 begin, Vector2 end, Color color)
        {
            Rectangle r = new Rectangle((int)begin.X, (int)begin.Y, (int)(end - begin).Length() + 1, 1);
            Vector2 v = Vector2.Normalize(begin - end);
            float angle = (float)Math.Acos(Vector2.Dot(v, -Vector2.UnitX));
            if (begin.Y > end.Y) 
                angle = MathHelper.TwoPi - angle;
            spriteBatch.Draw(texture, r, null, color, angle, Vector2.Zero, SpriteEffects.None, 0);
        }

        public void Draw(SpriteBatch spriteBatch)
        {
            spriteBatch.Draw(texture, position, new Rectangle((int)position.X - 25, (int)position.Y - 20, 50, 40), Color.Red, rotRad, new Vector2(25,20), 1.0f, SpriteEffects.None, 0);
            if(lineList.Count >= 2)
            {
                for (int i = 1; i < lineList.Count; i++)
                {
                    DrawLine(spriteBatch, lineList[i - 1], lineList[i], Color.Green);
                }
            }
            if (renderRealWorld)
            {
                for (int i = 0; i < realObjects.Count; i++)
                {
                    realObjects[i].Draw(spriteBatch);
                }
            }
            else if (!autonomous)
            {
                if (virtualWorldPoints.Count >= 2)
                {
                    for (int i = 1; i < virtualWorldPoints.Count; i++)
                    {
                        DrawLine(spriteBatch, virtualWorldPoints[i - 1], virtualWorldPoints[i], Color.Gold);
                    }
                }
            }
            else if (autonomous)
            {
                for (int i = 0; i < virtualWorldLines.Count; i++)
                {
                    DrawLine(spriteBatch, virtualWorldLines[i].Value.start, virtualWorldLines[i].Value.end, Color.Gold);
                }

                for (int i = 0; i < gotoTargets.Count; i++)
                {
                    spriteBatch.Draw(texture, new Rectangle((int)gotoTargets[i].X - 10, (int)gotoTargets[i].Y - 10, 20, 20), Color.DarkMagenta);
                }

                for (int i = 0; i < manhattanTargets.Count; i++)
                {
                    spriteBatch.Draw(texture, new Rectangle((int)manhattanTargets[i].X - 10, (int)manhattanTargets[i].Y - 10, 20, 20), Color.Orange);
                }
            }
            sensor.Draw(spriteBatch);
            
        }
    }
}
