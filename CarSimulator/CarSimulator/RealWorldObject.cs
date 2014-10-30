using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace CarSimulator
{
    class RealWorldObject
    {
        Rectangle rectangle;
        Texture2D texture;


        public RealWorldObject(Vector2 position, int width, int height, Texture2D texture)
        {
            this.rectangle = new Rectangle((int)position.X, (int)position.Y, width, height);
            this.texture = texture;
        }

        public Rectangle GetRectangle()
        {
            return rectangle;
        }

        public void Draw(SpriteBatch spriteBatch)
        {
            spriteBatch.Draw(texture, rectangle, Color.DarkGray);
        }
    }
}
