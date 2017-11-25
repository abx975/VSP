import java.util.TimerTask;

/**
 * Handles sending of messages
 * @author lennarthartmann
 * @version 24.11.2017
 */
public class Sender extends TimerTask{

	private static int id = 2;
	MessageBuffer srcReader=null;
	
	/**
	 * Constructor
	 */
	public Sender() {
		id++;
		this.srcReader=MessageBuffer.getInstance();
	}
	
	public Sender(MessageBuffer srcReader) {
		id++;
		this.srcReader=srcReader;
	}
	
	private void getSenderId() {
		System.err.println("Sender ID: "+id);
	}
	
	public void run() {
		//todo:real implementation
		System.err.print("Sender Got frame: ");
		System.err.println(new String(srcReader.getTestframe())+" -> send");
	}
}
