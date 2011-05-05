import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.net.*;
import java.util.regex.*;
import javax.imageio.*;

/**
 * Window class
 * 
 * Create and lay out a scrollbar for PTZ and a camera control button.
 */
class WvFrame extends Frame implements AdjustmentListener, ActionListener
{
	private WvCanvas canvas;
	private Scrollbar pBar, tBar, zBar;
	private Button button;
	private ActionListener listener;
	private WvSession session;

	
	WvFrame(WvSession session) throws Exception
	{
		super("Network Camera Control Application");
		setSize(400, 300);

		this.session = session;

		// Camera control button
		button = new Button(" push ");
		button.addActionListener(this);
		button.setBackground(Color.yellow);

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

		Panel p1 = new Panel();
		p1.setLayout(new BorderLayout());
		p1.add("Center", pBar);
		p1.add("East", button);

		setLayout(new BorderLayout(10,10));
		add("Center", canvas);
		add("East", p0);
		add("South", p1);

		// Obtain the camera information (PTZ range etc) and initialize the GUI.
		String ‚“‚”‚’ = sendCommand("GetCameraInfo");
		processServerResponse(‚“‚”‚’);
		
		addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent windowEvent) {
				System.exit(0);
			}
		});
	}

	
	/**
	 * Interpret an event notification from a camera server and renew the GUI.
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
			else if(line.startsWith("pan_left_limit=")) {
				pBar.setMinimum(getValue(line));
			}
			else if(line.startsWith("pan_right_limit=")) {
				pBar.setMaximum(getValue(line));
			}
			else if(line.startsWith("tilt_up_limit=")) {
				tBar.setMaximum(getValue(line));
			}
			else if(line.startsWith("tilt_down_limit=")) {
				tBar.setMinimum(getValue(line));
			}
			else if(line.startsWith("zoom_tele_end=")) {
				zBar.setMinimum(getValue(line));
			}
			else if(line.startsWith("zoom_wide_end=")) {
				zBar.setMaximum(getValue(line));
			}
			else if(line.startsWith("enabled_camera_control"))
			{
				button.setLabel("control");
				button.setBackground(Color.green);
			}
			else if(line.startsWith("disabled_camera_control") ||
					line.startsWith("failed_to_get_camera_control"))
			{
				button.setLabel(" push ");
				button.setBackground(Color.yellow);
			}
			else if(line.startsWith("waiting_camera_control"))
			{
				button.setLabel("waiting");
				button.setBackground(Color.magenta);
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
	 * Display an image.
	 */
	public void setImage(Image image) {
		canvas.liveImage = image;
		canvas.repaint();
	}

	
	/**
	 * Send a command to a camera server and return the response.(Actual process is done by the session object.)
	 */
	private String sendCommand(String command)
	{
		try {
			return session.sendCommand(command);
		}catch(Exception e) {
			return null;
		}
	}
	
	
	/**
	 * Callback function called when the control button is pressed
	 */
	public void actionPerformed(ActionEvent e) {
		sendCommand("GetCameraControl");
	}

	
	/**
	 * Callback function called when the scroll bar is controlled
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
 * Class for managing a session with a camera server
 * 
 * Create a thread and execute the GetNotice command to a camera server.
 */
class WvSession implements Runnable {
	String server;
	String connectionId;
	Thread thread;
	WvFrame frame;

	
	/**
	 * Connect to a camera server to obtain a session ID.
	 */
	public WvSession(String server) throws Exception {
		this.server = server;
		String str = sendCommand("OpenCameraServer");

		// Extract connection_id from the response.
		Pattern pattern = Pattern.compile("connection_id=(.+)");
		Matcher matcher = pattern.matcher(str);
		if(matcher.find())
			connectionId = matcher.group(1);

		thread = new Thread(this);
		thread.start();
	}
	
	
	public void addListener(WvFrame frame) {
		this.frame = frame;
	}
	
	
	/**
	 * Main thread of session class
	 * Repeat the GetNotice command and notify the result to the window.
	 */
	public void run() {
		try {
			while(thread != null) {
				String str = sendCommand("GetNotice");
				if (frame != null) {
					frame.processServerResponse(str);
				}
				Thread.sleep(200);
			}
		}catch(Exception e) {}
	}

	
	/**
	 * Obtain images from a camera server.
	 */
	public Image getImage() throws Exception {
		URL url = new URL(server + "GetLiveImage?connection_id=" + connectionId);
		URLConnection con = url.openConnection();
		return ImageIO.read(con.getInputStream());
	}


	/**
	 * Send a command to a camera server and return the response.
	 */
	public String sendCommand(String command) throws Exception {
		if (command.indexOf("?") > 0) {
			command = command + "&connection_id=" + connectionId;
		} else {
			command = command + "?connection_id=" + connectionId;
		}
		
		URL url = new URL(server + command);
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
	}
}


/**
 * Main class
 * 
 * Create a window and a session, and then associate them with each other.
 * Also, display a live image obtained from a camera server on the window.
 */
public class CameraCtrlViewer {
	
	public static void main(String[] args) throws Exception {
		
		// 0)Create a session.
		WvSession session = new WvSession("http://172.20.28.76/-wvhttp-01-/");

		// 1)Create and display a window.
		WvFrame f = new WvFrame(session);
		f.setVisible(true);
		
		// 2)Pass the window to the session object.
		session.addListener(f);

		// 3)Renew the image.
		while(true) {
			f.setImage(session.getImage());
			Thread.sleep(500);
		}
	}
}