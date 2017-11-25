import java.net.*;
import java.util.Observable;
import java.lang.Runnable;

/**
 * Receives messages and notifies Station
 * @author lennarthartmann
 * @version 24.11.2017
 */
public class Receiver extends Observable implements Runnable{

	//Interface Address
//	private String interf;
//	private int port;
	private InetAddress ipAddr;
	private MulticastSocket socket;
//	private int syncOffset;
	//management variables
	private int currentSlot;
	private int nextSlot;

	private boolean runninig=true;
	MessageBuffer srcRdr=null;//provisional
	
	/**
	 * Constructor
	 */
	public Receiver(){
		srcRdr = MessageBuffer.getInstance();
	}
	
	/**
	 * Constructor
	 * @param srcRdr	a specific input reader for testing purposes
	 */
	public Receiver(MessageBuffer srcRdr){
		this.srcRdr=srcRdr;
	}
	
	public void run() {
		System.err.println("Receiver launching");
		// TODO Auto-generated method stub
		while (runninig){		
			if(Thread.currentThread().isInterrupted())break;
			//todo:receive
			try{
				Thread.sleep(4000);
			}catch(Exception e){
				System.err.println("Receiver:wait failed");
				e.printStackTrace();
			}
			setChanged();
			notifyObservers(srcRdr.getFrame());
			System.err.println("receive blocked");
		}
	}	
}
