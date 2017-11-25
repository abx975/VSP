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
	public static final int NO_OF_THREADS = 24;
	public static final int TIMESLOT_WIDTH = 40;
	public static final int TIMESLOT_OFFSET_MIDDLE = TIMESLOT_WIDTH/2;
	
	//constants	
	private int timeslot;
	private ScheduledThreadPoolExecutor threadpool;
	private Receiver receiver;
	private Sender sender;
	private Thread rcvThread;
	private Thread sendThread;
	
	
	/**
	 * Constructor
	 */
	public Station(){
		threadpool = new ScheduledThreadPoolExecutor(NO_OF_THREADS);
		receiver = new Receiver();
		sender = new Sender();
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
		threadpool.scheduleAtFixedRate(new Sender(), startTime, interval, TimeUnit.MILLISECONDS);
	}
	
	
	public static void main(String[] args) {
//		byte[] msg = "Hallozusammen".getBytes();
//		byte[] msge = Arrays.copyOfRange(msg, 5,msg.length);
//		System.err.println("Char: " + new String(msge));

		byte[] msg = MessageBuffer.getInstance().getFrame();
		long lng = Long.parseLong(new String(Arrays.copyOfRange(msg, 26, 33)));
		System.err.println("sendtime: " + lng);
		
		////////////////////////////
		Station station=new Station();
		station.setSchedule(0, 2000);
		station.rcvThread.start();
		MessageBuffer.getInstance();
		try {station.sendThread.join();}catch(Exception e) {System.out.print("Thread terminated -> done");};
	}
}
