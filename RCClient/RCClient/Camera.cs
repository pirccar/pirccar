using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;

namespace RCClient
{
    class Camera
    {
        public float zoom;
        public Vector2 position;
        public Vector2 origin;

        public Camera()
        {
            zoom = 5;
            position = Vector2.Zero;
            origin = Vector2.Zero;
            origin = new Vector2(0, 0);
        }

        public void Move(Vector2 direction)
        {
            position += direction;
        }

        public void Zoom(float direction)
        {
            if(direction > 0)
            {
                zoom += 0.5f;
                if (zoom > 10.0f)
                    zoom = 10.0f;
            }
            else if (direction < 0)
            {
                zoom -= 0.5f;
                if (zoom <= 0)
                    zoom = 0.5f;
            }
        }

        public Matrix GetTransform()
        {
            Matrix translationMatrix = Matrix.CreateTranslation(new Vector3(position.X, position.Y, 0));
            Matrix scaleMatrix = Matrix.CreateScale(new Vector3(zoom, zoom, 1));
            Matrix originMatrix = Matrix.CreateTranslation(new Vector3(origin.X, origin.Y, 0));

            return translationMatrix * scaleMatrix * originMatrix;
        }


    }
}
