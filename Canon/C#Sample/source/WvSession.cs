using System;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;
using System.IO;
using System.Net;
using System.Drawing;
using System.Threading;

namespace WvCameraCtrlViewer
{
    delegate void WvEventCallback(string str);

    /// <summary>
    /// Class for managing a session
    /// </summary>
    class WvSession : IDisposable
    {
        private string server;
        private string connectionId;
        private Thread thread;

        public event WvEventCallback OnGetNotice;


        public WvSession(string server)
        {
            this.server = server;
            string str = SendCommand("OpenCameraServer");

            // Extract connection_id from the response.
            Match m = Regex.Match(str, "connection_id=(.+)");
            if(m.Success)
                connectionId = m.Groups[1].Value;

            // Start a thread.
            thread = new Thread(new ThreadStart(GetNotice));
            thread.Start();
        }


        /// <summary>
        /// Main thread of session class
        /// Repeat the GetNotice command and notify the result to the window.
        /// </summary>
        private void GetNotice()
        {
            while (thread != null)
            {
                string str = SendCommand("GetNotice");
                if (OnGetNotice != null)
                {
                    OnGetNotice(str);
                }
            }
        }


        /// <summary>
        /// Obtain images from a camera server.
        /// </summary>
        public Image GetImage()
        {
            string url = server + "GetLiveImage?connection_id=" + connectionId;
            HttpWebRequest req = (HttpWebRequest)HttpWebRequest.Create(url);
            HttpWebResponse res = (HttpWebResponse)req.GetResponse();
            return new Bitmap(res.GetResponseStream());
        }


        /// <summary>
        /// Send a command to a camera server and return the response.
        /// </summary>
        public string SendCommand(string command)
        {
            if (connectionId != null)
            {
                if (command.IndexOf('?') > 0)
                {
                    command = command + "&connection_id=" + connectionId;
                }
                else
                {
                    command = command + "?connection_id=" + connectionId;
                }
            }

            try
            {
                HttpWebRequest req = (HttpWebRequest)HttpWebRequest.Create(server + command);
                HttpWebResponse res = (HttpWebResponse)req.GetResponse();
                StreamReader sr = new StreamReader(res.GetResponseStream());
                return sr.ReadToEnd();
            }
            catch (Exception){}

            return null;
        }



        public void Dispose()
        {
            thread = null;
        }
    }
}
