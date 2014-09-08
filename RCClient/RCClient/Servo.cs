using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace RCClient
{
    enum ServoType
    { 
        Unknown,
        Steering,
        Throttle,
        Camera,
        Gear

    };

    //representation of a PWM servo
    class Servo
    {
        int id; //id mapping to physical port on PCA9685
        float value;
        float min;
        float max;
        ServoType type;

        public Servo(int id)
        {
            this.id = id;
            value = 0.0f;
            type = ServoType.Unknown;
            min = 130.0f;
            max = 470.0f;
        }

        public void setValue(float value)
        {
            if (value > max)
                value = max;
            else if (value < min)
                value = min;
            this.value = value;
        }

        public void setType(ServoType type)
        {
            this.type = type;

            switch (type)
            { 
                case ServoType.Camera :
                    min = 320.0f - 150.0f;
                    max = 320.0f + 150.0f;
                    break;

                case ServoType.Gear :
                    min = 130.0f;
                    max = 470.0f;
                    break;

                case ServoType.Steering:
                    min = 245.0f;
                    max = 395.0f;
                    break;

                case ServoType.Throttle:
                    min = 180.0f;
                    max = 400.0f;
                    break;

                case ServoType.Unknown:
                    min = 130.0f;
                    max = 470.0f;
                    break;
            }
        }

        public void setOff()
        {
            value = min;
        }

        public void setMid()
        {
            value = 320.0f;
        }

        public void setOn()
        {
            value = max;
        }

        public int getId()
        {
            return id;
        }

        public float getMin()
        {
            return min;
        }

        public float getMax()
        {
            return max;
        }

        public float getValue()
        {
            return value;
        }

        //returns a sendable string containing ID and its value, ready to be sent to server
        public String getMessage()
        {
            String msg = "S";
            String s = String.Format("{0:00}", id);
            String v = String.Format("{0:0000}", value);
            msg += s + v;

            return msg;
        }
    }
}
