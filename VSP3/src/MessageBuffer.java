import java.io.IOException;

/**
 * Abstraction from frame source for messages adhering to protocol for testing purposes
 * @author lennarthartmann
 * @version 24.11.2017
 */
public class MessageBuffer extends Thread{
	public static final int MSG_SIZE = 34;
	private byte[] currentFrame;
	private byte[] testFrame;
	
	private static MessageBuffer readerInstance=null;
	
	/**
	 * Constructor
	 */
	private MessageBuffer(){
		currentFrame="Aregular message_________112345678".getBytes();
	}
	
	/**
	 * Gives reference to already running SourceReader
	 * @return the sourceReader
	 */
	public static MessageBuffer getInstance(){
//		if(readerInstance.equals(null)){
		if(readerInstance==null){
			readerInstance = new MessageBuffer();
			readerInstance.start();
		}
		return readerInstance;
	}
	
	/**
	 * Sets a frame explicitly
 	 * @param frame the desired Frame
	 */
	public synchronized void setFrame(byte[] frame){
		currentFrame=frame;
	}
	
	/**
	 * Gets a previously set frame
	 * @return a new frame
	 */
	public synchronized byte[] getFrame(){
		return currentFrame;
	}
	
	/**
	 * Gets a frame previously read from message source
	 * @return a frame from testing frame
	 */
	public synchronized byte[] getTestframe(){
		testFrame="its a testframe__________112345678".getBytes();//stub
				   
		return testFrame;
	}
	
	/**
	 * reads a frame from message source adhering to protocol
	 * @return a frame from testing frame
	 */
	private void readFrame(){
		testFrame=new byte[MSG_SIZE];
		int i=0;
		try{
			while(i<MSG_SIZE){
				i+=System.in.read(testFrame,i,MSG_SIZE-i);
			}
		}catch(IOException e){
			e.printStackTrace();
		}
	}
	
	public void run(){
		while(!this.isInterrupted()){
			//todo: uncomment readFrame();
			try{
				sleep(500);
			}catch(InterruptedException e){
				e.printStackTrace();
			}
			System.err.println("reader: System.in");
		}
	}
}
