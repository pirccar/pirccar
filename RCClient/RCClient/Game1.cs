#region Using Statements
using System;
using System.Collections.Generic;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using Microsoft.Xna.Framework.Storage;
using Microsoft.Xna.Framework.GamerServices;
using System.Net;
using System.Threading;
using System.IO;
using Forms = System.Windows.Forms;
using Drawing = System.Drawing;
using RCClient.Menus;

#endregion



namespace RCClient
{
    /// <summary>
    /// This is the main type for your game
    /// </summary>
    public class Game1 : Game
    {
        //Standard XNA 
        GraphicsDeviceManager graphics;
        SpriteBatch spriteBatch;
        Socket_Connection connection;
        SpriteFont font;
        KeyboardState prevState;
        GamePadState prevPadState;
        
        //Map
        bool useBrowser = false;
        Forms.WebBrowser browser;
        bool browser_Initialized = false;
        Drawing.Bitmap formsBuffer;
        Texture2D map;
        bool updateMap = false;
        bool renderMap = false;
        bool alwaysRenderMap = true;
        String prevLatlng;
        float gpsSpeed;

        //Resolutions
        int screen_width = 1600;
        int screen_height = 768;
        int fpv_texture_width = 256;
        int fpv_texture_height = 256;
        int imageQuality = 75;
        int map_width = 256;
        int map_height = 256;

        //servo channels
        int cameraXChannel = 0;
        int cameraYChannel = 1;
        int steeringFChannel = 4;
        int steeringBChannel = 5;
        int throttleFChannel = 2;
        int throttleBChannel = 3;
        int gearChannel = 7;
        SelectBoxHandler servoBoxes;

        Servo[] servos;
        Servo[] prevServos;

        Texture2D ipTexture;
        IpInputBox ipInputbox;

        //connection and mesurements
        //String ip = "25.88.95.6";
        String ip = "172.26.101.210";
        

        //HERERERERE
        Texture2D testTexture;
        
        //FPS MESSUREMENT
        int nRec = 0;
        int framesPSec = 0;
        double recTime = 0.0;
        
        //control values
        float x;
        float y;
        float z;
        float lx;
        int speedLimiter = 0;
        float maxSpeed = 190.0f;
        bool gear = false;
        bool serverGear = false;
        bool strafe = false;
        bool stabilized = false;
        float xmin = 1;
        float[] lxmean;
        int lxindex = 0;
        int lxmeansize = 10;

        bool printDebug = false;

        //Torque Meter & speed
        Texture2D meter;
        Texture2D meterStick;
        Meter torqueMeter;
        Meter speedMeter;

        //Menu
        Menu menu;

        //Voltages
        int voltageIndex = 0;
        double[] battery0 = new double[20];
        double[] battery1 = new double[20];
        double[] amps0 = new double[20];
        double[] amps1 = new double[20];
        String batteryString0 = "";
        String batteryString1 = "";
        String batteryString2 = "";
        String batteryString3 = "";

        //Screenshot
        Texture2D screen;
        RenderTarget2D renderTarget;

        //Read thread
        ReadThread readThread;
        Thread thread;

        //Camera stream
        Texture2D camera;
        Decompressor decompressor;

        public Game1()
            : base()
        {
            graphics = new GraphicsDeviceManager(this);
            Content.RootDirectory = "Content";
        }

