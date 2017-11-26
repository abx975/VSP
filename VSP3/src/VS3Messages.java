import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.lang.IllegalArgumentException;
import java.util.Arrays;

/**
 * Helper class for managing messages adhering to protocol specified in VS-Lab3
 * 
 * @author Lennart Hartmann, Nils Eggebrecht
 * @version 26.11.2017
 */
public class VS3Messages {

	public static final int MESSAGE_SIZE = 34;
	public static final int STATION_TYPE_IDX = 0;
	public static final int PAYLOAD_START = 1;
	public static final int PAYLOAD_END = 24;
	public static final int NEXT_SLOT_IDX = 25;
	public static final int SEND_TIME_START = 26;
	public static final int SEND_TIME_END = 33;
	public static final int ZERO_IDX = 0;

	/**
	 * Retrieves the sending station's type
	 * 
	 * @param input
	 *            the stream to be checked
	 * @return the sender's class
	 * @throws IllegalArgumentException
	 */
	public static char getStationClass(byte[] input)
			throws IllegalArgumentException {
		if (input.length != MESSAGE_SIZE) {
			throw new IllegalArgumentException(
					"A valid message needs to have 34 bytes");
		}
		if (input[ZERO_IDX] == 'A') {
			return 'A';
		} else if (input[ZERO_IDX] == 'B') {
			return 'B';
		} else {
			throw new IllegalArgumentException(
					"Station type needs to be 'A' or 'B'");
		}
	}

	/**
	 * Retrieves the message public static final int SEND_TIME_END = 33; 's
	 * payload
	 * 
	 * @param input
	 *            the stream to be checked
	 * @return the payload
	 */
	public static byte[] getPayload(byte[] input) {
		if (input.length != MESSAGE_SIZE) {
			throw new IllegalArgumentException(
					"A valid message needs to have 34 bytes");
		}
		return Arrays.copyOfRange(input, PAYLOAD_START, PAYLOAD_END);
	}

	/**
	 * Retrieves the message's payload as String
	 * 
	 * @param input
	 *            the stream to be checked
	 * @return the payload
	 */
	public static String getString(byte[] input) {
		return new String(getPayload(input));
	}

	/**
	 * Retrieves the slot the message's sender intends to use next
	 * 
	 * @return the slot intended
	 */
	public static int getNextSlotScheduled(byte[] input) {
		if (input.length != MESSAGE_SIZE) {
			throw new IllegalArgumentException(
					"A valid message needs to have 34 bytes");
		}
		return (int) input[NEXT_SLOT_IDX];
	}

	/**
	 * Retrieves the time the message was sent
	 * 
	 * @return the slot intended
	 */
	public static long getSendTime(byte[] input) {
		if (input.length != MESSAGE_SIZE) {
			throw new IllegalArgumentException(
					"A valid message needs to have 34 bytes");
		}
		byte[] time = Arrays.copyOfRange(input, SEND_TIME_START, SEND_TIME_END);

		long retval = 0;
		for (int i = 0; i < input.length; i++) {
			retval = (retval << 8) + (input[i] & 0xff);
		}

		return retval;

		// return Long.parseLong(new String(time));
	}

	/**
	 * Creates a new message according to protocol
	 * 
	 * @param type
	 *            the sending station's type
	 * @param message
	 *            the payload as a String
	 * @param nextSlot
	 *            the next slot intended to be used by sender
	 * @param sendTime
	 *            the time the message was sent
	 * @return the message created
	 */
	public static byte[] getMessage(char type, String payload, byte nextSlot,
			long sendTime) {
		byte[] sendTimeBytes;
		byte[] msg = new byte[MESSAGE_SIZE];

		// convert long to byte[]
		ByteArrayOutputStream baos = new ByteArrayOutputStream();
		DataOutputStream dos = new DataOutputStream(baos);
		try {
			dos.writeLong(sendTime);
		} catch (IOException e) {
			e.printStackTrace();
		}
		try {
			dos.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		sendTimeBytes = baos.toByteArray();
		try {
			baos.close();
		} catch (IOException e) {
			e.printStackTrace();
		}

		// write type
		msg[STATION_TYPE_IDX] = (byte) type;

		// write payload
		System.arraycopy(payload.getBytes(), ZERO_IDX, msg, PAYLOAD_START,
				payload.length());

		// write nextSlot
		msg[NEXT_SLOT_IDX] = (byte) nextSlot;

		// write sendtime
		System.arraycopy(sendTimeBytes, ZERO_IDX, msg, SEND_TIME_START,
				sendTimeBytes.length);

		// //reverse byte order: required?
		// byte[] reverseMsg = new byte[MESSAGE_SIZE];
		// for(int i=0; i< MESSAGE_SIZE; i++){
		// reverseMsg[i] = msg[(MESSAGE_SIZE-1)-i];
		// }
		// return reverseMsg;
		return msg;
	}

	/**
	 * Creates a new message according to protocol
	 * @param type
	 *            the sending station's type
	 * @param message
	 *            the payload
	 * @param nextSlot
	 *            the next slot intended to be used by sender
	 * @param sendTime
	 *            the time the message was sent
	 * @return the message created
	 */
	public static byte[] getMessage(char type, byte[] payload, byte nextSlot,
			long sendTime) {
		byte[] sendTimeBytes;
		byte[] msg = new byte[MESSAGE_SIZE];

		// convert long to byte[]
		ByteArrayOutputStream baos = new ByteArrayOutputStream();
		DataOutputStream dos = new DataOutputStream(baos);
		try {
			dos.writeLong(sendTime);
		} catch (IOException e) {
			e.printStackTrace();
		}
		try {
			dos.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		sendTimeBytes = baos.toByteArray();
		try {
			baos.close();
		} catch (IOException e) {
			e.printStackTrace();
		}

		// write type
		msg[STATION_TYPE_IDX] = (byte) type;

		// write payload
		System.arraycopy(payload, ZERO_IDX, msg, PAYLOAD_START, payload.length);

		// write nextSlot
		msg[NEXT_SLOT_IDX] = (byte) nextSlot;

		// write sendtime
		System.arraycopy(sendTimeBytes, ZERO_IDX, msg, SEND_TIME_START,
				sendTimeBytes.length);
		return msg;
	}
}