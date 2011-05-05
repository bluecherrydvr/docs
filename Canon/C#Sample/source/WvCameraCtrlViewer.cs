using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.IO;
using System.Windows.Forms;

namespace WvCameraCtrlViewer
{

    /// <summary>
    /// Window class
    /// 
    /// Create and lay out a scrollbar for PTZ and a camera control button.  
    /// </summary>
    public partial class WvCameraCtrlViewer : Form
    {
        private WvSession session;
        private Timer imageTimer;
        private delegate void GetNoticeCallback(string str);


        public WvCameraCtrlViewer()
        {
            InitializeComponent();
        }


        /// <summary>
        /// Start a session with a camera server.
        /// 
        /// This callback function is executed after the GUI is initialized.
        /// </summary>
        private void WvCameraCtrlViewer_Load(object sender, EventArgs e)
        {
            // 0)Start a session with a camera server.
            session = new WvSession("http://172.20.28.60/-wvhttp-01/"); 


            // 1)Register the event handler function.
            session.OnGetNotice += new WvEventCallback(ProcessServerResponse);


            // 2)Brings the slider bar and others up to date.
            string str = session.SendCommand("GetCameraInfo");
            ProcessServerResponse(str);


            // 3)Start the timer for obtaining images.
            imageTimer = new System.Windows.Forms.Timer();
            imageTimer.Interval = 500;
            imageTimer.Tick += new EventHandler(ImageTimerTick);
            imageTimer.Enabled = true;
        }


        /// <summary>
        /// Callback function for the timer
        /// </summary>
        private void ImageTimerTick(object sender, EventArgs e)
        {
            liveScreen.Image = session.GetImage();
        }

        
        /// <summary>
        /// Interpret an event notification from a camera server and renew the GUI.
        /// </summary>
        void ProcessServerResponse(string str)
        {
            if (string.IsNullOrEmpty(str))
                return;

            if (this.InvokeRequired)
            {
                GetNoticeCallback d = new GetNoticeCallback(ProcessServerResponse);
                this.Invoke(d, new object[] { str });
            }
            else
            {
                int pVal = int.MaxValue, tVal = int.MaxValue, zVal = int.MaxValue;

                StringReader sr = new StringReader(str);
                while (true)
                {
                    string line = sr.ReadLine();
                    if (line == null) break;

                    if (line.StartsWith("pan_current_value") || line.StartsWith("pan="))
                    {
                        pVal = GetValue(line);
                    }
                    else if (line.StartsWith("tilt_current_value") || line.StartsWith("tilt="))
                    {
                        tVal = GetValue(line);
                    }
                    else if (line.StartsWith("zoom_current_value") || line.StartsWith("zoom="))
                    {
                        zVal = GetValue(line);
                    }
                    else if (line.StartsWith("pan_left_limit="))
                    {
                        pScrollBar.Minimum = GetValue(line);
                    }
                    else if (line.StartsWith("pan_right_limit="))
                    {
                        pScrollBar.Maximum = GetValue(line);
                    }
                    else if (line.StartsWith("tilt_up_limit="))
                    {
                        tScrollBar.Maximum = GetValue(line);
                    }
                    else if (line.StartsWith("tilt_down_limit="))
                    {
                        tScrollBar.Minimum = GetValue(line);
                    }
                    else if (line.StartsWith("zoom_tele_end="))
                    {
                        zScrollBar.Minimum = GetValue(line);
                    }
                    else if (line.StartsWith("zoom_wide_end="))
                    {
                        zScrollBar.Maximum = GetValue(line);
                    }
                    else if (line.StartsWith("enabled_camera_control"))
                    {
                        controlButton.Text = "control";
                        controlButton.BackColor = Color.Green;
                    }
                    else if (line.StartsWith("disabled_camera_control") ||
                        line.StartsWith("failed_to_get_camera_control"))
                    {
                        controlButton.Text = "push";
                        controlButton.BackColor = Color.Yellow;
                    }
                    else if (line.StartsWith("waiting_camera_control"))
                    {
                        controlButton.Text = "waiting";
                        controlButton.BackColor = Color.Magenta;
                    }
                }

                // Note: Please note that the tile range is upside down.
                if (pVal != int.MaxValue)
                    pScrollBar.Value = pVal;
                if (tVal != int.MaxValue)
                    tScrollBar.Value = tScrollBar.Maximum + tScrollBar.Minimum - tVal;
                if (zVal != int.MaxValue)
                    zScrollBar.Value = zVal;
            }
        }


        private int GetValue(string line)
        {
            int p = line.IndexOf('=');
            if (p > 0)
                return int.Parse(line.Substring(p + 1));
            return 0;
        }


        // Callback function called when the control button is pressed.
        private void controlButton_Click(object sender, EventArgs e)
        {
            session.SendCommand("GetCameraControl");
        }


        // Callback function called when the scroll bar (pan) is controlled 
        private void pScrollBar_Scroll(object sender, ScrollEventArgs e)
        {
            if (e.Type == ScrollEventType.EndScroll)
                session.SendCommand("OperateCamera?pan="+e.NewValue);
        }


        // Callback function called when the scroll bar (tilt) is controlled
        private void tScrollBar_Scroll(object sender, ScrollEventArgs e)
        {
            if (e.Type == ScrollEventType.EndScroll)
            {
                int t = tScrollBar.Maximum + tScrollBar.Minimum - e.NewValue;
                session.SendCommand("OperateCamera?tilt=" + t);
            }
        }


        // Callback function called when the scroll bar (zoom) is controlled
        private void zScrollBar_Scroll(object sender, ScrollEventArgs e)
        {
            if (e.Type == ScrollEventType.EndScroll)
                session.SendCommand("OperateCamera?zoom=" + e.NewValue);
        }

        
        // Callback function called when the window is closed. Close the session
        private void WvCameraCtrlViewer_FormClosing(object sender, FormClosingEventArgs e)
        {
            imageTimer.Enabled = false;

            session.OnGetNotice -= new WvEventCallback(ProcessServerResponse);
            session.SendCommand("CloseCameraServer");
            session.Dispose();
        }
    }
}