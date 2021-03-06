import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.net.NetworkInterface;
import java.util.TimerTask;

/**
 * Handles sending of messages
 * @author lennarthartmann
 * @version 24.11.2017
 */
public class Sender extends TimerTask{

	public static final int TTL=1;
	MessageBuffer srcReader=null; //debug only
	
	//management info
	private int msgCount=0;
	private char stationType = 'A';
	private long localTimeOffset=0;
	private int timeslot=0;
	
	//Conection info
	private int port;
	private InetAddress mCastAddr;
	private String interf;
	private MulticastSocket socket;
	
	/**
	 * Constructor
	 * @throws IOException 
	 */
	public Sender(String mCastIP, int port, String interf) {
		this.srcReader=MessageBuffer.getInstance();
		this.port=port;
		this.interf=interf;
		//set up socket
		try {
			socket = new MulticastSocket(port);
			socket.setTimeToLive(TTL);
			//socket.setNetworkInterface(NetworkInterface.getByName(interf));
			socket.setNetworkInterface(NetworkInterface.getByName(interf));
			socket.joinGroup(this.mCastAddr);
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public Sender(char type) {
		this.srcReader=MessageBuffer.getInstance();

		//set up socket
		try {
			socket = new MulticastSocket(this.port);
			socket.setTimeToLive(TTL);
			socket.setNetworkInterface(NetworkInterface.getByName(this.interf));
			socket.joinGroup(this.mCastAddr);
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public Sender(MessageBuffer srcReader) {
		this.srcReader=srcReader;
	}
	
	/**
	 * getter
	 * @return number of messages sent
	 */
	public int getMsgCount(){
		return msgCount;
	}
	
	/**
	 * setter
	 * @param stationType	the reliability class of this Stations clock
	 */
	public void setStationType(char stationType){
		this.stationType=stationType;
	}
	
	/**
	 * setter
	 * @param localTimeOffset	the desired inaccuracy
	 */
	public void setLocalTimeOffset(long localTimeOffset) {
		this.localTimeOffset = localTimeOffset;
	} 
	
	/**
	 * setter
	 * @param timeslot	the Timeslot to be used
	 */
	public void setTimeslot(int timeslot){
		this.timeslot = timeslot;
	}
		
	public void run() {
		//todo:real implementation
		System.err.print("Sender Got frame: ");
		System.err.println(new String(srcReader.getTestframe())+" -> send");
		
		
		byte[] msg = VS3Messages.getMessage('A', "Test für sender: Nr."+msgCount, (byte)1, System.currentTimeMillis());
		msgCount++;
		try {
			socket.send(new DatagramPacket(msg, msg.length,mCastAddr,port));
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
