using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;


namespace RCClient.Menus
{
    class MenuItem
    {
        Vector2 position;

        public MenuItem()
        {
            position = Vector2.Zero;
        }

        public virtual void LoadContent(ContentManager content)
        { 
            
        }

        public virtual void Update()
        { 
        
        }

        public virtual void Draw(SpriteBatch spriteBatch)
        { 
        
        }
    }
}
