import java.awt.*;
import java.awt.image.*;
import java.awt.event.*;
import java.net.*;
import javax.imageio.*;

/**
 * Window class
 * 
 * Pop up a window and draw an image on it.
 */
class WvFrame extends Frame
{
	private Image liveImage;

	WvFrame()
	{
		super("Network Camera Sample Application");
		setSize(320, 260);
		addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent windowEvent) {
				System.exit(0);
			}
		});
	}

	public void setImage(Image image) {
		liveImage = image;
		repaint();
	}

	public void paint(Graphics g) {
		if(liveImage != null)
			g.drawImage(liveImage, 0, 20, null);
	}
	
	public void update(Graphics g) {
		paint(g);
	}
}


/**
 * Main class
 * 
 * Create a window and display live images obtained from a camera server on the window.
 */
public class SimpleStillImage {

	public static void main(String[] args) throws Exception {
	
		// 0)Create and display a window.
		WvFrame f = new WvFrame();
		f.setVisible(true);

		// 1)Create the URL for obtaining images.
		URL url = new URL("http://172.20.28.60/-wvhttp-01-/GetOneShot");

		// 2)Connect to a server.
		URLConnection con = url.openConnection();

		// 3)Create an Image object from a server's response.
		BufferedImage image = ImageIO.read(con.getInputStream());
			
		// 4)Display obtained images on the window.
		f.setImage(image);
	}
}