        //This is called when the program starts
        protected override void Initialize()
        {
            IsFixedTimeStep = true;
            //Window spawn location
            Window.SetPosition(new Point(10, 10));

            this.IsMouseVisible = true;

            //Init
            connection = new Socket_Connection(fpv_texture_width, fpv_texture_height);
            readThread = new ReadThread(ip, fpv_texture_width, fpv_texture_height, false);
            thread = new Thread(new ThreadStart(readThread.Run));

            servos = new Servo[16];
            prevServos = new Servo[16];
            camera = new Texture2D(graphics.GraphicsDevice, fpv_texture_width, fpv_texture_height, false, SurfaceFormat.Color);
            decompressor = new Decompressor();
            lxmean = new float[lxmeansize];

            //keyDispatcher = new KeyboardDispatcher(Window);
            ipInputbox = new IpInputBox();

            torqueMeter = new Meter(160.0f, maxSpeed);
            speedMeter = new Meter(0, 50.0f);
            gpsSpeed = 0.0f;

            //setup servos
            for (int i = 0; i < 16; i++)
                servos[i] = new Servo(i+1);

            for (int i = 0; i < 16; i++)
            {
                servos[i].setMid();
                prevServos[i] = new Servo(i + 1);
                prevServos[i].setValue(servos[i].getValue());
            }

            //Set camera servos, this will limit their movements
            servos[cameraXChannel].setType(ServoType.Camera);
            servos[cameraYChannel].setType(ServoType.Camera);
            servos[throttleBChannel].setType(ServoType.Throttle);
            servos[throttleFChannel].setType(ServoType.Throttle);
            servos[steeringFChannel].setType(ServoType.Steering);
            servos[steeringBChannel].setType(ServoType.Steering);

            servoBoxes = new SelectBoxHandler();
            servoBoxes.LoadDefaults();
            servoBoxes.Add(new Vector2(250, 300), 250, 45, true);
            servoBoxes.Add(new Vector2(250, 350), 250, 45, true);
            servoBoxes.Add(new Vector2(250, 400), 250, 45, true);
            servoBoxes.Add(new Vector2(250, 450), 250, 45, true);
            servoBoxes.Add(new Vector2(250, 500), 250, 45, true);
            servoBoxes.Add(new Vector2(250, 550), 250, 45, true);
            servoBoxes.Add(new Vector2(250, 600), 250, 45, true);
            servoBoxes.Add(new Vector2(250, 650), 250, 45, true);
            LoadServoConfig();

            //Browser, deprecated
            if (useBrowser)
            {
                browser = new Forms.WebBrowser();
                browser.Size = new Drawing.Size(map_width, map_height);
                browser.DocumentCompleted += new Forms.WebBrowserDocumentCompletedEventHandler(wb_DocumentCompleted);
                formsBuffer = new Drawing.Bitmap(map_width, map_height, Drawing.Imaging.PixelFormat.Format32bppArgb);
                SetMapURL("http://www.tritech.se");

                while (!browser_Initialized)
                {
                    Forms.Application.DoEvents();
                    Thread.Sleep(100);
                }
            }

            LoadConfig();

            menu = new Menu(new Vector2(screen_width / 2 - 200, screen_height / 2 - 200), screen_width, screen_height);


            //set window size
            SetGraphics();

            //streamThreadT.Start();
            base.Initialize();
        }

        void wb_DocumentCompleted(object sender, Forms.WebBrowserDocumentCompletedEventArgs e)
        {
            Console.WriteLine(browser.Document.Body.InnerHtml);
            browser_Initialized = true;
        }

        private void SetGraphics()
        {
            //graphics.ToggleFullScreen();
            graphics.PreferredBackBufferWidth = screen_width;
            graphics.PreferredBackBufferHeight = screen_height;
            graphics.ApplyChanges();
        }

        protected override void LoadContent()
        {
            //load a font
            
            spriteBatch = new SpriteBatch(GraphicsDevice);
            font = Content.Load<SpriteFont>("SpriteFont1");
            map = new Texture2D(GraphicsDevice, map_width, map_height, false, SurfaceFormat.Color);

            ipTexture = Content.Load<Texture2D>("textboxW");
            meter = Content.Load<Texture2D>("meter");
            meterStick = Content.Load<Texture2D>("meterStick");

            torqueMeter.SetTextures(meter, meterStick);
            torqueMeter.SetPosition(new Vector2(1024, screen_height/2 - 115));
            torqueMeter.SetFont(font);
            torqueMeter.SetText("Torque");

            speedMeter.SetTextures(meter, meterStick);
            speedMeter.SetPosition(new Vector2(1254, screen_height / 2 - 115));
            speedMeter.SetFont(font);
            speedMeter.SetText("Kmh");

            ipInputbox.SetFont(font);
            ipInputbox.SetTexture(ipTexture);
            ipInputbox.SetPosition(new Vector2(1024, 0));
            ipInputbox.SetWidth(576);
            ipInputbox.SetText(ip);

            servoBoxes.InitAll(ipTexture, font);

            menu.LoadContent(GraphicsDevice, Content);

            testTexture = new TextureColor(GraphicsDevice, Color.Red);

            PresentationParameters pp = GraphicsDevice.PresentationParameters;
            renderTarget = new RenderTarget2D(GraphicsDevice, pp.BackBufferWidth, pp.BackBufferHeight, false, GraphicsDevice.DisplayMode.Format, DepthFormat.None, pp.MultiSampleCount, RenderTargetUsage.DiscardContents);
        }

        protected override void UnloadContent()
        {
            // TODO: Unload any non ContentManager content here
        }

