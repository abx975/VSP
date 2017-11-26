import java.io.IOException;
import java.util.Observable;
import java.util.Observer;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

import java.util.Arrays;

/**
 * Provides a node for communication via simulated wireless using TCP/IP multicast
 * @author lennarthartmann
 *
 */
public class Station implements Observer{
	//constants
	public static final int ARGS_EXPECTED = 5;
	public static final int NO_OF_THREADS = 24;
	public static final int TIMESLOT_WIDTH = 40;
	public static final int TIMESLOT_OFFSET_MIDDLE = TIMESLOT_WIDTH/2;
	//default values
	public static final String defaultType = "A";
	public static final String defaultInterface = "wlp4s0";
	public static final String default_Host = "225.10.1.2";
	public static final String defaultPort = "15000";
	public static final String defaultClockCorrection = "0";
	
	//variables	
	private int timeslot;
	private ScheduledThreadPoolExecutor threadpool;
	private Receiver receiver;
	private Sender sender;
	private Thread rcvThread;
	private Thread sendThread;
	
	/**
	 * Constructor
	 */
	public Station(char stationType, String mCastIP, int port, String interf, long clockCorrection){
		threadpool = new ScheduledThreadPoolExecutor(NO_OF_THREADS);
		receiver = new Receiver(stationType, interf, mCastIP, port, clockCorrection);
		sender = new Sender(mCastIP, port, interf);
		rcvThread = new Thread(receiver);
		sendThread = new Thread(sender);
		receiver.addObserver(this);
	}

	/**
	 * Defines behaviour for incoming message event
	 */
	public void update(Observable receiver, Object payload) {
		// TODO Auto-generated method stub
		timeslot=1;
		System.err.println("Station: got Update");
		byte[] msg = (byte[])payload;
		System.err.println("got message: "+new String(msg));
		//setSchedule(startTime, interval);
	}

	
	/**
	 * Updates schedule for sending messages
	 */
	private void setSchedule(long startTime,long interval){
		threadpool.scheduleAtFixedRate(sender, startTime, interval, TimeUnit.MILLISECONDS);
	}
	
	
	public static void main(String[] args) {
		if(args.length!=ARGS_EXPECTED){
			args=new String[ARGS_EXPECTED];
			args[0]=defaultType;
			args[1]=defaultInterface;
			args[2]=default_Host;
			args[3]=defaultPort;
			args[4]=defaultClockCorrection;
		}

		byte[] msg = MessageBuffer.getInstance().getFrame();
//		long lng = Long.parseLong(new String(Arrays.copyOfRange(msg, 26, 33)));
//		System.err.println("sendtime: " + lng);
		
		//public Station(char stationType, String mCastIP, int port, String interf, long clockCorrection)
		Station station=new Station(args[0].charAt(0),args[2], Integer.parseInt(args[3]),"localhost", Long.parseLong(args[4]));
		station.setSchedule(0, 2000);
		station.rcvThread.start();
		MessageBuffer.getInstance();
		try {station.sendThread.join();}catch(Exception e) {System.out.print("Thread terminated -> done");};
	}
}
