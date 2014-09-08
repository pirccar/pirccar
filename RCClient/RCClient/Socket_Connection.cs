using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.IO;
using ZLibNet;

namespace RCClient
{
    class Socket_Connection
    {
        private TcpClient client;
        private Stream stream;
        private byte[] result;
        private int BUFFER_SIZE = 64*64*4;

        private UdpClient udpClient;
        IPEndPoint groupEP;
        private IPEndPoint localEndPoint;

        private static ManualResetEvent readDone = new ManualResetEvent(false);

        public Socket_Connection(int width, int height){
            IPAddress[] adresses = Dns.GetHostEntry(Dns.GetHostName()).AddressList;
            IPAddress local = null;
            for(int i = 0; i < adresses.Length; i++)
            {
                if(adresses[i].AddressFamily == AddressFamily.InterNetwork)
                    local = adresses[i];
            } 
            localEndPoint = new IPEndPoint(local, 0);
            client = new TcpClient(localEndPoint);
            client.ReceiveBufferSize = 10000000;
            BUFFER_SIZE = width * height * 4;
        }

        private void selectCorrectLocal(String ip)
        {
            IPAddress[] adresses = Dns.GetHostEntry(Dns.GetHostName()).AddressList;
            IPAddress local = null;
            for (int i = 0; i < adresses.Length; i++)
            {
                if (adresses[i].AddressFamily == AddressFamily.InterNetwork)
                {
                    if (ip.Contains("25.") && adresses[i].ToString().Contains("25."))
                        local = adresses[i];
                    else if (!ip.Contains("25.") && !adresses[i].ToString().Contains("25."))
                        local = adresses[i];
                }
            }

            localEndPoint = new IPEndPoint(local, 0);
        }
        
        //connect to ip at port, TCP connection
        public void connect(String ip, int port)
        {
            if (!client.Connected)
            {
                try
                {
                    client.Close();
                    selectCorrectLocal(ip);
                    client = new TcpClient(localEndPoint);
                    client.ReceiveBufferSize = 10000000;
                    client.Connect(ip, port);
                    result = new byte[BUFFER_SIZE];
                    stream = client.GetStream();
                    stream.WriteTimeout = 250;
                }
                catch (Exception e)
                {
                    System.Windows.Forms.MessageBox.Show("Couldn't connect to server", "Connection error");
                }
            }
        }

        public void localConnect(int port)
        {
            if (!client.Connected)
            {
                try
                {
                    client.Close();
                    IPAddress adress = IPAddress.Parse("127.0.0.1");
                    localEndPoint = new IPEndPoint(adress, 0);
                    client = new TcpClient(localEndPoint);
                    client.ReceiveBufferSize = 10000000;
                    client.Connect("127.0.0.1", port);
                    result = new byte[BUFFER_SIZE];
                    stream = client.GetStream();
                    stream.WriteTimeout = 2500;
                }
                catch (Exception e)
                {
                    System.Windows.Forms.MessageBox.Show("Couldn't connect to server, local", "Connection error");
                }
            }
        }

        //opens UDP connection at port
        public void udpConnect(int port)
        {

            udpClient = new UdpClient(port);
            udpClient.Client.ReceiveTimeout = 1000;
            groupEP = new IPEndPoint(IPAddress.Any, port);
        }

        public void bruteConnect(String ip, int port)
        {
            try
            {
                client.Connect(ip, port);
                result = new byte[BUFFER_SIZE];
                stream = client.GetStream();
                stream.WriteTimeout = 250;

            }
            catch (Exception e)
            {
                //Handle
            }
        }

        //send data
        public void send(String msg)
        {
            try
            {
                byte[] ba = Encoding.ASCII.GetBytes(msg);
                stream.Write(ba, 0, ba.Length);
            }
            catch (Exception e)
            {
                String s = e.Message;
            }
        }

        //send servo information
        public void send(int servo, int value)
        {
            String msg = "S";
            String s = String.Format("{0:00}", servo);
            String v = String.Format("{0:0000}", value);
            msg += s + v;

            byte[] ba = Encoding.ASCII.GetBytes(msg);
            stream.Write(ba, 0, ba.Length);
        }

        //send ping
        public void sendPing()
        {
            String msg = "P";
            byte[] ba = Encoding.ASCII.GetBytes(msg);
            stream.Write(ba, 0, ba.Length);
        }

        //request frame, deprecated
        public void requestFrame()
        {
            String msg = "F";
            byte[] ba = Encoding.ASCII.GetBytes(msg);
            stream.Write(ba, 0, ba.Length);
        }

        //read unknown size
        public String read()
        {
            int len = 0;
            byte[] rec = new byte[32];
            len = stream.Read(rec, 0, 32);
            return Convert.ToString(rec);
        }

        //read a frame of size texW*texH*Channels
        public byte[] readFrame()
        {
            byte[] rec = new byte[BUFFER_SIZE];
            byte[] send = null;
            try
            {
                int len = stream.Read(rec, 0, BUFFER_SIZE);
                send = new byte[len];
                Array.Copy(rec, send, len);
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }

            return send;
        }

        public byte[] readFrameUDP()
        {
            byte[] rec = null;
            try
            {
                rec = udpClient.Receive(ref groupEP);
            }
            catch (Exception e)
            { 
                
            }
            return rec;
        }

        public void udpDisconnect()
        {
            udpClient.Close();
        }

        public bool isConnected()
        {
            return client.Connected;
        }

        public void disconnect()
        {
            client.Close();
        }
    }
}
