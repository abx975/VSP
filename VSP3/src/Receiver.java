import java.net.*;
import java.util.Observable;
import java.io.IOException;
import java.lang.Runnable;

/**
 * Receives messages and notifies Station
 * @author lennarthartmann
 * @version 24.11.2017
 */
public class Receiver extends Observable implements Runnable{

	public static final int MESSAGE_SIZE = 34; 
	private String interf;
	private InetAddress mCastAddr;
	private MulticastSocket socket;
	
	byte[] receiveBuff;

	
	MessageBuffer srcRdr=null;//provisional
	
	/**
	 * Constructor
	 * @throws IOException 
	 */
	public Receiver() throws IOException{
		srcRdr = MessageBuffer.getInstance();
		socket = new MulticastSocket();
		socket.joinGroup(this.mCastAddr);
		socket.setNetworkInterface(NetworkInterface.getByName(this.interf));
	}
	
	/**
	 * Constructor
	 * @param srcRdr	a specific input reader for testing purposes
	 * @throws IOException 
	 */
	public Receiver(MessageBuffer srcRdr) throws IOException{
		this.srcRdr=srcRdr;
		socket = new MulticastSocket();
		socket.joinGroup(this.mCastAddr);
		socket.setNetworkInterface(NetworkInterface.getByName(this.interf));
	}
	
	/**
	 * Constructor actually in use
	 * @param stationType	clock reliability class
	 * @param interf	interface in use
	 * @param host	hostaddress
	 * @param port	port in use
	 * @param clockCorrection	deliberate clock inaccuracy
	 */
	public Receiver(char stationType, String interf, String host, int port, long clockCorrection){
		this.interf=interf;
		try {
			socket = new MulticastSocket(port);
		} catch (IOException e) {
			e.printStackTrace();
		}
		try {
			this.mCastAddr=InetAddress.getByName(host);
		} catch (UnknownHostException e) {
			e.printStackTrace();
		}
		try {
			socket.joinGroup(mCastAddr);
		} catch (IOException e) {
			e.printStackTrace();
		}
		
	}
	
	public void run() {
		System.err.println("Receiver launching");
		boolean runninig=true;
		while (runninig){
			if(Thread.currentThread().isInterrupted()){
				runninig = false;
			}		
			receiveBuff = new byte[MESSAGE_SIZE];
			DatagramPacket packet = new DatagramPacket(receiveBuff,receiveBuff.length);
			try {
				socket.receive(packet);
				setChanged();
				notifyObservers(packet.getData());
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		socket.close();
	}	
}
