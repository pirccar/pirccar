using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using Microsoft.Xna.Framework;

/* This class is used for entering ip adresses, 
 * could also be used as a text input box
 * With some modification (and new keys)
 * */

namespace RCClient
{
    class KeyState
    {
        public Keys key;
        public double time;
        public char value;

        public KeyState(Keys key, char value = '\0')
        {
            this.key = key;
            if (value == '\0')
                this.value = (char)key;
            else
                this.value = value;
            time = 0.0;
        }
    }

    class IpInputBox
    {
        String text;
        bool selected;
        int width;
        Vector2 position;
        SpriteFont font;
        Texture2D texture;
        KeyboardState prevState;
        LinkedList<KeyState> states;

        public IpInputBox()
        {
            text = null;
            selected = false;
            width = 100;
            position = Vector2.Zero;
            font = null;

            states = new LinkedList<KeyState>();
            states.AddLast(new KeyState(Keys.D0));
            states.AddLast(new KeyState(Keys.D1));
            states.AddLast(new KeyState(Keys.D2));
            states.AddLast(new KeyState(Keys.D3));
            states.AddLast(new KeyState(Keys.D4));
            states.AddLast(new KeyState(Keys.D5));
            states.AddLast(new KeyState(Keys.D6));
            states.AddLast(new KeyState(Keys.D7));
            states.AddLast(new KeyState(Keys.D8));
            states.AddLast(new KeyState(Keys.D9));
            states.AddLast(new KeyState(Keys.NumPad0, (char)Keys.D0));
            states.AddLast(new KeyState(Keys.NumPad1, (char)Keys.D1));
            states.AddLast(new KeyState(Keys.NumPad2, (char)Keys.D2));
            states.AddLast(new KeyState(Keys.NumPad3, (char)Keys.D3));
            states.AddLast(new KeyState(Keys.NumPad4, (char)Keys.D4));
            states.AddLast(new KeyState(Keys.NumPad5, (char)Keys.D5));
            states.AddLast(new KeyState(Keys.NumPad6, (char)Keys.D6));
            states.AddLast(new KeyState(Keys.NumPad7, (char)Keys.D7));
            states.AddLast(new KeyState(Keys.NumPad8, (char)Keys.D8));
            states.AddLast(new KeyState(Keys.NumPad9, (char)Keys.D9));
            states.AddLast(new KeyState(Keys.OemPeriod));
            states.AddLast(new KeyState(Keys.Back));
        }

        public void SetTexture(Texture2D texture)
        {
            this.texture = texture;
        }

        public void SetFont(SpriteFont font)
        {
            this.font = font;
        }

        public void SetPosition(Vector2 pos)
        {
            this.position = pos;
        }

        public void SetWidth(int width)
        {
            this.width = width;
        }

        public bool isSelected()
        {
            return selected;
        }

        public String GetText()
        {
            return text;
        }

        public void SetText(String text)
        {
            this.text = text;
        }

        public void Update(GameTime gameTime)
        {
            MouseState state = Mouse.GetState();

            if (font != null)
            {
                if (state.LeftButton == ButtonState.Pressed)
                {
                    Rectangle boxRect = new Rectangle((int)position.X, (int)position.Y, width, font.LineSpacing);
                    if (boxRect.Contains(new Point(state.X, state.Y)))
                    {
                        selected = true;
                    }
                    else
                    {
                        selected = false;
                    }
                }

                if(selected)
                    ParseInput(gameTime);
            }
        }

        private void ParseInput(GameTime gameTime)
        {
            KeyboardState keyState = Keyboard.GetState();
            foreach (KeyState key in states)
            {
                if (keyState.IsKeyDown(key.key))
                {
                    if (key.key == Keys.Back)
                    {
                        if (text.Length > 0)
                        {
                            if (prevState.IsKeyDown(key.key))
                            {
                                if (key.time >= 0.15)
                                {
                                    text = text.Remove(text.Length - 1, 1);
                                    key.time = 0.0;
                                }
                            }
                            else
                            {
                                text = text.Remove(text.Length - 1, 1);
                            }
                        }
                    }
                    else if(key.key == Keys.OemPeriod)
                    {
                        if (prevState.IsKeyDown(key.key))
                        {
                            if (key.time >= 0.33)
                            {
                                text += ".";
                                key.time = 0.0;
                            }
                        }
                        else
                        {
                            text += ".";
                        }
                    }
                    else 
                    {
                        if (prevState.IsKeyDown(key.key))
                        {
                            if (key.time >= 0.33)
                            {
                                text += key.value;
                                key.time = 0.0;
                            }
                        }
                        else
                        {
                            text += key.value;
                        }
                    }

                    key.time += gameTime.ElapsedGameTime.TotalSeconds;
                }
                else
                {
                    key.time = 0.0;
                }
            }

            prevState = keyState;
        }

        public void Draw(SpriteBatch spriteBatch)
        {
            if(font != null)
            {
                if (texture != null)
                {
                    spriteBatch.Draw(texture, new Rectangle((int)position.X, (int)position.Y, width, font.LineSpacing), Color.White);
                    if(text != null)
                        spriteBatch.DrawString(font, text, position, Color.Black);
                }
                else
                {
                    if (text != null)
                        spriteBatch.DrawString(font, text, position, Color.White);
                }
            }
        }
    }
}
