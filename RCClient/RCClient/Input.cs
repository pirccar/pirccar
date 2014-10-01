using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Input;

namespace RCClient
{

    class Input
    {
        KeyboardState currentKeyState;
        KeyboardState previousKeyState;
        GamePadState currentPadState;
        GamePadState previousPadState;


        public bool keyDown(Keys key)
        {
            return (!previousKeyState.IsKeyDown(key) && currentKeyState.IsKeyDown(key));
        }

        public bool keyUp(Keys key)
        {
            return (previousKeyState.IsKeyDown(key) && !currentKeyState.IsKeyDown(key));
        }

        public bool keyHeld(Keys key)
        {
            return (previousKeyState.IsKeyDown(key) && currentKeyState.IsKeyDown(key));
        }

        public bool buttonDown(Buttons button)
        {
            return (!previousPadState.IsButtonDown(button) && currentPadState.IsButtonDown(button));
        }

        public bool buttonUp(Buttons button)
        {
            return (previousPadState.IsButtonDown(button) && !currentPadState.IsButtonDown(button));
        }

        public bool buttonHeld(Buttons button)
        {
            return (previousPadState.IsButtonDown(button) && currentPadState.IsButtonDown(button));
        }

        public Vector2 getLeftStick()
        {
            return currentPadState.ThumbSticks.Left;
        }

        public Vector2 getRightStick()
        {
            return currentPadState.ThumbSticks.Right;
        }

        public float getLeftTrigger()
        {
            return currentPadState.Triggers.Left;
        }

        public float getRightTrigger()
        {
            return currentPadState.Triggers.Right;
        }

        public void update()
        {
            previousKeyState = currentKeyState;
            currentKeyState = Keyboard.GetState();

            previousPadState = currentPadState;
            currentPadState = GamePad.GetState(Microsoft.Xna.Framework.PlayerIndex.One);
        }
    }
}
