using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

/* This class represents a meter gauge
 * For now it's used as a analog speedmeter
 * and one for torque
 * */

namespace RCClient
{
    class Meter
    {
        Texture2D meter;
        Texture2D meterStick;
        float min, max, value;
        Vector2 position;
        String text;
        SpriteFont font;

        public Meter(float min, float max)
        {
            this.min = min;
            this.max = max;
            value = (min + max) / 2.0f;
            meter = null;
            meterStick = null;
            position = Vector2.Zero;
            font = null;
            text = null;
        }

        public void SetTextures(Texture2D meter, Texture2D stick)
        {
            this.meter = meter;
            this.meterStick = stick;
        }

        public void SetMin(float min)
        {
            this.min = min;
        }

        public void SetMax(float max)
        {
            this.max = max;
        }

        public void SetValue(float value)
        {
            this.value = value;
        }

        public void SetPosition(Vector2 pos)
        {
            this.position = pos;
        }

        public void SetText(String text)
        {
            this.text = text;
        }

        public void SetFont(SpriteFont font)
        {
            this.font = font;
        }

        public void Draw(SpriteBatch spriteBatch)
        {
            if (meter != null && meterStick != null)
            {
                spriteBatch.Draw(meter, new Rectangle((int)position.X, (int)position.Y, meter.Width, meter.Height), Color.White);
                float angle = 0.0f;
                if (value <= min)
                    angle = -MathHelper.PiOver2;
                else if (value >= max)
                    angle = MathHelper.PiOver2;
                else
                {
                    float mid = (max + min) / 2;
                    if (value < mid)
                    {
                        float range = mid - min;
                        float calcVal = mid - value;
                        float percent = calcVal / range;
                        percent = MathHelper.Clamp(percent, 0.0f, 1.0f);
                        angle = -MathHelper.PiOver2 * percent;
                    }
                    else
                    {
                        float range = max - mid;
                        float calcVal = max - value;
                        float percent = calcVal / range;
                        percent = MathHelper.Clamp(percent, 0.0f, 1.0f);
                        percent = 1.0f - percent;
                        angle = MathHelper.PiOver2 * percent;
                    }
                }
                if (text != null && font != null)
                {
                    Vector2 textPos = font.MeasureString(text);
                    textPos.X /= 2;
                    Vector2 target = new Vector2(position.X + meter.Width / 2, position.Y + meter.Height / 2);
                    target.X -= textPos.X;
                    //spriteBatch.DrawString(font, text, new Vector2(position.X + meter.Width / 4, position.Y + meter.Height / 2), Color.White);
                    spriteBatch.DrawString(font, text, target, Color.White);
                }
                spriteBatch.Draw(meterStick, new Rectangle((int)position.X + meter.Width / 2, (int)position.Y + meter.Height, meterStick.Width, meterStick.Height), null, Color.White, angle, new Vector2(meterStick.Width / 2, meterStick.Height), SpriteEffects.None, 0.0f);
            }
        }
    }
}
