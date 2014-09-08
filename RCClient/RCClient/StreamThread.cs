using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework.Graphics;

namespace RCClient
{
    class StreamThread
    {
        volatile bool run = false;
        bool newData = false;
        Texture2D texture;
        int width, height;
        Decompressor decompressor;
        bool encoding = false;

        public StreamThread(int width, int height)
        {
            run = true;
            texture = null;
            this.width = width;
            this.height = height;
            decompressor = new Decompressor();
        }

        public void Run()
        {
            while (run)
            {
                if (newData)
                {
                    encoding = true;
                    Byte[] raw = new Byte[width * height * 4];
                    texture.GetData<Byte>(raw);
                    encoding = false;

                    Byte[] png = decompressor.EncodePNG(raw, width, height);
                    Byte[] jpeg = decompressor.Encode(raw, width, height);
                    newData = false;
                }
            }
        }

        public bool SetTexture(Texture2D indata)
        {
            if(encoding)
                return false;

            texture = indata;
            newData = true;

            return true;
        }

        public void Kill()
        {
            run = false;
        }
    }
}
