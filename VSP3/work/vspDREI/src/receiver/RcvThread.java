package src.receiver;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

public class RcvThread extends Thread {

	private MulticastSocket socket;
	private int port;
	private String interf;
	private InetAddress mCastAdr;
	private Receiver rcvr;
	private Queue<RcvObj> msgs;
	
	public RcvThread(int port, String interf, InetAddress inetadr, Receiver rcvr) {
		super();
		this.port = port;
		this.interf = interf;
		this.mCastAdr = inetadr;
		this.rcvr = rcvr;
		this.msgs = new LinkedList<>();
	}

	@Override
	public void run() {
		super.run();
		try {	
			try {
				Thread.sleep(1000 - (this.rcvr.getTime() % 1000));
			} catch (InterruptedException e) {
			}

			socket = new MulticastSocket(this.port);
			socket.setNetworkInterface(NetworkInterface.getByName(this.interf));
			socket.joinGroup(this.mCastAdr);
		
			while (!this.isInterrupted()){
				byte[] buff = new byte[34];
				DatagramPacket packet = new DatagramPacket(buff, 34);
				socket.receive(packet);
				this.addMsg(new RcvObj(packet.getData(), this.rcvr.getTime()));
			}

			socket.close();
		} catch (SocketException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public synchronized List<RcvObj> getMsgs(long frameEndTime){
		List<RcvObj> rcvList = new ArrayList<>();
		while (this.msgs.peek() != null && (this.msgs.peek().getTimeRcvd() <= frameEndTime))
			rcvList.add(this.msgs.poll());

		return rcvList;
	}
	
	public synchronized void addMsg(RcvObj obj){
		this.msgs.add(obj);
	}
	
	public int getMsgSize(){
		return this.msgs.size();
	}
}