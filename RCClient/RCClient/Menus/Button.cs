using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;

namespace RCClient.Menus
{
    class Button : MenuItem
    {
        Vector2 position;
        Vector2 offset;
        String text;
        Texture2D texture;
        SpriteFont font;
        bool hover = false;
        MouseState prevMouseState;
        Action trigger;

        int width;
        int height;

        public Button(Action trigger, String text, Vector2 position, int width, int height, Vector2 offset)
        {
            this.text = text;
            this.position = position;
            this.width = width;
            this.height = height;
            this.trigger = trigger;
            this.offset = offset;
        }

        public override void LoadContent(Microsoft.Xna.Framework.Content.ContentManager content)
        {
            base.LoadContent(content);

            this.texture = content.Load<Texture2D>("textboxW");
            this.font = content.Load<SpriteFont>("SpriteFont1");
        }

        public String GetText()
        {
            return text;
        }

        public override void Update()
        {
            base.Update();

            Rectangle rect = new Rectangle((int)(position.X + offset.X), (int)(position.Y + offset.Y), width, height);

            MouseState ms = Mouse.GetState();

            if (rect.Contains(new Point(ms.X, ms.Y)))
            {
                hover = true;
                if (ms.LeftButton == ButtonState.Pressed && prevMouseState.LeftButton == ButtonState.Released)
                {
                    trigger();
                }
            }
            else
                hover = false;

            prevMouseState = ms;
        }

        public override void Draw(SpriteBatch spriteBatch)
        {
            
            spriteBatch.Draw(texture, new Rectangle((int)(position.X + offset.X), (int)(position.Y + offset.Y), width, height), hover ? Color.SlateGray : Color.Gray);
            spriteBatch.DrawString(font, text, new Vector2(position.X + offset.X + width/2 - width /4, position.Y + offset.Y + height/2 - height/4), Color.Black);
        }
    }


}
