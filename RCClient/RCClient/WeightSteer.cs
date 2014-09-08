using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;

namespace RCClient
{
    static class WeightSteer
    {
        public static float Weight(float value, float low, float high)
        {
            if (value < low)
            {
                float percent = value / low;
                value = value * 0.15f * percent;
            }
            else if (value > low && value < high)
            {
                float range = high - low;
                float percent = value / range;
                value = value * 0.25f * percent;
            }
            else 
            {
                float range = 1 - high;
                float percent = value / range;
                value = value * 0.6f * percent;
            }

            return value;
        }
    }
}
