package src.receiver;

import src.sender.Sender;
import java.io.IOException;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Receiver extends Thread {
	
	public static final long SLOTMS = 40;
	public static final long FRAMEMS = 1000;
	public static final int SLOTS =(int) (FRAMEMS / SLOTMS);

	private String interf;
	private int port;
	private long offset;
	private List<RcvObj> msgs;
	private InetAddress mCastAdr;
	private int[] reservedSlots;
	private MulticastSocket socket;
	private int nextSlot2Send;
	private int tmpSlot;
	private Sender sender;
	private boolean isSending;
	private int syncCnt = 0;
	private int sumOffset = 0;
	private RcvThread rcvThread;
	
	public static void main(String[] args) throws NumberFormatException, SecurityException, IOException{
        if (args.length < 1) {
            args = new String[6];
            args[0] = "A"; // station
 //           args[1] = "enp0s25"; // interface
            args[1] = "wlp4s0";
            args[2] = "225.10.1.2"; // host
            args[3] = "15000"; // port
            args[4] = "0"; // offset
        }
        Receiver receiver = new Receiver(args[0].charAt(0), args[1], args[2], Integer.parseInt(args[3]), Long.parseLong(args[4]));
		receiver.start();
	}
	
	public Receiver(char type, String interf, String mCastAdr, int port, long offset) throws SecurityException, IOException {
		super();
		this.interf = interf;
		this.port = port;
		this.offset = offset;
		this.msgs = new ArrayList<>();
		this.setPriority(Thread.MAX_PRIORITY);
		this.mCastAdr = InetAddress.getByName(mCastAdr);
		this.reservedSlots = new int[SLOTS];
		Sender sender = new Sender(mCastAdr, this.port, this, type, this.interf);
		sender.start();
		this.sender = sender;
		this.nextSlot2Send = -1;
		this.rcvThread = new RcvThread(this.port, this.interf, this.mCastAdr, this);
	}

	public int getFreeSlotNextFrame() {
		if (this.nextSlot2Send == 0){
			this.nextSlot2Send = this.searchNextSlot();
		}
		return nextSlot2Send;
	}

	private int searchNextSlot() {
		for (int i = 0; i < this.reservedSlots.length; i++)
			if (this.reservedSlots[i] == 0)
				return i+1;
		return -1;
	}

	public long getTime() {
		return System.currentTimeMillis() + this.offset;
	}

	public boolean collision() {
		return this.rcvThread.getMsgSize() >= 1;
	}

	public void run() {
		super.run();
		try {	
			this.rcvThread.start();
			try {
				Thread.sleep(1000 - (this.getTime() % 1000));
			} catch (InterruptedException e) {
			}
			
			socket = new MulticastSocket(this.port);
			socket.setNetworkInterface(NetworkInterface.getByName(this.interf));
			socket.joinGroup(this.mCastAdr);
			this.listenOnFrame();
		
			while (!this.isInterrupted()){
				this.listenOnFrame();
			}
			this.sender.interrupt();
			socket.close();
		} catch (SocketException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	private void listenOnFrame() throws IOException {
		long now = this.getTime();
		for(int i = 0; i < SLOTS; i++){
			long frameEndTime = now - (now % 1000) + (i+1) * SLOTMS;
			int slotNr = i+1;
			listenOnSlot(frameEndTime, slotNr);
		}
		if (!this.isSending)
			this.nextSlot2Send = this.getFreeSlot();
		this.tmpSlot = this.nextSlot2Send;
		this.reservedSlots = new int[SLOTS];
		this.isSending = false;
		if (syncCnt > 0){
			int newOffset = (this.sumOffset / this.syncCnt);
			this.offset = this.offset + Math.round((float)newOffset/4.0f);
		}
		
		this.syncCnt = 0;
		this.sumOffset = 0;
	}

	
	private void listenOnSlot(long frameEndTime, int slotNr) throws IOException {
		if (slotNr == this.nextSlot2Send){
			this.nextSlot2Send = 0;
			sender.triggerSender(frameEndTime);
		}
		int time2Wait = (int) (frameEndTime - this.getTime());
		try {
			if (time2Wait > 0){
				Thread.sleep(time2Wait);
			}
		} catch (InterruptedException e) {
			this.interrupt();
		}
			
		this.msgs = this.rcvThread.getMsgs(frameEndTime-1);
		this.handleMessage(slotNr);
	}

	private void handleMessage(int slotNr) {
		if (this.msgs.size() > 1)
			this.msgs.clear();
		else if (this.msgs.size() == 1) {
			if (this.tmpSlot == slotNr && this.sender.isSending())
				this.isSending = true;
			byte[] msg = this.msgs.get(0).getMsg();
			ByteBuffer bBuff = ByteBuffer.wrap(Arrays.copyOfRange(msg, 26, 34));
			this.sync((char) msg[0], bBuff.getLong(), this.msgs.get(0).getTimeRcvd());
			this.reservedSlots[msg[25]-1] = 1;
			this.msgs.clear();
		}
	}

	private void sync(char type, long timestamp, long received) {
		if (type == 'A'){
			this.syncCnt++;
			long newOffset = (timestamp - received);
			this.sumOffset += newOffset;
		}
	}

	public int getFreeSlot() {
		List<Integer> freeSlots = new ArrayList<>();
		for (int i = 0; i < this.reservedSlots.length; i++){
			if (this.reservedSlots[i] == 0){
				freeSlots.add(i+1);
			}
		}
		if (freeSlots.size() > 0){
			int index = (int)(Math.random() * freeSlots.size());
			return freeSlots.get(index);
		}
		else {
			return 0;
		}
	}
}