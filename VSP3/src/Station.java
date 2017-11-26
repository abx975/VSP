import java.util.Observable;
import java.util.Observer;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * Provides a node for communication via simulated wireless using TCP/IP multicast
 * @author lennarthartmann
 *
 */
public class Station implements Observer{
	//constants
	public static final int ARGS_EXPECTED = 5;
	public static final int NO_OF_THREADS = 24;
	public static final long FRAME_WIDTH = 1000;
	public static final int TIMESLOT_WIDTH = 40;
	public static final int TIMESLOT_OFFSET_MIDDLE = TIMESLOT_WIDTH/2;
	public static final int NUMBER_OF_TIMESLOTS = 24;
	public static final long ACCEPTABLE_DELTA = 5;
	
	//default values
	public static final char defaultType = 'A';
	public static final String defaultInterface = "wlp4s0";
	public static final String default_Host = "225.10.1.2";
	public static final String defaultPort = "15000";
	public static final String defaultClockCorrection = "0";
	//variables	
	private char stationType = defaultType;
	private long localTimeOffset=0;
	private int timeslot=0;
	private ScheduledThreadPoolExecutor threadpool;
	private Receiver receiver;
	private Sender sender;
	private Thread rcvThread;
	private Thread sendThread;
	private long[] timeslotsAccessTimes;
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
		timeslotsAccessTimes = new long[NUMBER_OF_TIMESLOTS];
	}

	/**
	 * Displays incoming message and checks synchronization
	 */
	public void update(Observable receiver, Object pckg) {
		byte[] msg = (byte[])pckg;
		
		checkSnchronization(msg);
		if(this.stationType == 'B'){
			synchronizeClock(msg);
		}
		
		System.err.println("got message: " + VS3Messages.getString(msg));
	}
	
	/**
	 * Gets the current system time with simulated inaccuracy
	 * @return the system time
	 */
	private long getCurrentSystemTime(){
		return System.currentTimeMillis()+localTimeOffset;
	}
	
	/**
	 * Checks whether another timeslot needs to be chosen to avoid future collisions
	 * Updates accordingly.
	 * @param pckg	the latest package received
	 */
	private void checkSnchronization(byte[] pckg){
		//check for senders timeslot for collision
		int senderSlot = VS3Messages.getNextSlotScheduled(pckg);
		if(timeslot == senderSlot){
			timeslotsAccessTimes[senderSlot] = VS3Messages.getSendTime(pckg);
			long leastRecentAccess = Long.MAX_VALUE;
			int leastRecentTimeslot = 0;
			//look least recently used timeslot
			for(int i=0; i < timeslotsAccessTimes.length; i++){
				if(timeslotsAccessTimes[i] < leastRecentAccess){
					leastRecentAccess = timeslotsAccessTimes[i];
					leastRecentTimeslot = i;
				}
			}
			this.timeslot = leastRecentTimeslot;
			reschedule();
		}
	}
	
	/**
	 * Synchronizes simulated local clock with senders typeA clock if applicable
	 * @param pckg	the incoming package
	 */
	private void synchronizeClock(byte[] pckg){
		//check whether source clock is more reliable than own
		if(Math.abs(getCurrentSystemTime() - VS3Messages.getSendTime(pckg)) > ACCEPTABLE_DELTA 
				&& VS3Messages.getStationClass(pckg) == 'A'){
			localTimeOffset = VS3Messages.getSendTime(pckg) - getCurrentSystemTime();
			reschedule();
		}
	}
	
	/**
	 * Updates schedule for sending messages. Uses simulated local time
	 */
	private void setSchedule(long startTime,long interval){
		threadpool.scheduleAtFixedRate(sender, startTime, interval, TimeUnit.MILLISECONDS);
	}
	
	/**
	 * Updates scheduling with simulated local time and selected timeslot
	 */
	private void reschedule(){
		//if too late resume in next frame
		long startTime = localTimeOffset >= 0 ? localTimeOffset : FRAME_WIDTH - localTimeOffset;
		//apply offset to middle of selected timeslot
		startTime += (TIMESLOT_WIDTH * timeslot + TIMESLOT_OFFSET_MIDDLE);
		threadpool.scheduleAtFixedRate(sender, startTime, FRAME_WIDTH, TimeUnit.MILLISECONDS);
	}
	
	
	public static void main(String[] args) {
		if(args.length!=ARGS_EXPECTED){
			args=new String[ARGS_EXPECTED];
			args[0]=defaultType+"";;
			args[1]=defaultInterface;
			args[2]=default_Host;
			args[3]=defaultPort;
			args[4]=defaultClockCorrection;
		}

		Station station=new Station(args[0].charAt(0),args[2], Integer.parseInt(args[3]),"localhost", Long.parseLong(args[4]));
		station.setSchedule(0, 2000);
		station.rcvThread.start();
		MessageBuffer.getInstance();
		try {station.sendThread.join();}catch(Exception e) {System.out.print("Thread terminated -> done");};
	}
}