        //Deprecated
        private void SetMapURL(String url)
        {
            browser.Url = new Uri(url);
            browser.Navigate(url);
            updateMap = true;
        }

        //Deprecated
        private void CopyBitmapToTexture()
        {
            byte[] textureData = new byte[4 * map_width * map_height];

            System.Drawing.Imaging.BitmapData bmpData = formsBuffer.LockBits(new System.Drawing.Rectangle(0, 0, map_width, map_height), System.Drawing.Imaging.ImageLockMode.ReadOnly, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
            IntPtr safePtr = bmpData.Scan0;
            System.Runtime.InteropServices.Marshal.Copy(safePtr, textureData, 0, textureData.Length);
            formsBuffer.UnlockBits(bmpData);

            map.SetData<byte>(textureData);
        }

        //send client config to server
        private void sendConfig()
        {
            String throttleF = String.Format("{0:00}", throttleFChannel+1);
            String throttleB = String.Format("{0:00}", throttleBChannel + 1);
            String gear = String.Format("{0:00}", gearChannel+1);
            String width = String.Format("{0:0000}", fpv_texture_width);
            String height = String.Format("{0:0000}", fpv_texture_height);
            String quality = String.Format("{0:000}", imageQuality);
            String udp = String.Format("{0:0}", fpv_texture_width > 256 ? 0 : 1);
            String send = "C" + throttleF + throttleB + gear + width + height + quality + udp;
            connection.send(send);
        }

        //checks input in configboxes for servo channels
        private void parseConfigBoxes()
        {
            for (int i = 0; i < 8; i++)
            {
                String val = servoBoxes.GetBoxValue(i);

                if (val.CompareTo("Throttle Front") == 0)
                    throttleFChannel = i;
                else if (val.CompareTo("Throttle Back") == 0)
                    throttleBChannel = i;
                else if (val.CompareTo("Steering Front") == 0)
                    steeringFChannel = i;
                else if (val.CompareTo("Steering Back") == 0)
                    steeringBChannel = i;
                else if (val.CompareTo("Camera X") == 0)
                    cameraXChannel = i;
                else if (val.CompareTo("Camera Y") == 0)
                    cameraYChannel = i;
                else if (val.CompareTo("Gear Switch") == 0)
                    gearChannel = i;

            }
            SaveServoConfig();
        }

        //saves servoconfig to a file
        private void SaveServoConfig()
        {
            String output = "";
            for(int i = 0; i < 8; i++)
            {
                output += servoBoxes.GetBoxValue(i);
                output += "\r\n";
            }
            String path = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location) + "\\servoConfig.cfg";
            System.IO.File.WriteAllText(path, output); 
        }

        private void SaveConfig()
        {
            String output = "ip:" + ip + "\n";
            String path = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location) + "\\config.cfg";
            System.IO.File.WriteAllText(path, output);
        }

