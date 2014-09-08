using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using Microsoft.Xna.Framework.Content;

namespace RCClient.Menus
{
    class Checkbox : MenuItem
    {
        bool check;
        bool hover;
        Vector2 position;
        String text;
        int width;
        int height;
        Texture2D checkboxUnchecked;
        Texture2D checkboxChecked;
        SpriteFont font;
        MouseState prevMouseState;
        Rectangle rect;
        Vector2 offset;
        int id;
        Action<bool, int> trigger;

        public Checkbox(Action<bool, int>trigger, int id, Vector2 position, Vector2 offset, String text = "")
        {
            this.position = position;
            this.text = text;
            width = 100;
            height = 80;
            this.offset = offset;
            this.id = id;
            this.trigger = trigger;
        }

        public override void LoadContent(ContentManager content)
        {
            this.font = content.Load<SpriteFont>("SpriteFont1");
            this.checkboxUnchecked = content.Load<Texture2D>("box");
            this.checkboxChecked = content.Load<Texture2D>("boxChecked");

            Vector2 textSize = font.MeasureString(text);
            width = (int)textSize.X + 50 + checkboxChecked.Bounds.Width;
            height = (int)textSize.Y + 10;
        }

        public bool GetChecked()
        {
            return check;
        }

        public override void Update()
        {
            rect = new Rectangle((int)(position.X + offset.X), (int)(position.Y + offset.Y), width, height);

            MouseState ms = Mouse.GetState();

            if (rect.Contains(new Point(ms.X, ms.Y)))
            {
                hover = true;
                if (ms.LeftButton == ButtonState.Pressed && prevMouseState.LeftButton == ButtonState.Released)
                {
                    check = !check;
                    trigger(check, id);
                }
            }
            else
                hover = false;

            prevMouseState = ms;
        }

        public override void Draw(SpriteBatch spriteBatch)
        {
            spriteBatch.Draw(check ? checkboxChecked : checkboxUnchecked, new Vector2(offset.X + width - checkboxUnchecked.Width,  offset.Y + position.Y), Color.White);
            spriteBatch.DrawString(font, text, new Vector2(offset.X + position.X - 50, offset.Y + position.Y), hover ? Color.SlateGray :  Color.Black);
        }
    }
}
