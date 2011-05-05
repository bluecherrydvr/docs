import java.awt.*;
import java.awt.image.*;
import java.awt.event.*;
import java.io.*;
import java.net.*;
import javax.imageio.*;

/**
 * Window class
 * 
 * Create and lay out a scrollbar for PTZ.
 */
class WvFrame extends Frame implements AdjustmentListener
{
	private WvCanvas canvas;
	private Scrollbar pBar, tBar, zBar;
	private Button button;
	
	
	WvFrame() throws Exception
	{
		super("Network Camera Control Application");
		setSize(400, 300);

		// Scrollbar button
		tBar = new Scrollbar(Scrollbar.VERTICAL);
		pBar = new Scrollbar(Scrollbar.HORIZONTAL);
		zBar = new Scrollbar(Scrollbar.VERTICAL);
		
		pBar.addAdjustmentListener(this);
		tBar.addAdjustmentListener(this);
		zBar.addAdjustmentListener(this);

		// Image display area
		canvas = new WvCanvas();

		// Layout each part
		Panel p0 = new Panel();
		p0.setLayout(new GridLayout(1,2,10,0));
		p0.add(tBar);
		p0.add(zBar);

		setLayout(new BorderLayout(10,10));
		add("Center", canvas);
		add("East", p0);
		add("South", pBar);

		// Obtain the camera information (PTZ range etc) and initialize the GUI.
		String str = sendCommand("GetCameraInfo");
		processServerResponse(str);
		
		addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent windowEvent) {
				System.exit(0);
			}
		});		
	}
	
	
	/**
	 * Interpret a response from a camera server and renew the GUI.
	 */
	public void processServerResponse(String response) throws Exception
	{
		int pVal = Integer.MIN_VALUE, tVal = Integer.MIN_VALUE, zVal = Integer.MIN_VALUE;

		BufferedReader br = new BufferedReader(new StringReader(response));
		while(true) {
			String line = br.readLine();
			if(line == null) {
				break;
			}
			else if(line.startsWith("pan_current_value=")||line.startsWith("pan="))
			{
				pVal = getValue(line);
			}
			else if(line.startsWith("tilt_current_value=")||line.startsWith("tilt="))
			{
				tVal = getValue(line);
			}
			else if(line.startsWith("zoom_current_value=")||line.startsWith("zoom="))
			{
				zVal = getValue(line);
			}
			else if(line.startsWith("pan_left_end=")) {
				pBar.setMinimum(getValue(line));
			}
			else if(line.startsWith("pan_right_end=")) {
				pBar.setMaximum(getValue(line));
			}
			else if(line.startsWith("tilt_up_end=")) {
				tBar.setMaximum(getValue(line));
			}
			else if(line.startsWith("tilt_down_end=")) {
				tBar.setMinimum(getValue(line));
			}
			else if(line.startsWith("zoom_tele_end=")) {
				zBar.setMinimum(getValue(line));
			}
			else if(line.startsWith("zoom_wide_end=")) {
				zBar.setMaximum(getValue(line));
			}
		}
		
		// Note: Please note that the tilt range is upside down.
		if(pVal != Integer.MIN_VALUE)
			pBar.setValue(pVal);
		if(tVal != Integer.MIN_VALUE)
			tBar.setValue( tBar.getMaximum() + tBar.getMinimum() - tVal );
		if(zVal != Integer.MIN_VALUE)
			zBar.setValue(zVal);
	}
	
	
	private int getValue(String line)
	{
		int p = line.indexOf('=');
		if( p > 0 )
			return Integer.parseInt(line.substring(p+1));
		return 0;
	}


	/**
	 * Dispaly an image.
	 */
	public void setImage(Image image) {
		canvas.liveImage = image;
		canvas.repaint();
	}

	
	/**
	 * Callback function called when the scrollbar is controlled
	 */
	public void adjustmentValueChanged(AdjustmentEvent e){
		if(e.getValueIsAdjusting())
			return;

		Object src = e.getSource();
		int val = e.getValue();
		if(src == pBar) {
			sendCommand("control?pan="+val);
		}
		else if(src == tBar) {
			sendCommand("control?tilt="+(tBar.getMaximum() + tBar.getMinimum() - val));
		}
		else if(src == zBar) {
			sendCommand("control?zoom="+val);
		}
	}

	
	/**
	 * Send a command to a camera server and return the response.
	 */
	private String sendCommand(String command)
	{
		try {
			URL url = new URL("http://172.20.28.60/-wvhttp-01-/" + command);
			URLConnection con = url.openConnection();
			InputStreamReader isr = new InputStreamReader(con.getInputStream());
			BufferedReader br = new BufferedReader(isr);
			StringBuilder sb = new StringBuilder();
			while (true) {
				String line = br.readLine();
				if (line == null)
					break;
				sb.append(line+"\n");
			}
			return sb.toString();
		} catch (Exception e) {}
		return null;
	}

	
	/**
	 * Internal class for displaying live images
	 */
	class WvCanvas extends Canvas {
		private Image liveImage;

		public void paint(Graphics g) {
			if (liveImage != null) {
				Dimension d = getSize();
				g.drawImage(liveImage, 0, 0, d.width, d.height, null);
			}
		}

		public void update(Graphics g) {
			paint(g);
		}
				
		public Dimension getPreferredSize() {
			return new Dimension(320, 240);
		}
	}
}


/**
 * Main class
 * 
 * Create a window and display live images obtained from a camera server on the window.
 */
public class SimpleCtrlViewer {

	public static void main(String[] args) throws Exception {
		
		// 0)Create and display a window.
		WvFrame f = new WvFrame();
		f.setVisible(true);

		while(true)
		{
			// 1)Create the URL for obtaining images.
			URL url = new URL("http://172.20.28.60/-wvhttp-01-/GetOneShot");

			// 2)Connect to a server.
			URLConnection con = url.openConnection();

			// 3)Create an Image object from a server's response.
			BufferedImage image = ImageIO.read(con.getInputStream());
				
			// 4)Display obtained images on the window.
			f.setImage(image);
				
			// 5)Sleep.
			Thread.sleep(500);
		}
	}
}
