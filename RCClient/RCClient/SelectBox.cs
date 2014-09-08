using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;

namespace RCClient
{
    class SelectBox
    {
        Vector2 position;
        int width;
        int height;
        int expandHeight;
        String value;
        LinkedList<String> selectables;
        Texture2D texture;
        SpriteFont font;
        MouseState prevState;
        bool selected;
        bool dropdown;
        int index = 0;

        public SelectBox(Vector2 position, int width, int height, bool dropdown = false)
        {
            this.position = position;
            this.width = width;
            this.height = height;
            value = "-";
            selected = false;
            texture = null;
            font = null;
            selectables = new LinkedList<String>();
            this.dropdown = dropdown;
        }

        public String GetValue()
        {
            return value;
        }

        public Rectangle GetDropdownRect()
        {
            Rectangle rect;

            if (position.Y + height + expandHeight + 5 > 768)
                rect = new Rectangle((int)position.X, (int)position.Y - 5 - expandHeight, width, expandHeight);
            else
                rect = new Rectangle((int)position.X, (int)position.Y + height + 5, width, expandHeight);

            return rect;
        }

        public bool IsCovered(Rectangle rect)
        {
            return rect.Contains(new Point((int)position.X, (int)position.Y)) || rect.Contains(new Point((int)position.X + width, (int)position.Y + height));
        }

        public void SetValue(String value)
        {
            this.value = value;
        }

        public bool IsSelected()
        {
            return selected;
        }

        public bool IsDropdown()
        {
            return dropdown;
        }

        public void SetSelected(bool selected)
        {
            this.selected = selected;
        }

        public void SetSelectables(LinkedList<String> list)
        {
            this.selectables = list;
            value = selectables.ElementAt(index);
            expandHeight = selectables.Count * 35;
        }

        public void SetTexture(Texture2D texture)
        {
            this.texture = texture;
        }

        public void SetFont(SpriteFont font)
        {
            this.font = font;
        }

        public void Increase()
        {
            index++;
            if (index > selectables.Count-1)
                index = 0;

            value = selectables.ElementAt(index);
        }

        public void Decrease()
        {
            index--;
            if (index < 0)
                index = selectables.Count - 1;

            value = selectables.ElementAt(index);
        }

        public void Update()
        {
            MouseState state = Mouse.GetState();

            if (state.LeftButton == ButtonState.Pressed && prevState.LeftButton == ButtonState.Released)
            {
                Rectangle boxRect = new Rectangle((int)position.X, (int)position.Y, width, font.LineSpacing);
                
                if (boxRect.Contains(new Point(state.X, state.Y)))
                {
                    if (dropdown)
                        selected = !selected;
                    else
                        selected = true;
                }
                else if (GetDropdownRect().Contains(new Point(state.X, state.Y)) && selected)
                {
                    updateDropdown();
                }
                else
                {
                    selected = false;
                }
            }

            
            prevState = state;
        }

        public void updateDropdown()
        {
            MouseState state = Mouse.GetState();
            if (dropdown && selected)
            {
                if (position.Y + height + expandHeight + 5 > 768)
                {
                    Rectangle dropdownRect = new Rectangle((int)position.X, (int)position.Y - expandHeight - 5, width, expandHeight);
                    if (dropdownRect.Contains(new Point(state.X, state.Y)))
                    {
                        index = (state.Y - ((int)position.Y - expandHeight -5)) / 35;
                        value = selectables.ElementAt(index);
                        selected = false;
                    }
                }
                else
                {
                    Rectangle dropdownRect = new Rectangle((int)position.X, (int)position.Y + height + 5, width, expandHeight);
                    if (dropdownRect.Contains(new Point(state.X, state.Y)))
                    {
                        int stride = (int)position.Y + height + 5;
                        index = (state.Y - stride) / 35;
                        value = selectables.ElementAt(index);
                        selected = false;
                    }
                }
            }
        }

        public void Draw(SpriteBatch spriteBatch)
        {
            if (texture != null)
            {
                if (selected)
                {
                    spriteBatch.Draw(texture, new Rectangle((int)position.X, (int)position.Y, width, height), Color.White);
                    if (dropdown)
                    {
                        MouseState state = Mouse.GetState();
                        if (position.Y + height + expandHeight + 5 > 768) //upwards
                        {
                            spriteBatch.Draw(texture, new Rectangle((int)position.X, (int)position.Y - expandHeight - 5, width, expandHeight), Color.White);
                            int i = 0;
                            int highlight = (state.Y - ((int)position.Y - expandHeight - 5)) / 35;
                            foreach (String s in selectables)
                            {
                                if (i == highlight)
                                    spriteBatch.DrawString(font, s, new Vector2(position.X, position.Y - expandHeight - 5 + (i * 35)), Color.LightSlateGray);
                                else
                                    spriteBatch.DrawString(font, s, new Vector2(position.X, position.Y - expandHeight- 5 + (i * 35)), Color.Black);
                                i++;
                            }
                        }
                        else //down
                        {
                            spriteBatch.Draw(texture, new Rectangle((int)position.X, (int)position.Y + height + 5, width, expandHeight), Color.White);
                            int i = 0;
                            int stride = (int)position.Y + height + 5;
                            int highlight = (state.Y - stride) / 35;
                            foreach (String s in selectables)
                            { 
                                if(i == highlight)
                                    spriteBatch.DrawString(font, s, new Vector2(position.X, position.Y + height + 5 + (i * 35)), Color.LightSlateGray);
                                else
                                    spriteBatch.DrawString(font, s, new Vector2(position.X, position.Y + height + 5 + (i * 35)), Color.Black);
                                i++;
                            }
                        }
                    }
                }
                else
                {
                    spriteBatch.Draw(texture, new Rectangle((int)position.X, (int)position.Y, width, height), Color.LightGray);
                }

                spriteBatch.DrawString(font, value, new Vector2(position.X, position.Y), Color.Black);
            }
        }
    }
}
