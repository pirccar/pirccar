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
        int value;
        int min;
        int max;
        int trim;
        ServoType type;

        public Servo(int id)
        {
            this.id = id;
            value = 0;
            type = ServoType.Unknown;
            min = 130;
            max = 470;
            trim = 0;
        }

        public void setValue(int value)
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
                    min = 320 - 150;
                    max = 320 + 150;
                    break;

                case ServoType.Gear :
                    min = 130;
                    max = 470;
                    break;

                case ServoType.Steering:
                    min = 245;
                    max = 395;
                    break;

                case ServoType.Throttle:
                    min = 180;
                    max = 400;
                    break;

                case ServoType.Unknown:
                    min = 130;
                    max = 470;
                    break;
            }
        }

        public void setOff()
        {
            value = min;
        }

        public void setMid()
        {
            value = 320;
        }

        public void setOn()
        {
            value = max;
        }

        public int getId()
        {
            return id;
        }

        public int getMin()
        {
            return min;
        }

        public int getMax()
        {
            return max;
        }

        public int getValue()
        {
            return value + trim;
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

        public void setTrim(int trim)
        {
            this.trim = trim;
        }

        public int getTrim()
        {
            return trim;
        }
    }
}
