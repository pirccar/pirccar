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
    class Menu
    {
        bool open;
        List<Button> buttons;
        Texture2D bg;
        Texture2D frame;
        SpriteFont font;
        String caption;
        Vector2 position;
        MouseState prevMouseState;
        bool quit = false;
        bool renderOptions = false;
        bool doDisconnect = false;

        int screenWidth;
        int screenHeight;

        //Options
        List<MenuItem> optionsItems;

        public Menu(Vector2 position, int width, int height)
        {
            screenWidth = width;
            screenHeight = height;
            this.position = position;
            caption = "Rc Controller";
            buttons = new List<Button>();
            buttons.Add(new Button(Hide, "Resume", new Vector2(100, 50), 200, 80, position));
            buttons.Add(new Button(OpenOptions, "Options", new Vector2(100, 150), 200, 80, position));
            buttons.Add(new Button(Disconnect, "Disconnect", new Vector2(100, 250), 200, 80, position));
            buttons.Add(new Button(Quit, "Quit", new Vector2(100, 350), 200, 80, position));

            optionsItems = new List<MenuItem>();
            optionsItems.Add(new Button(CloseOptions, "Back", new Vector2(100, 400), 200, 80, new Vector2(position.X, position.Y - 50)));
            optionsItems.Add(new Checkbox(CheckBoxStatus, 0, new Vector2(50, 50), new Vector2(position.X, position.Y - 50),  "Always render map"));
        }

        public void LoadContent(GraphicsDevice graphicsDevice, ContentManager content)
        {
            this.font = content.Load<SpriteFont>("SpriteFont1");
            Color c = Color.Black;
            c.A = 128;
            this.bg = new TextureColor(graphicsDevice, c);
            this.frame = content.Load<Texture2D>("textboxW");

            foreach (Button b in buttons)
            {
                b.LoadContent(content);
            }

            foreach (MenuItem mi in optionsItems)
            {
                mi.LoadContent(content);
            }
        }

        public void Show()
        {
            open = true;
        }

        public void Hide()
        {
            open = false;
        }

        public void Toggle()
        {
            open = !open;
        }

        public bool isOpen()
        {
            return open;
        }

        public bool GetQuit()
        {
            return quit;
        }

        public bool GetDisconnect()
        {
            bool ret = doDisconnect;
            doDisconnect = false;
            return ret;
        }

        public void Quit()
        {
            quit = true;
        }

        public void OpenOptions()
        {
            renderOptions = true;
        }

        public void CloseOptions()
        {
            renderOptions = false;
        }

        public void Disconnect()
        {
            doDisconnect = true;
            Hide();
        }

        public void CheckBoxStatus(bool on, int id)
        { 
            //Handle status of checboxes
        }

        public void Update()
        {
            if (open)
            {                
                MouseState ms = Mouse.GetState();
                if (!renderOptions)
                {
                    foreach (MenuItem mi in buttons)
                    {
                        mi.Update();
                    }
                }
                else
                { 
                    //update optionswindow here
                    foreach (MenuItem mi in optionsItems)
                    {
                        mi.Update();
                    }
                }

                prevMouseState = ms;
            }
        }

        public void Draw(SpriteBatch spriteBatch)
        {
            if (open)
            {
                spriteBatch.Draw(bg, new Rectangle(0, 0, screenWidth, screenHeight), Color.White);
                spriteBatch.Draw(frame, new Rectangle((int)position.X, (int)position.Y, 400, 500), Color.DarkGray);
                spriteBatch.DrawString(font, caption, new Vector2(position.X + position.X/5, position.Y), Color.Black);

                foreach (MenuItem mi in buttons)
                {
                    mi.Draw(spriteBatch);
                }

                if (renderOptions)
                { 
                    //Render options window here
                    spriteBatch.Draw(frame, new Rectangle((int)position.X - 50, (int)position.Y - 50, 500, 500), Color.DarkGray);
                    spriteBatch.DrawString(font, "Options", new Vector2(screenWidth / 2 - 50, position.Y -50), Color.Black);
                    foreach (MenuItem mi in optionsItems)
                    {
                        mi.Draw(spriteBatch);
                    }
                }
            }
        }
    }
}
