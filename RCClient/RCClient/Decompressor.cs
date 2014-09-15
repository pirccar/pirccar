using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Threading;
using System.Runtime.InteropServices;
using System.Windows.Media.Imaging;

/*
This class is used to compress and decompress jpegs and png images. 
*/
namespace RCClient
{
    class Decompressor
    {
        int width;
        int height;
        MemoryStream stream;
        public Decompressor()
        {
            stream = new MemoryStream();
        }

        public int getWidth()
        {
            return width;
        }

        public int getHeight()
        {
            return height;
        }

        public Byte[] Decompress(Byte[] arr, bool flipRB)
        {
            Byte[] output;
            try
            {
                using (Image image = Image.FromStream(new MemoryStream(arr)))
                {
                    /* OLD AND SLOW
                     * Bitmap bitmap = new Bitmap(image);
                    output = new Byte[bitmap.Width * bitmap.Height * 4];
                    int pixel = 0;
                    for (int y = 0; y < bitmap.Height; y++)
                    {
                        for (int x = 0; x < bitmap.Width; x++)
                        {
                            Color c = bitmap.GetPixel(x, y);
                            output[pixel] = c.R;
                            output[pixel + 1] = c.G;
                            output[pixel + 2] = c.B;
                            output[pixel + 3] = 255;
                            pixel += 4;
                        }
                    }*/

                    /* FASTER BUT BGR
                    Bitmap bitmap = new Bitmap(image);
                    BitmapData data = bitmap.LockBits(new Rectangle(0, 0, bitmap.Width, bitmap.Height), ImageLockMode.ReadOnly, bitmap.PixelFormat);
                    try
                    {
                        IntPtr ptr = data.Scan0;
                        
                        int bytes = Math.Abs(data.Stride) * bitmap.Height;
                        output = new Byte[bytes];
                        Marshal.Copy(ptr, output, 0, bytes);
                    }
                    finally
                    {
                        bitmap.UnlockBits(data);
                    }*/

                    
                    //FAST RGB, Pointer in C# = unsafe
                    unsafe
                    {
                        Bitmap bitmap = new Bitmap(image);
                        BitmapData data = bitmap.LockBits(new Rectangle(0, 0, bitmap.Width, bitmap.Height), ImageLockMode.ReadOnly, bitmap.PixelFormat);
                        uint* ptr = (uint*)data.Scan0;

                        if (flipRB)
                        {
                            for (int i = 0; i < bitmap.Width * bitmap.Height; i++)
                            {
                                ptr[i] = (ptr[i] & 0x000000ff) << 16 | (ptr[i] & 0x0000FF00) | (ptr[i] & 0x00FF0000) >> 16 | (ptr[i] & 0xFF000000);
                            }
                        }

                        width = bitmap.Width;
                        height = bitmap.Height;
                        int bytes = Math.Abs(data.Stride) * bitmap.Height;
                        output = new Byte[bytes];
                        Marshal.Copy(data.Scan0, output, 0, bytes);

                        ptr = null;
                        bitmap.UnlockBits(data);
                    }
                }
            }
            catch (Exception e)
            {
                output = null;
            }

            return output;
        }

        public uint[] DecompressToIntArray(Byte[] arr, bool flipRB)
        {
            uint[] output;
            try
            {
                using (Image image = Image.FromStream(new MemoryStream(arr)))
                {
                    unsafe
                    {
                        Bitmap bitmap = new Bitmap(image);
                        BitmapData data = bitmap.LockBits(new Rectangle(0, 0, bitmap.Width, bitmap.Height), ImageLockMode.ReadOnly, bitmap.PixelFormat);
                        uint* ptr = (uint*)data.Scan0;
                        output = new uint[bitmap.Width * bitmap.Height];
                        if (flipRB)
                        {
                            for (int i = 0; i < bitmap.Width * bitmap.Height; i++)
                            {
                                output[i] = (ptr[i] & 0x000000ff) << 16 | (ptr[i] & 0x0000FF00) | (ptr[i] & 0x00FF0000) >> 16 | (ptr[i] & 0xFF000000);
                            }
                        }

                        width = bitmap.Width;
                        height = bitmap.Height;
                       
                        //Marshal.Copy(data.Scan0, output, 0, bitmap.Width * bitmap.Height);

                        ptr = null;
                        bitmap.UnlockBits(data);
                    }
                }
            }
            catch (Exception e)
            {
                output = null;
            }

            return output;
        }

        public byte[] Encode(byte[] raw, int width, int height, System.Windows.Media.PixelFormat pf)
        {
            int stride = width / (pf.BitsPerPixel / 4);
            byte[] output = new byte[height * stride];
            
            using (MemoryStream ms = new MemoryStream())
            {
                JpegBitmapEncoder encoder = new JpegBitmapEncoder();
                BitmapSource image = BitmapSource.Create(width, height, 96.0, 96.0, pf, null, raw, pf.BitsPerPixel * width / (pf.BitsPerPixel / 4));
                encoder.QualityLevel = 100;
                encoder.Frames.Add(BitmapFrame.Create(image));
                encoder.Save(ms);
                
                output = ms.ToArray();
            }
            return output;
        }

        public byte[] EncodePNG(byte[] raw, int width, int height)
        {
            using (MemoryStream ms = new MemoryStream())
            {
                PngBitmapEncoder encoder = new PngBitmapEncoder();
                encoder.Interlace = PngInterlaceOption.On;
                encoder.Frames.Add(BitmapFrame.Create(BitmapSource.Create(width, height, 96, 96, System.Windows.Media.PixelFormats.Bgra32, null, raw, width * System.Windows.Media.PixelFormats.Bgra32.BitsPerPixel / 8)));
                encoder.Save(ms);
                return ms.ToArray();
            }
        }
    }
}
