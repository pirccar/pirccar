#region Using Statements
using System;
using System.Collections.Generic;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using Microsoft.Xna.Framework.Storage;
using Microsoft.Xna.Framework.GamerServices;
#endregion

namespace CarSimulator
{
    public class Game1 : Game
    {
        GraphicsDeviceManager graphics;
        SpriteBatch spriteBatch;
        Input input;
        Car car;
        Vector2 spawnPos;
        bool gear;
        Texture2D basicTexture;
        SpriteFont font;
        List<Vector2> targets;
        double startTime;
        double currentTime;

        public Game1()
            : base()
        {
            graphics = new GraphicsDeviceManager(this);
            Content.RootDirectory = "Content";
        }

        protected override void Initialize()
        {
            Window.Position = new Point(10, 10);
            graphics.PreferredBackBufferWidth = 1600;
            graphics.PreferredBackBufferHeight = 1000;
            graphics.ApplyChanges();
            input = new Input();
            gear = true;
            //spawnPos = new Vector2(150, 400); //trippleRoom spawn
            spawnPos = new Vector2(150, 800);
            basicTexture = new Texture2D(GraphicsDevice, 1, 1);
            basicTexture.SetData(new Color[] {Color.White});
            car = new Car(spawnPos, basicTexture);
            targets  = new List<Vector2>();
            targets.Add(new Vector2(1000, 150)); //s curve
            //targets.Add(new Vector2(150, 800));
            startTime = -1;
            currentTime = -1;
            
            for (int i = 0; i < targets.Count; i++)
            {
                car.AddGotoTarget(targets[i]);
            }
            base.Initialize();
        }

        protected override void LoadContent()
        {
            // Create a new SpriteBatch, which can be used to draw textures.
            spriteBatch = new SpriteBatch(GraphicsDevice);
            font = Content.Load<SpriteFont>("SpriteFont1");
        }

        protected override void UnloadContent()
        {
        }

        protected override void Update(GameTime gameTime)
        {
            if (Keyboard.GetState().IsKeyDown(Keys.Escape))
                Exit();

            input.update();
            float z = input.getRightTrigger(); //used for throttle

            float lx = input.getLeftStick().X; //used for steering

            car.SetServo(0, (int)MathHelper.Clamp(320.0f + ((lx * -1) * 48.0f), 130, 470));
            car.SetServo(1, (int)MathHelper.Lerp(180, 195, z));

            if (input.buttonDown(Buttons.A))
            {
                gear = !gear;
                if (gear)
                    car.SetServo(2, 0);
                else
                    car.SetServo(2, 470);
            }

            if (input.buttonDown(Buttons.Y))
                car.GoHome();

            if (input.buttonDown(Buttons.B))
                car.Reset();

            if (input.buttonDown(Buttons.LeftShoulder) || input.keyDown(Keys.Space))
                car.ToggleRealWorld();

            if (input.buttonDown(Buttons.Back) || input.keyDown(Keys.A))
                car.ToggleAutonomous();

            car.Update((float)gameTime.ElapsedGameTime.TotalSeconds);

            if (startTime == -1 && car.IsAutonomous())
                startTime = gameTime.TotalGameTime.TotalSeconds;
            else if(car.IsAutonomous() && car.TargetCount() > 0)
            {
                currentTime = gameTime.TotalGameTime.TotalSeconds - startTime;
            }

            base.Update(gameTime);
        }

        protected override void Draw(GameTime gameTime)
        {
            GraphicsDevice.Clear(Color.CornflowerBlue);


            spriteBatch.Begin();
            spriteBatch.Draw(basicTexture, new Rectangle((int)spawnPos.X - 25, (int)spawnPos.Y - 20, 50, 40), Color.Blue);
            for (int i = 0; i < targets.Count; i++)
            {
                spriteBatch.Draw(basicTexture, new Rectangle((int)targets[i].X - 25, (int)targets[i].Y - 20, 50, 40), Color.Green);
            }
            
            car.Draw(spriteBatch);
            spriteBatch.DrawString(font, Math.Round(car.GetPosition().X, 2) + ":" + Math.Round(car.GetPosition().Y, 2), new Vector2(5, 5), Color.Black);
            spriteBatch.DrawString(font, Math.Round(car.GetTravelDistance(), 2).ToString(), new Vector2(250, 5), Color.Black);
            spriteBatch.DrawString(font, car.GetVirtualLineCount().ToString(), new Vector2(400, 5), Color.Black);
            spriteBatch.DrawString(font, Math.Round(car.GetSteering(), 2).ToString(), new Vector2(650, 5), Color.Black);
            spriteBatch.DrawString(font, Math.Round(car.GetSpeed(),1).ToString(), new Vector2(750, 5), Color.Black);
            spriteBatch.DrawString(font, Math.Round(MathHelper.ToDegrees(car.GetRotation()), 1).ToString(), new Vector2(550, 5), Color.Black);
            spriteBatch.DrawString(font, car.GetAcceleration().ToString(), new Vector2(850, 5), Color.Black);
            spriteBatch.DrawString(font, currentTime.ToString(), new Vector2(1200, 5), Color.Black);
            spriteBatch.End();

            base.Draw(gameTime);
        }
    }
}
