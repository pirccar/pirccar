using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;

namespace RCClient
{
    class SelectBoxHandler
    {
        LinkedList<SelectBox> boxes;
        LinkedList<String> selectables;
        int selected;


        public SelectBoxHandler()
        {
            boxes = new LinkedList<SelectBox>();
            selectables = new LinkedList<String>();
            selected = -1;
        }

        public void LoadDefaults()
        {
            selectables.AddLast("None");
            selectables.AddLast("Throttle Front");
            selectables.AddLast("Throttle Back");
            selectables.AddLast("Steering Front");
            selectables.AddLast("Steering Back");
            selectables.AddLast("Camera X");
            selectables.AddLast("Camera Y");
            selectables.AddLast("Gear Switch");
        }

        public void Add(Vector2 position, int width, int height, bool dropdown = false)
        {
            boxes.AddLast(new SelectBox(position, width, height, dropdown));
            boxes.Last.Value.SetSelectables(selectables);
        }

        public void InitAll(Texture2D texture, SpriteFont font)
        {
            foreach (SelectBox sb in boxes)
            {
                sb.SetTexture(texture);
                sb.SetFont(font);
            }
        }

        public void Update()
        {
            bool foundSelected = false;
            int i = 0;
            Rectangle coverRect = new Rectangle();
            foreach (SelectBox sb in boxes)
            {
                if (sb.IsSelected())
                {
                    selected = i;
                    foundSelected = true;
                    if (sb.IsDropdown())
                        coverRect = sb.GetDropdownRect();
                    break;
                }
                i++;
            }

            i = 0;
            foreach (SelectBox sb in boxes)
            {
                if(!sb.IsCovered(coverRect) || i == selected || !foundSelected)
                    sb.Update();
                i++;
            }
            int count = 0;
            if (foundSelected)
            {
                foreach (SelectBox sb in boxes)
                {
                    if (count != selected)
                    {
                        sb.SetSelected(false);
                    }
                    count++;
                }
            }
        }

        public void SelectIncrease()
        {
            selected++;

            if (selected > boxes.Count - 1)
                selected = 0;

            int count = 0;
            foreach (SelectBox sb in boxes)
            {
                if (count == selected)
                    sb.SetSelected(true);
                else
                    sb.SetSelected(false);
            }
        }

        public void SelectDecrease()
        {
            selected--;

            if (selected <0)
                selected = boxes.Count - 1; ;

            int count = 0;
            foreach (SelectBox sb in boxes)
            {
                if (count == selected)
                    sb.SetSelected(true);
                else
                    sb.SetSelected(false);
            }
        }

        public void ForwardSelection()
        {
            foreach (SelectBox sb in boxes)
            {
                if (sb.IsSelected())
                {
                    sb.Increase();
                }
            }
        }

        public void BackwardSelection()
        {
            foreach (SelectBox sb in boxes)
            {
                if (sb.IsSelected())
                {
                    sb.Decrease();
                }
            }
        }

        public String GetBoxValue(int index)
        {
            if (index >= 0 && index <= boxes.Count - 1)
            {
                return boxes.ElementAt(index).GetValue();
            }

            return null;
        }

        public void SetBoxValue(int index, String value)
        {
            if (index >= 0 && index <= boxes.Count - 1)
            {
                boxes.ElementAt(index).SetValue(value);
            }
        }

        public void Draw(SpriteBatch spriteBatch)
        {
            foreach (SelectBox sb in boxes)
            {
                if(!sb.IsSelected())
                    sb.Draw(spriteBatch);
            }
            foreach (SelectBox sb in boxes)
            {
                if (sb.IsSelected())
                    sb.Draw(spriteBatch);
            }
        }
    }
}