        //loads servoconfig from a file
        private void LoadServoConfig()
        { 
            String path = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location) + "\\servoConfig.cfg";
            String input;
            if (System.IO.File.Exists(path))
            {
                input = System.IO.File.ReadAllText(path);

                String[] lines = input.Split('\n');

                for (int i = 0; i < 8; i++)
                {
                    servoBoxes.SetBoxValue(i, lines[i].Remove(lines[i].IndexOf('\r')));
                }
                parseConfigBoxes();
            }
            else
            {
                SaveServoConfig();
                parseConfigBoxes();
            }
        }

        private void LoadConfig()
        {
            String path = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location) + "\\config.cfg";
            String input;
            if (System.IO.File.Exists(path))
            {
                input = System.IO.File.ReadAllText(path);
                String[] lines = input.Split('\n');

                for (int i = 0; i < lines.Length; i++)
                { 
                    if(lines[i].Contains("ip"))
                    {
                        String[] split = lines[i].Split(':');
                        if (split.Length == 2)
                        {
                            ip = split[1];
                            ipInputbox.SetText(ip);
                        }
                    }
                }
            }
        }

        //sends new input to the server if the input has changed
        private void sendAndAck(GameTime dt)
        {
            int nSent = 0;
            String msg = "";

            for (int i = 0; i < 16; i++)
            {
                if (i == gearChannel)
                {
                    msg += servos[i].getMessage();
                    nSent++;
                }
                else if (servos[i].getValue() != prevServos[i].getValue())
                {
                    msg += servos[i].getMessage();
                    nSent++;
                }
            }

            if (connection.isConnected())
            {
                if (nSent > 0 && stabilized)
                {
                    connection.send(msg);
                }
                else
                {
                    connection.send("P");
                }
            }

            for (int i = 0; i < 16; i++)
            {
                prevServos[i].setValue(servos[i].getValue());
            }
        }

        //checks if readframe has new data, if it has new data load it into texture
        private void parseIndata()
        {
            //new data on readthread
            if (readThread.isDataReady())
            {
                Byte[] data = readThread.getData(); //grab the data from readthread

                //check if the data is a voltage reading
                if (data.Length < 100 && data[0] == 'V')
                {
                    //for now just save the voltage reading locally
                    String str = System.Text.Encoding.ASCII.GetString(data); //convert byte[] to string
                    String[] arr = str.Split(':');

                    for (int i = 0; i < arr.Length; i++)
                    {
                        String channel = arr[i].Substring(1, 1); //extract the channel from the string, it is at index 1
                        String value = arr[i].Substring(2, arr[i].Length - 2); //get the rest of the data from index 2 to data.length - startIndex(2)
                        value = value.Replace('.', ','); //Double.Parse() expects ',' instead of '.' for doubles
                        double vol = Double.Parse(value);

                        //Channel 0
                        if (int.Parse(channel) == 0)
                        {
                            battery0[voltageIndex] = vol;
                            double sum = 0.0;
                            for (int j = 0; j < battery0.Length; j++)
                            {
                                sum += battery0[j];
                            }
                            sum /= battery0.Length;
                            batteryString0 = "Car Bat:" + Math.Round(sum * 14.19 / 3.256, 3);
                        }
                        //Channel 1
                        else if (int.Parse(channel) == 1)
                        {
                            amps0[voltageIndex] = vol;
                            double sum = 0.0;
                            for (int j = 0; j < amps0.Length; j++)
                            {
                                sum += amps0[j];
                            }
                            sum /= amps0.Length;
                            sum = Math.Round((sum / 0.025), 3);
                            batteryString1 = "Car Amps:" + sum;
                        }
                        //Channel 2
                        else if (int.Parse(channel) == 2)
                        {
                            battery1[voltageIndex] = vol;
                            double sum = 0.0;
                            for (int j = 0; j < battery1.Length; j++)
                            {
                                sum += battery1[j];
                            }
                            sum /= battery1.Length;
                            batteryString2 = "Pi Bat:" + Math.Round(sum * 14.19 / 3.256, 3);
                        }
                        //Channel 3
                        else if (int.Parse(channel) == 3)
                        {
                            amps1[voltageIndex] = vol;
                            double sum = 0.0;
                            for (int j = 0; j < amps1.Length; j++)
                            {
                                sum += amps1[j];
                            }
                            sum /= amps1.Length;
                            sum = Math.Round((sum / 0.025), 3);
                            batteryString3 = "Pi Amps:" + sum;
                        }
                    }

                    voltageIndex++;
                    if (voltageIndex >= 20)
                        voltageIndex = 0;
                } //Stabilized message from server
                else if (data.Length == 5 && data[0] == 'r' && data[1] == 'e' && data[2] == 'a' && data[3] == 'd' && data[4] == 'y' )
                {
                    stabilized = true;
                } //unstable message from server
                else if (data.Length == 7 && data[0] == 'u' && data[1] == 'n' && data[2] == 'r' && data[3] == 'e' && data[4] == 'a' && data[5] == 'd' && data[6] == 'y')
                {
                    stabilized = false;
                } //gear status
                else if (data.Length == 2 && data[0] == 'G')
                {
                    if (data[1] == '1')
                        serverGear = true;
                    else
                        serverGear = false;
                }
                else if (data[0] == '$' && data[1] == 'G' && data[2] == 'P' && data[3] == 'R') //GPS data
                {
                    //NMEA RMC
                    String str = System.Text.Encoding.ASCII.GetString(data);
                    String[] fields = str.Split(',');
                    String command = fields[0];
                    String time = fields[1];
                    String active = fields[2];
                    String longitude = fields[3];
                    String longEW = fields[4];
                    String latitude = fields[5];
                    String latiNS = fields[6];
                    String speed = fields[7];
                    String angle = fields[8];
                    String date = fields[9];
                    String magnetic = fields[10];
                    String checksum = fields[11];

                    String latlng = "";
                    //only grab map data if gps is active
                    if (active.CompareTo("A") == 0)
                    {
                        speed = speed.Replace(".", ",");
                        float knots = float.Parse(speed);
                        gpsSpeed = 1.852f * knots;

                        longitude = longitude.Replace(".", ",");
                        int index = longitude.IndexOf(",");

                        String minutes = longitude.Substring(index - 2);
                        double dminutes = Double.Parse(minutes);
                        String degree = longitude.Substring(0, index - 2);
                        double degrees = Convert.ToDouble(degree) + dminutes / 60.0;

                        longitude = degrees.ToString();
                        longitude = longitude.Replace(",", ".");

                        latitude = latitude.Replace(".", ",");
                        index = latitude.IndexOf(",");

                        minutes = latitude.Substring(index - 2);
                        dminutes = Double.Parse(minutes);
                        degree = latitude.Substring(0, index - 2);
                        degrees = Convert.ToDouble(degree) + dminutes / 60.0;

                        latitude = degrees.ToString();
                        latitude = latitude.Replace(",", ".");

                        if (latiNS.CompareTo("S") == 0)
                            latitude = "-" + latitude;

                        if (longEW.CompareTo("W") == 0)
                            longitude = "-" + longitude;

                        latlng = longitude + "," + latitude;
                        String maptype = "hybrid";
                        String format = "jpg";

                        WebClient wc = new WebClient();
                        String maps_command = "http://maps.googleapis.com/maps/api/staticmap?center=" + latlng +
                            "&zoom=16&size=" + map_width + "x" + map_height + "&format=" + format + "&maptype=" + maptype + "&markers=color:blue%7Clabel:S%7C" +
                                latlng + "&sensor=true&key=AIzaSyC8ybRNzgn675MF0ABD1-_SZ0rYudTKixI";
                        Byte[] raw;
                        Byte[] decomp;
                        try
                        {
                            raw = wc.DownloadData(maps_command);
                        }
                        finally
                        {
                            wc.Dispose();
                        }
                        decomp = decompressor.Decompress(raw, false);
                        if (decomp != null)
                        {
                            map.SetData(decomp);
                        }
                    }
                    prevLatlng = latlng;
                }
                else //it is a frame
                {
                    Byte[] decomp = decompressor.Decompress(data, true); //decompress the data
                    //uint[] decomp = decompressor.Decompress(data, true);
    
                    int width = decompressor.getWidth();
                    int height = decompressor.getHeight();

                    bool texture_format_changed = false;

                    //check for format changes
                    if (width != fpv_texture_width)
                    {
                        fpv_texture_width = width;
                        texture_format_changed = true;
                    }
                    if (height != fpv_texture_height)
                    {
                        fpv_texture_height = height;
                        texture_format_changed = true;
                    }

                    if (texture_format_changed)
                        camera = new Texture2D(GraphicsDevice, fpv_texture_width, fpv_texture_height);

                    if (decomp != null) //if decompression was successful set the new camera data
                    {
                        camera.SetData(decomp);
                    }
                    nRec++; //for fps counter
                }
            }
        }

        //updates everything
        protected override void Update(GameTime gameTime)
        {
            GamePadState padState = GamePad.GetState(PlayerIndex.One, GamePadDeadZone.IndependentAxes);
            KeyboardState state = Keyboard.GetState();


            if ((padState.Buttons.Back == ButtonState.Pressed && prevPadState.Buttons.Back == ButtonState.Released)
                || (state.IsKeyDown(Keys.Escape) && prevState.IsKeyUp(Keys.Escape)))
            {
                menu.Toggle();

            }

            //Exit application
            if (menu.GetQuit())
            {
                if (thread.IsAlive)
                {
                    readThread.kill();
                    if (connection.isConnected())
                        connection.disconnect();
                    //thread.Abort();
                    thread.Join();
                }

                SaveConfig();
                Exit();
            }

            //Connect
            if ((state.IsKeyDown(Keys.Enter) && prevState.IsKeyUp(Keys.Enter)) || padState.Buttons.Start == ButtonState.Pressed && prevPadState.Buttons.Start == ButtonState.Released)
            {
                bool useInputIp = false;
                String ipinput = ipInputbox.GetText();
                if (ipinput != null)
                {
                    if (ipinput.Length > 7 && ipinput.Length < 16)
                    {
                        try
                        {
                            int first, second, third;
                            first = second = third = -1;
                            first = ipinput.IndexOfAny(".".ToCharArray());
                            second = ipinput.IndexOfAny(".".ToCharArray(), first + 1, 4);
                            third = ipinput.IndexOfAny(".".ToCharArray(), second + 1, 4);
                            if (first > 0 && second > first && third > second)
                                useInputIp = true;
                            else
                                System.Windows.Forms.MessageBox.Show("Invalid IPV4 adress", "Oh Noes!");
                        }
                        catch (Exception e)
                        {
                            System.Windows.Forms.MessageBox.Show("Invalid IPV4 adress", "Oh Noes!");
                        }
                    }
                }

                parseConfigBoxes();
                ip = ipinput;

                if (!thread.IsAlive)
                {
                    readThread = new ReadThread(ip, fpv_texture_width, fpv_texture_height, fpv_texture_width > 256 ? false : true);
                    thread = new Thread(new ThreadStart(readThread.Run));
                    thread.Start();
                }
                else if (!connection.isConnected())
                {
                    readThread.kill();
                    thread.Join(500);
                    readThread = new ReadThread(ip, fpv_texture_width, fpv_texture_height, fpv_texture_width > 256 ? false : true);
                    thread = new Thread(new ThreadStart(readThread.Run));
                    thread.Start();
                }
                
                if (useInputIp)
                    connection.connect(ipinput, 8001);

               

                sendConfig();

                /*if (!streamThreadT.IsAlive)
                    streamThreadT.Start();*/

                //sendConfig();
            }

            //Disconnect
            if(menu.GetDisconnect())
            {
                if (thread.IsAlive)
                {
                    readThread.kill();
                    thread.Join();
                }
                if (connection.isConnected())
                    connection.disconnect();

                stabilized = false;

                SaveConfig();
            }

            //imageQuality
            if ((padState.DPad.Up == ButtonState.Pressed && prevPadState.DPad.Up == ButtonState.Released) || 
                (state.IsKeyDown(Keys.I) && prevState.IsKeyUp(Keys.I)))
            {
                imageQuality += 5;
                if (imageQuality > 90)
                    imageQuality = 90;
                sendConfig();
            }
            if ((padState.DPad.Down == ButtonState.Pressed && prevPadState.DPad.Down == ButtonState.Released) ||
                (state.IsKeyDown(Keys.K) && prevState.IsKeyUp(Keys.K)))
            {
                imageQuality -= 5;
                if (imageQuality < 1)
                    imageQuality = 1;
                sendConfig();
            }

            //Servos
            if (!connection.isConnected())
            {
                if (padState.DPad.Right == ButtonState.Pressed && prevPadState.DPad.Right == ButtonState.Released)
                {
                    servoBoxes.ForwardSelection();
                }
                if (padState.DPad.Left == ButtonState.Pressed && prevPadState.DPad.Left == ButtonState.Released)
                {
                    servoBoxes.BackwardSelection();
                }
            }

            //map
            if (updateMap && browser.ReadyState == Forms.WebBrowserReadyState.Complete)
            {
                browser.DrawToBitmap(formsBuffer, new Drawing.Rectangle(0, 0, map_width, map_height));
                CopyBitmapToTexture();
                updateMap = false;
            }

            //Gear
            if (padState.Buttons.A == ButtonState.Pressed && prevPadState.Buttons.A == ButtonState.Released)
            {
                gear = !gear;
                if (gear)
                    servos[gearChannel].setOn();
                else
                    servos[gearChannel].setOff();
            }

            //Strafe
            if (padState.Buttons.LeftStick == ButtonState.Pressed && prevPadState.Buttons.LeftStick == ButtonState.Released)
            {
                strafe = !strafe;
            }

            //grab controller values
            x = -padState.ThumbSticks.Right.X;
            y = -padState.ThumbSticks.Right.Y;
            z = padState.Triggers.Right;

            padState.ThumbSticks.Left.Normalize();
            lx = padState.ThumbSticks.Left.X;

            if (lx > 0 && lx < xmin)
                xmin = lx;

            if (lx > 0)
            {
                lx -= 0.27f;
                float add = lx * 0.27f;
                lx += add;
            }
            else if (lx < 0)
            {
                lx += 0.27f;
                float add = lx * 0.27f;
                lx += add;
            }

            lxmean[lxindex] = lx;

            lxindex++;
            if (lxindex > lxmeansize - 1)
                lxindex = 0;

            lx = 0;
            for (int i = 0; i < lxmeansize; i++)
                lx += lxmean[i];

            lx /= lxmeansize;

            //lx *= 0.66f;

            //Deadzoneing of camera
            float xMoveFactor = 0.0f;
            float yMoveFactor = 0.0f;
            if (x > 0.5f || x < -0.5f)
            {
                xMoveFactor = (100.0f * x * (float)gameTime.ElapsedGameTime.TotalSeconds);
            }

            if (y > 0.5f || y < -0.5f)
            {
                yMoveFactor = (50.0f * y * (float)gameTime.ElapsedGameTime.TotalSeconds);
            }

            //Set new values of servos
            servos[cameraXChannel].setValue(servos[0].getValue() + xMoveFactor); //CameraX
            servos[cameraYChannel].setValue(servos[1].getValue() + yMoveFactor); //CameraY
            servos[steeringFChannel].setValue(MathHelper.Clamp(320.0f + ((lx * -1) * 48.0f), servos[steeringFChannel].getMin(), servos[steeringFChannel].getMax())); //Steering 1
            if (strafe)
                servos[steeringBChannel].setValue(MathHelper.Clamp(320.0f + ((lx * -1) * 48.0f), servos[steeringBChannel].getMin(), servos[steeringBChannel].getMax())); //steering 2
            else
                servos[steeringBChannel].setValue(MathHelper.Clamp(320.0f + (lx * 48.0f), servos[steeringBChannel].getMin(), servos[steeringBChannel].getMax())); //steering 2
            servos[throttleFChannel].setValue(MathHelper.Lerp(servos[throttleFChannel].getMin(), maxSpeed, z)); //throttle
            servos[throttleBChannel].setValue(MathHelper.Lerp(servos[throttleBChannel].getMin(), maxSpeed, z));

            //speedLimiter
            if (padState.Buttons.Y == ButtonState.Pressed && prevPadState.Buttons.Y == ButtonState.Released)
            {
                speedLimiter++;
                if (speedLimiter > 3)
                    speedLimiter = 0;

                if (speedLimiter == 0)
                    maxSpeed = 190.0f;
                else if (speedLimiter == 1)
                    maxSpeed = 195.0f;
                else if (speedLimiter == 2)
                    maxSpeed = 200.0f;
                else if (speedLimiter == 3)
                    maxSpeed = 210.0f;

                torqueMeter.SetMax(maxSpeed);
            }

            //Send config, this should become an automated process on connect
            if (padState.Buttons.X == ButtonState.Pressed && prevPadState.Buttons.X == ButtonState.Released)
            {
                sendConfig();
            }

            if (state.IsKeyDown(Keys.W) && prevState.IsKeyUp(Keys.W))
                sendConfig();

            //Reset view
            if (GamePad.GetState(PlayerIndex.One).Buttons.B == ButtonState.Pressed)
            {
                servos[cameraXChannel].setMid();
                servos[cameraYChannel].setMid();
            }

            //Reset steering
            if (padState.Buttons.LeftShoulder == ButtonState.Pressed)
            {
                servos[steeringFChannel].setMid();
                servos[steeringBChannel].setMid();
            }


            if (state.IsKeyDown(Keys.D) && prevState.IsKeyUp(Keys.D))
                printDebug = !printDebug;

            //send input to server
            sendAndAck(gameTime);

            //check for new camera frame and other sensor readings
            parseIndata();

            //input of IP adress to server
            ipInputbox.Update(gameTime);
            servoBoxes.Update();

            //Torquemeter
            torqueMeter.SetValue(MathHelper.Lerp(160.0f, maxSpeed, z));
            speedMeter.SetValue(gpsSpeed);

            //Fps calculations
            recTime += gameTime.ElapsedGameTime.TotalSeconds;

            if (recTime > 1.0)
            {
                framesPSec = nRec;
                nRec = 0;
                recTime -= 1.0;
            }

            menu.Update();

            //save old state of controller and keyboard
            prevState = state;
            prevPadState = padState;
            base.Update(gameTime);
        }

        //Render everything to a texture
        private void Render(GameTime gameTime)
        {
            spriteBatch.Begin();
            spriteBatch.Draw(camera, new Rectangle(0, 0, 1024, screen_height), null, Color.White, MathHelper.Pi, new Vector2((float)fpv_texture_width, (float)fpv_texture_height), SpriteEffects.None, 0);

            if (connection.isConnected())
                spriteBatch.DrawString(font, "Connected", new Vector2(screen_width - 200, 50), Color.White);
            else
                spriteBatch.DrawString(font, "Disconnected", new Vector2(screen_width - 200, 50), Color.White);

            spriteBatch.DrawString(font, "FPS:" + framesPSec.ToString(), new Vector2(1024, 100), Color.White);
            spriteBatch.DrawString(font, batteryString0, new Vector2(screen_width - 300, 100), Color.White);
            spriteBatch.DrawString(font, batteryString1, new Vector2(screen_width - 300, 135), Color.White);
            spriteBatch.DrawString(font, batteryString2, new Vector2(screen_width - 300, 170), Color.White);
            spriteBatch.DrawString(font, batteryString3, new Vector2(screen_width - 300, 205), Color.White);

            if (printDebug)
            {
                spriteBatch.DrawString(font, x.ToString() + ":" + servos[cameraXChannel].getValue().ToString(), new Vector2(10, 50), Color.White);
                spriteBatch.DrawString(font, y.ToString() + ":" + servos[cameraYChannel].getValue().ToString(), new Vector2(10, 100), Color.White);
                spriteBatch.DrawString(font, z.ToString() + ":" + servos[throttleFChannel].getValue().ToString(), new Vector2(10, 150), Color.White);
                spriteBatch.DrawString(font, lx.ToString() + ":" + servos[steeringFChannel].getValue().ToString() + ":" + servos[steeringBChannel].getValue().ToString(), new Vector2(10, 200), Color.White);
                spriteBatch.DrawString(font, xmin.ToString(), new Vector2(1124, 650), Color.White);
            }
            spriteBatch.DrawString(font, "Strafe: " + (strafe ? "On" : "Off"), new Vector2(1024, screen_height / 2 - 200), Color.White);
            spriteBatch.DrawString(font, "Gear: " + (serverGear ? "Reverse" : "Forward"), new Vector2(1024, screen_height / 2 - 250), Color.White);
            if (stabilized)
                spriteBatch.DrawString(font, "Connection Stabilized", new Vector2(1024, 50), Color.Green);
            else
                spriteBatch.DrawString(font, "Connection Unstable", new Vector2(1024, 50), Color.Red);
            spriteBatch.DrawString(font, "Speed limit: " + maxSpeed, new Vector2(1024, screen_height / 2 - 150), Color.White);

            if(!connection.isConnected())
            {
                spriteBatch.DrawString(font, "Hello and welcome to the car client!", new Vector2(100, 50), Color.White);
                spriteBatch.DrawString(font, "Enter ip adress to the server in the top right corner", new Vector2(100, 100), Color.White);
                spriteBatch.DrawString(font, "Current configureation", new Vector2(100, 250), Color.White);
                spriteBatch.DrawString(font, "Servo 0: ", new Vector2(100, 300), Color.White);
                spriteBatch.DrawString(font, "Servo 1: ", new Vector2(100, 350), Color.White);
                spriteBatch.DrawString(font, "Servo 2: ", new Vector2(100, 400), Color.White);
                spriteBatch.DrawString(font, "Servo 3: ", new Vector2(100, 450), Color.White);
                spriteBatch.DrawString(font, "Servo 4: ", new Vector2(100, 500), Color.White);
                spriteBatch.DrawString(font, "Servo 5: ", new Vector2(100, 550), Color.White);
                spriteBatch.DrawString(font, "Servo 6: ", new Vector2(100, 600), Color.White);
                spriteBatch.DrawString(font, "Servo 7: ", new Vector2(100, 650), Color.White);

                servoBoxes.Draw(spriteBatch);

            }

            //ipInput.Draw(spriteBatch, gameTime);
            ipInputbox.Draw(spriteBatch);

            if (renderMap || alwaysRenderMap)
                spriteBatch.Draw(map, new Rectangle(1024, (screen_height / 2), screen_width - 1024, (screen_height / 2)), Color.White);

            torqueMeter.Draw(spriteBatch);
            speedMeter.Draw(spriteBatch);

            menu.Draw(spriteBatch);


            //spriteBatch.Draw(testTexture, new Rectangle(0, 0, screen_width, screen_height), Color.White);
            spriteBatch.End();
        }

        //draw the rendered texture to the screen
        protected override void Draw(GameTime gameTime)
        {
            //GraphicsDevice.Clear(Color.Black);

            //Render(gameTime);

            GraphicsDevice.SetRenderTarget(renderTarget);
            GraphicsDevice.Clear(Color.Black);
            Render(gameTime);
            GraphicsDevice.SetRenderTarget(null);
            screen = (Texture2D)renderTarget;

            GraphicsDevice.Clear(Color.Black);
            spriteBatch.Begin();
            spriteBatch.Draw(screen, Vector2.Zero, Color.White);
            spriteBatch.End();

            base.Draw(gameTime);

        }
    }
}
