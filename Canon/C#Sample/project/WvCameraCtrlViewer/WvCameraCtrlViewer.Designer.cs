namespace WvCameraCtrlViewer
{
    partial class WvCameraCtrlViewer
    {
        /// <summary>
        /// Required designer variable
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up all resources in use.
        /// </summary>
        /// <param name="disposing">true to dispose managed resources, false to not dispose managed resources.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code


        /// <summary>
        /// Required method for designer support 
        /// Do not change the method by using the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.liveScreen = new System.Windows.Forms.PictureBox();
            this.pScrollBar = new System.Windows.Forms.HScrollBar();
            this.controlButton = new System.Windows.Forms.Button();
            this.tScrollBar = new System.Windows.Forms.VScrollBar();
            this.zScrollBar = new System.Windows.Forms.VScrollBar();
            ((System.ComponentModel.ISupportInitialize)(this.liveScreen)).BeginInit();
            this.SuspendLayout();
            // 
            // liveScreen
            // 
            this.liveScreen.Location = new System.Drawing.Point(12, 12);
            this.liveScreen.Name = "liveScreen";
            this.liveScreen.Size = new System.Drawing.Size(320, 240);
            this.liveScreen.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.liveScreen.TabIndex = 0;
            this.liveScreen.TabStop = false;
            // 
            // pScrollBar
            // 
            this.pScrollBar.Location = new System.Drawing.Point(9, 255);
            this.pScrollBar.Name = "pScrollBar";
            this.pScrollBar.Size = new System.Drawing.Size(323, 21);
            this.pScrollBar.TabIndex = 1;
            this.pScrollBar.Scroll += new System.Windows.Forms.ScrollEventHandler(this.pScrollBar_Scroll);
            // 
            // controlButton
            // 
            this.controlButton.Location = new System.Drawing.Point(336, 253);
            this.controlButton.Name = "controlButton";
            this.controlButton.Size = new System.Drawing.Size(75, 23);
            this.controlButton.TabIndex = 2;
            this.controlButton.Text = "push";
            this.controlButton.UseVisualStyleBackColor = true;
            this.controlButton.Click += new System.EventHandler(this.controlButton_Click);
            // 
            // tScrollBar
            // 
            this.tScrollBar.Location = new System.Drawing.Point(336, 13);
            this.tScrollBar.Name = "tScrollBar";
            this.tScrollBar.Size = new System.Drawing.Size(17, 237);
            this.tScrollBar.TabIndex = 3;
            this.tScrollBar.Scroll += new System.Windows.Forms.ScrollEventHandler(this.tScrollBar_Scroll);
            // 
            // zScrollBar
            // 
            this.zScrollBar.Location = new System.Drawing.Point(372, 13);
            this.zScrollBar.Name = "zScrollBar";
            this.zScrollBar.Size = new System.Drawing.Size(17, 237);
            this.zScrollBar.TabIndex = 4;
            this.zScrollBar.Scroll += new System.Windows.Forms.ScrollEventHandler(this.zScrollBar_Scroll);
            // 
            // WvCameraCtrlViewer
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(422, 286);
            this.Controls.Add(this.zScrollBar);
            this.Controls.Add(this.tScrollBar);
            this.Controls.Add(this.controlButton);
            this.Controls.Add(this.pScrollBar);
            this.Controls.Add(this.liveScreen);
            this.Name = "WvCameraCtrlViewer";
            this.Text = "Form1";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.WvCameraCtrlViewer_FormClosing);
            this.Load += new System.EventHandler(this.WvCameraCtrlViewer_Load);
            ((System.ComponentModel.ISupportInitialize)(this.liveScreen)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.PictureBox liveScreen;
        private System.Windows.Forms.HScrollBar pScrollBar;
        private System.Windows.Forms.Button controlButton;
        private System.Windows.Forms.VScrollBar tScrollBar;
        private System.Windows.Forms.VScrollBar zScrollBar;
    }
}

