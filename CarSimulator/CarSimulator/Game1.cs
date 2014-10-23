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
    /// <summary>
    /// This is the main type for your game
    /// </summary>
    public class Game1 : Game
    {
        GraphicsDeviceManager graphics;
        SpriteBatch spriteBatch;
        Input input;
        Car car;
        bool gear;
        Texture2D basicTexture;
        SpriteFont font;
        Vector2 target;

        public Game1()
            : base()
        {
            graphics = new GraphicsDeviceManager(this);
            Content.RootDirectory = "Content";
        }

        /// <summary>
        /// Allows the game to perform any initialization it needs to before starting to run.
        /// This is where it can query for any required services and load any non-graphic
        /// related content.  Calling base.Initialize will enumerate through any components
        /// and initialize them as well.
        /// </summary>
        protected override void Initialize()
        {
            // TODO: Add your initialization logic here
            graphics.PreferredBackBufferWidth = 1000;
            graphics.ApplyChanges();
            input = new Input();
            gear = true;
            basicTexture = new Texture2D(GraphicsDevice, 1, 1);
            basicTexture.SetData(new Color[] {Color.White});
            car = new Car(new Vector2(200, 300) ,basicTexture);
            target = new Vector2(600, 100);
            car.SetGotoTarget(target);
            base.Initialize();
        }

        /// <summary>
        /// LoadContent will be called once per game and is the place to load
        /// all of your content.
        /// </summary>
        protected override void LoadContent()
        {
            // Create a new SpriteBatch, which can be used to draw textures.
            spriteBatch = new SpriteBatch(GraphicsDevice);
            font = Content.Load<SpriteFont>("SpriteFont1");

            // TODO: use this.Content to load your game content here
        }

        /// <summary>
        /// UnloadContent will be called once per game and is the place to unload
        /// all content.
        /// </summary>
        protected override void UnloadContent()
        {
            // TODO: Unload any non ContentManager content here
        }

        /// <summary>
        /// Allows the game to run logic such as updating the world,
        /// checking for collisions, gathering input, and playing audio.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
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

            if (input.buttonDown(Buttons.LeftShoulder))
                car.ToggleRealWorld();

            if (input.buttonDown(Buttons.Back))
                car.ToggleAutonomous();

            car.Update(gameTime);

            base.Update(gameTime);
        }

        /// <summary>
        /// This is called when the game should draw itself.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Draw(GameTime gameTime)
        {
            GraphicsDevice.Clear(Color.CornflowerBlue);


            spriteBatch.Begin();
            spriteBatch.Draw(basicTexture, new Rectangle(200 - 25, 300 - 20, 50, 40), Color.Blue);
            spriteBatch.Draw(basicTexture, new Rectangle((int)target.X - 25, (int)target.Y - 20, 50, 40), Color.Green);
            car.Draw(spriteBatch);
            spriteBatch.DrawString(font, Math.Round(car.GetPosition().X, 2) + ":" + Math.Round(car.GetPosition().Y, 2), new Vector2(5, 5), Color.Black);
            spriteBatch.DrawString(font, Math.Round(car.GetSteering(), 2).ToString(), new Vector2(650, 5), Color.Black);
            spriteBatch.DrawString(font, Math.Round(car.GetSpeed(),1).ToString(), new Vector2(750, 5), Color.Black);
            spriteBatch.End();
            // TODO: Add your drawing code here

            base.Draw(gameTime);
        }
    }
}
