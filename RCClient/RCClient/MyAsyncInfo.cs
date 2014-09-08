using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace RCClient
{
    class MyAsyncInfo
    {
        public Byte[] ByteArray { get; set; }
        public int nRead { get; set; }

        public MyAsyncInfo(Byte[] array)
        {
            ByteArray = array;
            nRead = 0;
        }
    }
}
