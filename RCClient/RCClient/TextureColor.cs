using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace RCClient
{
    class TextureColor : Texture2D
    {
        private Color color;

        public Color Color
        {
            get { return color; }
            set
            {
                if (value != color)
                {
                    color = value;
                    SetData<Color>(new Color[] { color });
                }
            }
        }

        public TextureColor(GraphicsDevice graphicsDevice) : base(graphicsDevice, 1 ,1)
        { 
            
        }

        public TextureColor(GraphicsDevice graphicsDevice, Color color)
            : base(graphicsDevice, 1, 1)
        {
            Color = color;
        }
    }
}
