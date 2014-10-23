using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace RCClient
{
    class SimCar //Simulated represenation of real car
    {
        Vector2 position;
        Quaternion rotation;
        List<Vector2> lineList;
        Texture2D texture;
        int prevGearState;
        int currentCheckpoint;
        bool startedGoingHome;
        public Camera camera;
        bool useSplit = false;

        public SimCar(Vector2 position, Texture2D texture)
        {
            this.position = position;
            this.texture = texture;
            rotation = Quaternion.Identity;
            lineList = new List<Vector2>();
            prevGearState = 1;
            camera = new Camera();
            camera.origin.X = 800;
            camera.origin.Y = 384;
            camera.position = position;
            currentCheckpoint = -1;
            startedGoingHome = false;
            
            lineList.Add(position);
        }

        public void Reset() //reset simcar to starting values
        {
            position = Vector2.Zero;
            rotation = Quaternion.Identity;
            lineList = new List<Vector2>();
            prevGearState = 1;
            camera = new Camera();
            camera.origin.X = 800;
            camera.origin.Y = 384;
            camera.position = position;
            currentCheckpoint = -1;
            startedGoingHome = false;

            lineList.Add(position);
        }

        public void AddState(float dist, int steering, int gear) //add a servo and distance state from the server
        {
            int reverse = prevGearState;
            if(gear != -1)
            {
                reverse = gear == 470 ? -1 : 1;
            }
            Vector2 prevPosition = position;

            //simulate the state
            steering -= 320;
            float x = (50 / Math.Abs((float)Math.Atan(steering)) + 50 / 2);
            float r = (float)Math.Sqrt(x * x + (50 / 2) * (50 / 2));
            while (dist > 0) //split the distance into smaller states, curves will be more apperent then
            {
                float theta = 0;
                float useDist = 5.0f;
                if (!useSplit)
                    useDist = dist;

                if (dist < 5.0f)
                    useDist = dist;

                dist -= useDist;
                theta = useDist * reverse / r;

                if (steering > 0.0f)
                theta = -theta;

                rotation = rotation * Quaternion.CreateFromAxisAngle(new Vector3(0, 0, 1), theta);
                Vector2 right = new Vector2(1, 0);
                Vector2 addVector = Vector2.Transform(right, rotation);

                position += addVector * useDist * reverse;
                lineList.Add(position);
            }

            if (lineList.Count >= 2) //move camera
            {
                Vector2 first = lineList[0];
                Vector2 last = lineList[lineList.Count - 1];
                camera.position = first - last;
            }
        }

        public void SetCurrentCheckpoint(int checkpoint) //used to add a marker when the car is going home
        {
            startedGoingHome = true;
            currentCheckpoint = checkpoint +1;
        }

        private void DrawLine(SpriteBatch spriteBatch, Vector2 begin, Vector2 end) //draws a line between two points
        {
            Rectangle r = new Rectangle((int)begin.X, (int)begin.Y, (int)(end - begin).Length() + 1, 1);
            Vector2 v = Vector2.Normalize(begin - end);
            float angle = (float)Math.Acos(Vector2.Dot(v, -Vector2.UnitX));
            if (begin.Y > end.Y) angle = MathHelper.TwoPi - angle;
            spriteBatch.Draw(texture, r, null, Color.Green, angle, Vector2.Zero, SpriteEffects.None, 0);
        }

        public void Draw(SpriteBatch spriteBatch) //draw to a rendertarget that will be rendered into client texture
        {
            Matrix viewMatrix = camera.GetTransform();
            spriteBatch.Begin(SpriteSortMode.Deferred, BlendState.NonPremultiplied, null, null, null, null,
                                            viewMatrix);
            if (lineList.Count >= 2)
            {
                for (int i = 1; i < lineList.Count; i++)
                {
                    DrawLine(spriteBatch, lineList[i - 1], lineList[i]);
                }
            }

            if (startedGoingHome)
            {
                currentCheckpoint = MathHelper.Clamp(currentCheckpoint, 0, lineList.Count - 1);
                Vector2 curPos = lineList[currentCheckpoint];
                spriteBatch.Draw(texture, new Rectangle((int)curPos.X - 5, (int)curPos.Y - 5, 10, 10), Color.Red);

                camera.position = -curPos;
            }

            if (currentCheckpoint <= 1 && startedGoingHome)
            {
                Reset();
            }

            spriteBatch.End();
        }
    }
}
