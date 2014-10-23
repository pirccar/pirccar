using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace CarSimulator
{
    enum ServoType
    {
        steering,
        throttle,
        gear
    }
    class Servo
    {
        int channel;
        int value;
        ServoType type;

        public Servo(int channel, int value, ServoType type)
        {
            this.channel = channel;
            this.value = value;
            this.type = type;
        }
        public Servo(Servo other)
        {
            channel = other.channel;
            value = other.value;
            type = other.type;
        }

        public void SetValue(int value)
        {
            this.value = value;
        }

        public int GetValue()
        {
            return value;
        }
    }
}
