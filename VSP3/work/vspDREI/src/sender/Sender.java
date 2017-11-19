package src.sender;

import src.receiver.Receiver;

import java.io.IOException;
import java.net.*;
import java.nio.ByteBuffer;

public class Sender extends Thread {

	private int port;
	private InetAddress mCastAdr;
	private SendBuff sendBuff;
	private Receiver rcvr;
	private char type;
	private boolean isSending;
	private long frameEndTime;
	private String interf;
	private MulticastSocket socket;
	
	
	public Sender(String mCastAdr, int port, Receiver rec, char type, String interf) throws IOException {
		this.mCastAdr = InetAddress.getByName(mCastAdr);
		this.port = port;
		this.sendBuff = new SendBuff();
		this.rcvr = rec;
		sendBuff.start();
		this.type = type;
		this.interf = interf;
		this.setPriority(Thread.MAX_PRIORITY);
		socket = new MulticastSocket(this.port);
		socket.setTimeToLive(1);
		socket.joinGroup(this.mCastAdr);
		socket.setNetworkInterface(NetworkInterface.getByName(this.interf));
	}

	public void send() throws SocketException {
		this.isSending = false;
		try {
			int time2Wait = (int)(this.frameEndTime - (this.rcvr.getTime() +  this.rcvr.SLOTMS / 2));
			if (time2Wait > 0)
				Thread.sleep(time2Wait);
		} catch (InterruptedException e1) {
			this.interrupt();
		}
		if (!this.rcvr.collision()) {
			try {
				if (timeToSend()) {
					this.isSending = true;
					byte[] toSend = this.prepareMsg();
					socket.send(new DatagramPacket(toSend, toSend.length, this.mCastAdr, port));
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
	
	private boolean timeToSend() {
		long time = this.rcvr.getTime();
		return time < this.frameEndTime;
	}

	public byte[] prepareMsg() {
		byte[] data = new byte[34];
		byte[] msg = this.sendBuff.getData();
		int slot = this.rcvr.getFreeSlotNextFrame();
		Long now = this.rcvr.getTime();
		ByteBuffer bBuff = ByteBuffer.allocate(8);
		byte[] nowAsBytesArr = bBuff.putLong(now).array();
		data[0] = (byte) this.type;
		for (int i = 0; i < msg.length; i++)
			data[i + 1] = msg[i];
		data[25] = (byte) slot;
		for (int i = 0; i < nowAsBytesArr.length; i++)
			data[i + 26] = nowAsBytesArr[i];
		return data;
	}


	@Override
	public synchronized void run() {
		super.run();
		while (!this.isInterrupted()) {
			try {
				this.wait();
				try {
					this.send();
				} catch (SocketException e) {
					e.printStackTrace();
				}
			} catch (InterruptedException e) {
				this.interrupt();
			}
		}
		socket.close();
		this.sendBuff.interrupt();
	}

	public boolean isSending() {
		return this.isSending;
	}

	public synchronized void triggerSender(long frameEndTime) {
		this.frameEndTime = frameEndTime;
		this.notify();		
	}
}
