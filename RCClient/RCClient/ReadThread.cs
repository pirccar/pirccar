using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

/*This thread is used for reading udp messages on a socket
 * */

namespace RCClient
{

    //Thread for reading from a socket
    class ReadThread
    {
        Socket_Connection connection;
        byte[] data;
        private int BUFFER_SIZE = 64*64*4;
        bool dataReady;
        volatile bool run = true;
        volatile bool connected = false;
        int width;
        int height;
        bool isUdp;
        String ip;

        public ReadThread(String ip, int width, int height, bool isUdp = true)
        {
            this.ip = ip;
            this.width = width;
            this.height = height;
            this.isUdp = isUdp;
            BUFFER_SIZE = width * height * 4;
        }

        public void Run()
        {
            //create buffer and connection
            data = new byte[BUFFER_SIZE];
            connection = new Socket_Connection(width, height);

            do
            {
                //connect
                //connection.connect(ip, 8002);
                if (isUdp)
                    connection.udpConnect(8002);
                else
                    connection.tcpListen(ip, 8002);
                connected = true;

                //while (connection.isConnected() && connected)
                while (connected)
                {
                    //mutex
                    if (!dataReady)
                    {
                        //data = connection.readFrame();
                        data = connection.readFrame();
                        if (data != null && data.Length > 0)
                            dataReady = true;
                    }
                }

            } while (run);

            connection.disconnect();
        }

        //stop the connection
        public void kill()
        {
            /*if(connection.isConnected())
                connection.disconnect();*/

            connected = false;
            run = false;
        }

        public bool isDataReady()
        {
            return dataReady;
        }

        //get the data, sets mutex
        public byte[] getData()
        {
            dataReady = false;
            return data;
        }
    }
}
