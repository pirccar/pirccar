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

    class Car
    {
        Servo[] servos;
        Texture2D texture;
        float acceleration;
        Vector2 position;
        Vector2 direction;
        Quaternion rotation;
        float rotRad;
        float steering;
        float speed;
        int reverse;
        float tireWidth;
        float tireRowSpacing;
        List<Vector2> lineList;
        float lineListDistance;

        DistanceSensor sensor;
        DistanceSensor[] passiveSensors;
        List<RealWorldObject> realObjects;
        List<Vector2> virtualWorldPoints;
        List<Line?> virtualWorldLines;
        Vector2 gotoTarget;
        List<Vector2> gotoTargets;
        int gotoTargetIndex;
        bool renderRealWorld;
        bool autonomous;
        bool turnAround;
        float turnAroundStartRotation;
        bool turnAroundEStop;
        bool turnAroundSStop;
        float travelDistance;
        float travelDistanceLimit;
        int scanSteps;
        int scanIndex;
        bool sensorLeft;
        bool detectAny;
        bool scanning;
        float previousPassiveValue;
        int collLeft;
        int collRight;
        Vector2? lineStart;
        Vector2? lineEnd;

        List<ServoTiming> goHomeData;
        int goHomeIndex;
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
            virtualWorldPoints = new List<Vector2>();
            virtualWorldLines = new List<Line?>();
            renderRealWorld = true;
            autonomous = false;
            travelDistance = 0.0f;
            scanSteps = 40;
            scanIndex = 0;
            sensorLeft = true;
            detectAny = false;
            gotoTargets = new List<Vector2>();
            gotoTargetIndex = 0;
            turnAround = false;
            scanning = false;
            turnAroundEStop = false;
            turnAroundSStop = false;

            realObjects = new List<RealWorldObject>();
            /*
            RealWorldObject realObject = new RealWorldObject(new Vector2(550, 150), 25, 400, texture);
            realObjects.Add(realObject);
            realObject = new RealWorldObject(new Vector2(0, 0), 550, 25, texture);
            realObjects.Add(realObject);
            realObject = new RealWorldObject(new Vector2(0, 0), 25, 500, texture);
            realObjects.Add(realObject);
            realObject = new RealWorldObject(new Vector2(0, 450), 550, 25, texture);
            realObjects.Add(realObject);
            */

            RealWorldObject realObject = new RealWorldObject(new Vector2(550, 300), 50, 50, texture);
            realObjects.Add(realObject);
            realObject = new RealWorldObject(new Vector2(200, 375), 50, 50, texture);
            realObjects.Add(realObject);
            realObject = new RealWorldObject(new Vector2(300, 150), 50, 50, texture);
            realObjects.Add(realObject);
            realObject = new RealWorldObject(new Vector2(575, 125), 50, 50, texture);
            realObjects.Add(realObject);

            goHomeIndex = -1;
            goHome = false;

            for (int i = 0; i < passiveSensors.Length; i++)
            {
                passiveSensors[i] = new DistanceSensor(position, rotRad, texture);
            }
            passiveSensors[0].SetInternalRotation(-MathHelper.PiOver4);
            passiveSensors[1].SetInternalRotation((float)-Math.PI / 8);
            passiveSensors[2].SetInternalRotation((float)Math.PI / 8);
            passiveSensors[3].SetInternalRotation(MathHelper.PiOver4);
        }

        public Vector2 GetPosition()
        {
            return position;
        }

        public float GetSpeed()
        {
            return speed;
        }

        public float GetSteering()
        {
            return steering;
        }

        public float GetRotation()
        {
            return rotRad;
        }

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
            detectAny = false;
            gotoTargetIndex = 0;
        }

        public void SetServo(int channel, int value)
        {
            servos[channel].SetValue(value);
        }

        public void GoHome()
        {
            if (!goHome)
            {
                errorSum = 0;
                lastError = 0;
            }
            goHome = true;
        }

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
            acceleration = (distance * 4.35f) / calcSpeed;
            acceleration = (int)MathHelper.Clamp(acceleration, 1, 15); 
            
        }

        private void CalculateSpeedPid(float distance, GameTime gameTime)
        {
            float timeNow = (float)gameTime.ElapsedGameTime.TotalSeconds;

            errorSum += (distance * timeNow);
            float dErr = (distance - lastError) / timeNow;

            acceleration = kp * distance + ki * errorSum + kd * dErr;
            lastError = distance;
        }

        private void InternalGoHome(GameTime gameTime)
        {
            if(goHomeIndex == -1) //this idicates that we have gone thru every state
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

        public void ToggleRealWorld()
        {
            renderRealWorld = !renderRealWorld;
        }

        public void ToggleAutonomous()
        {
            autonomous = !autonomous;
        }

        public void SetGotoTarget(Vector2 target)
        {
            this.gotoTarget = target;
        }

        public void AddGotoTarget(Vector2 target)
        {
            gotoTargets.Add(target);
        }

        private void AutonomousUpdate(GameTime gameTime)
        {
            float? passive = CheckPasiveSensors();
            if (gotoTargetIndex >= gotoTargets.Count)
            {
                acceleration = 0;
                gotoTargetIndex = 0;
                return;
            }
            else if (scanning || travelDistance >= travelDistanceLimit)
            //else if (scanning || (passive != previousPassiveValue && passive != null && passive.Value <= 250.0f))
            {
                acceleration = 0.0f;
                scanning = true;
                if (speed == 0.0f)
                {
                    if (scanIndex == 0)
                    {
                        virtualWorldLines.Clear();
                        lineStart = null;
                        lineEnd = null;
                        collLeft = 0;
                        collRight = 0;
                    }
                    InternalScan(gameTime);

                    if (scanIndex == 0 && travelDistance == 0)
                    {
                        CheckCollision(rotRad, Vector2.Distance(position, gotoTargets[gotoTargets.Count - 1 - gotoTargetIndex]));
                        //CheckNextState();
                        scanning = false;
                    }
                }
            }
            else if(gotoTargetIndex != -1)
            {
                if (gotoTargetIndex >= gotoTargets.Count)
                {
                    acceleration = 0;
                    gotoTargetIndex = 0;
                    return;
                }

                Vector2 dir = new Vector2((float)Math.Cos(rotRad), (float)Math.Sin(rotRad));
                Vector2 tarDir = gotoTargets[gotoTargets.Count - 1 - gotoTargetIndex] - position;
                
                tarDir.Normalize();

                float v1 = (float)Math.Atan2(tarDir.Y, tarDir.X);
                float v2 = (float)Math.Atan2(dir.Y, dir.X);
                float angle1 = v1 - v2;
                float angle2 = v2 - v1;
                float angleToTarget = angle1;
                if(angleToTarget > Math.PI)
                {
                    float removeFactor = angleToTarget - (float)Math.PI;
                    angleToTarget = (float)-Math.PI + removeFactor;
                }

                steering = (int)MathHelper.ToDegrees(angleToTarget);
                steering = -(int)MathHelper.Clamp(steering, -48, 48);
                float distance = Vector2.Distance(position, gotoTargets[gotoTargets.Count - 1 - gotoTargetIndex]);

                CalculateSpeed(distance < travelDistanceLimit - travelDistance ? distance : travelDistanceLimit - travelDistance);
                //CalculateSpeed(distance);
                if(distance <= 15.0f)
                {
                    gotoTargets.RemoveAt(gotoTargets.Count - 1 - gotoTargetIndex);
                    //gotoTargetIndex++;
                    travelDistanceLimit = 0;
                }
            }

            previousPassiveValue = passive.HasValue ? passive.Value : 0;
        }

        private void CheckNextState()
        {
            Vector2 dir = new Vector2((float)Math.Cos(rotRad), (float)Math.Sin(rotRad));
            Vector2 tarDir = gotoTargets[gotoTargets.Count - 1 - gotoTargetIndex] - position;
            float distance = Vector2.Distance(position, gotoTargets[gotoTargets.Count - 1 - gotoTargetIndex]);
            distance = distance < travelDistanceLimit - travelDistance ? distance : travelDistanceLimit - travelDistance;

            tarDir.Normalize();

            float v1 = (float)Math.Atan2(tarDir.Y, tarDir.X);
            float v2 = (float)Math.Atan2(dir.Y, dir.X);
            float angle1 = v1 - v2;
            float angle2 = v2 - v1;
            float angleToTarget = angle1;
            if (angleToTarget > Math.PI)
            {
                float removeFactor = angleToTarget - (float)Math.PI;
                angleToTarget = (float)-Math.PI + removeFactor;
            }

            steering = (int)MathHelper.ToDegrees(angleToTarget);
            steering = -(int)MathHelper.Clamp(steering, -48, 48);

            float x = (tireRowSpacing / Math.Abs((float)Math.Atan(steering)) + tireWidth / 2);
            float r = (float)Math.Sqrt(x * x + (tireRowSpacing / 2) * (tireRowSpacing / 2));
            float theta = (distance * reverse) / r;

            if (steering > 0.0f)
                theta = -theta;

            Quaternion newRotation = rotation * Quaternion.CreateFromAxisAngle(new Vector3(0, 0, 1), theta);

            Vector2 right = new Vector2(1, 0);
            Vector2 addVector = Vector2.Transform(right, newRotation);

            Vector2 calcPosition = position + addVector * reverse * distance;
            bool collision = false;
            for (int i = 0; i < virtualWorldLines.Count; i++)
            {
                if (virtualWorldLines[i] == null)
                    continue;
                if (lineIntersect(virtualWorldLines[i].Value.start, virtualWorldLines[i].Value.end, position, calcPosition))
                {
                    collision = true;
                }
            }

            if(collision)
            {
                gotoTargets.Add(position + dir * 50);
            }
        }

        private void InternalScan(GameTime gameTime)
        {
            if (sensorLeft)
                sensor.SetRotationTarget((float)(Math.PI / scanSteps) * (float)scanIndex - MathHelper.PiOver2);
            else
                sensor.SetRotationTarget((float)(MathHelper.PiOver2 - (Math.PI / scanSteps) * (float)scanIndex));
            sensor.Update(gameTime);
            if(!sensor.Rotating())
            {
                float? sensorValue = SensorDected();
                if (sensorValue != null)
                    detectAny = true;

                if (sensorValue != null)
                {
                    if (scanIndex < scanSteps / 2)
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
                AddVirtualLine(sensorValue);
                scanIndex++;
                if (scanIndex >= scanSteps+1)
                {
                    travelDistance = 0;
                    //travelDistanceLimit = (detectAny ? 50.0f : 100.0f);
                    scanIndex = 0;
                    sensorLeft = !sensorLeft;
                    detectAny = false;
                }
            }
        }

        private float? CheckPasiveSensors()
        {
            float? returnValue = null;
            
            for (int i = 0; i < passiveSensors.Length; i++)
            {
                passiveSensors[i].SetPosition(position);
                passiveSensors[i].SetDirection(rotRad);
                for (int j = 0; j < realObjects.Count; j++)
                {
                    float? sensorValue = passiveSensors[i].Intersect(realObjects[j].GetRectangle());
                    if (returnValue == null || sensorValue < returnValue)
                        returnValue = sensorValue;
                }
                passiveSensors[i].SetCollDistance(returnValue);
            }

            return returnValue;
        }

        private void CheckCollision(float rotation, float distance)
        {
            float newRot = rotation;
            float newDist = distance;
            bool pointAdded = false;
            int nChecks = 0;
            while (!pointAdded)
            {
                Vector2 newDir = new Vector2((float)Math.Cos(newRot), (float)Math.Sin(newRot));
                bool collision = false;
                float smallestDist = float.MaxValue;
                for (int i = 0; i < virtualWorldLines.Count; i++)
                {
                    if (virtualWorldLines[i] == null)
                        continue;
                    if (lineIntersect(virtualWorldLines[i].Value.start, virtualWorldLines[i].Value.end, position, position + newDir * newDist))
                    {
                        collision = true;
                        float d1 = Vector2.Distance(position, virtualWorldLines[i].Value.start);
                        float d2 = Vector2.Distance(position, virtualWorldLines[i].Value.end);
                        float s = d1 < d2 ? d1 : d2;
                        smallestDist = smallestDist < s ? smallestDist : s;
                    }
                }
                if (collision)
                {
                    if (collLeft < collRight)
                        newRot -= (float)Math.PI / 16;
                    else
                        newRot += (float)Math.PI / 16;
                    newDist = smallestDist;
                    travelDistanceLimit = 50.0f;
                    nChecks++;
                }
                else if (newRot != rotation)
                {
                    gotoTargets.Add(position + newDir * (newDist - newDist / 1.25f));
                    pointAdded = true;
                }
                else
                {
                    for (int i = 0; i < virtualWorldLines.Count; i++)
                    {
                        if (virtualWorldLines[i] == null)
                            continue;
                        if (lineIntersect(virtualWorldLines[i].Value.start, virtualWorldLines[i].Value.end, position, gotoTargets[gotoTargets.Count -1]))
                        {
                            gotoTargets.Add(position + newDir * 150);
                            break;
                        }
                    }
                    
                    pointAdded = true;
                    travelDistanceLimit = 150.0f;
                }
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
            Vector2 addVector = Vector2.Transform(right, rotation);

            rotRad += theta;

            position += addVector * speed * reverse * dt;

            if (autonomous)
                travelDistance += speed * reverse * dt;

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
            else if(!autonomous)
            {
                InternalGoHome(gameTime);
            }
            else
            {
                AutonomousUpdate(gameTime);
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

                for (int i = 0; i < passiveSensors.Length; i++)
                {
                    passiveSensors[i].Draw(spriteBatch);
                }
            }
            sensor.Draw(spriteBatch);
            
        }
    }
}
