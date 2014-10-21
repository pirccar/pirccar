using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace RCClient.ResolutionIndependence
{
    class ResolutionIndependentRenderer
    {
        private readonly Game game;
        private Viewport viewPort;
        private float ratioX;
        private float ratioY;
        private Vector2 virutalMousePosition = new Vector2();
        public Color backgroundColor = Color.Black;

        public int virtualHeight;
        public int virtualWidth;
        public int screenWidth;
        public int screenHeight;

        public bool renderingToScreenIsFinished;
        private static Matrix scaleMatrix;
        private bool dirtyMatrix = true;

        public ResolutionIndependentRenderer(Game game)
        {
            this.game = game;
            virtualWidth = 1600;
            virtualHeight = 768;

            screenWidth = 1600;
            screenHeight = 768;
        }

        public void Initialize()
        { 
            SetupVirtualScreenViewport();
            ratioX = (float)viewPort.Width / virtualWidth;
            ratioY = (float)viewPort.Height / virtualHeight;
            dirtyMatrix = true;
        }

        public void SetupFullViewport()
        { 
            var vp = new Viewport();
            vp.X = vp.Y = 0;
            vp.Width = screenWidth;
            vp.Height = screenHeight;
            game.GraphicsDevice.Viewport = vp;
            dirtyMatrix = true;
        }

        public void BeginDraw()
        { 
            SetupFullViewport();
            game.GraphicsDevice.Clear(backgroundColor);
            SetupVirtualScreenViewport();
        }

        public Matrix GetTransformationMatrix()
        {
            if(dirtyMatrix)
                RecreateScaleMatrix();

            return scaleMatrix;
        }

        private void RecreateScaleMatrix()
        {
            Matrix.CreateScale((float)screenWidth / virtualWidth, (float)screenHeight / virtualHeight, 1f, out scaleMatrix);
            dirtyMatrix = false;
        }

        public Vector2 ScaleMouseToScreenCordinates(Vector2 screenPosition)
        {
            var realX = screenPosition.X - viewPort.X;
            var realY = screenPosition.Y - viewPort.Y;
            virutalMousePosition.X = realX / ratioX;
            virutalMousePosition.Y = realY / ratioY;

            return virutalMousePosition;
        }

        public void SetupVirtualScreenViewport()
        {
            var targetAspectRatio = virtualWidth / (float)virtualHeight;
            var width = screenWidth;
            var height = (int)(width / targetAspectRatio + .5f);

            if(height > screenHeight)
            {
                height = screenHeight;

                width = (int)(height * targetAspectRatio + .5f);
            }

            viewPort = new Viewport
            {
                X = (screenWidth / 2) - (width / 2),
                Y = (screenHeight / 2) - (height / 2),
                Width = width,
                Height = height
            };
            game.GraphicsDevice.Viewport = viewPort;
        }
    }
}
