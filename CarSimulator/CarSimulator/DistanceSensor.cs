using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace CarSimulator
{
    class DistanceSensor
    {
        Vector2 position;
        float rotation;
        float internalRotation;
        bool goingRight;
        float max = 300.0f;
        float collDistance = -1;
        Texture2D texture;
        float roationTarget;
        bool stop;

        public DistanceSensor(Vector2 position, float rotation, Texture2D texture)
        {
            this.position = position;
            this.rotation = rotation;
            this.texture = texture;
            internalRotation = -MathHelper.PiOver2;
            goingRight = true;
            roationTarget = -MathHelper.PiOver2;
            stop = true;
        }

        public void SetPosition(Vector2 position)
        {
            this.position = position;
        }

        public void SetDirection(float rotation)
        {
            this.rotation = rotation;
        }

        public void SetInternalRotation(float rotation)
        {
            this.internalRotation = rotation;
        }

        public void SetCollDistance(float? dist)
        {
            if (dist == null)
                collDistance = -1;
            else
                collDistance = dist.Value;
        }

        public Vector2 GetDirection()
        {
            return new Vector2((float)Math.Cos(rotation + internalRotation), (float)Math.Sin(rotation + internalRotation));
        }

        public float? Intersect(Rectangle r)
        {
            Vector2 dir = new Vector2((float)Math.Cos(rotation + internalRotation), (float)Math.Sin(rotation + internalRotation));
            Ray ray = new Ray(new Vector3(position.X, position.Y, 0), new Vector3(dir.X, dir.Y, 0));
            BoundingBox b = new BoundingBox(new Vector3(r.X, r.Y, 0), new Vector3(r.X + r.Width, r.Y + r.Height, 1));
            float? result = ray.Intersects(b);

            if (result == null || result > max)
            {
                //collDistance = -1;
                return null;
            }
            //collDistance = result.Value;
            return result;
        }

        public void SetRotationTarget(float target)
        {
            roationTarget = target;
        }

        public bool Rotating()
        {
            return !stop;
        }

        private void DrawLine(SpriteBatch spriteBatch, Vector2 begin, Vector2 end)
        {
            Rectangle r = new Rectangle((int)begin.X, (int)begin.Y, (int)(end - begin).Length() + 1, 1);
            Vector2 v = Vector2.Normalize(begin - end);
            float angle = (float)Math.Acos(Vector2.Dot(v, -Vector2.UnitX));
            if (begin.Y > end.Y) angle = MathHelper.TwoPi - angle;
            spriteBatch.Draw(texture, r, null, Color.DarkViolet, angle, Vector2.Zero, SpriteEffects.None, 0);
        }

        public void Update(GameTime gameTime)
        {
            if (Math.Round(internalRotation, 2) > Math.Round(roationTarget, 2))
            {
                goingRight = false;
                stop = false;
            }
            else if (Math.Round(internalRotation, 2) < Math.Round(roationTarget, 2))
            {
                goingRight = true;
                stop = false;
            }
            else
                stop = true;

            if (stop == false)
                internalRotation += MathHelper.ToRadians(45.0f) * (goingRight ? 1 : -1) * (float)gameTime.ElapsedGameTime.TotalSeconds;
        }

        public void Draw(SpriteBatch spriteBatch)
        {
            Vector2 dir = new Vector2((float)Math.Cos(rotation + internalRotation), (float)Math.Sin(rotation + internalRotation));
            Vector2 endPos = position + dir * (collDistance > -1 ? collDistance : max);
            DrawLine(spriteBatch, position, endPos);
        }
    }
}
