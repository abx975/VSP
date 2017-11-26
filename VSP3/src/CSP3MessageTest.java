import static org.junit.Assert.*;
import java.util.Arrays;
import org.junit.Test;

/**
 * JUnit test class for  CSP3MessageTest
 * 
 * @author Nils Eggebrecht
 * @version 26.11.2017
 */

public class CSP3MessageTest {

	@Test
	public void testGetStationClassByAorB() {

		char[] ch = { '0' };

		// StationClass = 'A'
		byte[] retval = VS3Messages.getMessage('A', "ABC", (byte) 1,
				(long) System.currentTimeMillis());
		ch[0] = VS3Messages.getStationClass(retval);
		assertEquals("ch[0] =! 'A'", ch[0], 'A');

		// StationClass = 'B'
		retval = VS3Messages.getMessage('B', "ABC", (byte) 1,
				(long) System.currentTimeMillis());
		ch[0] = VS3Messages.getStationClass(retval);
		assertEquals("ch[0] =! 'B'", ch[0], 'B');
	}

	@Test(expected = IllegalArgumentException.class)
	public void testGetStationClassByFoultValue() {
		byte[] retval = VS3Messages.getMessage('C', "ABC", (byte) 1,
				(long) System.currentTimeMillis());
		VS3Messages.getStationClass(retval);
	}

	@Test
	public void testGetPayload() {
		byte[] retval = VS3Messages.getMessage('A', "1234", (byte) 1,
				System.currentTimeMillis());
		byte[] ch = VS3Messages.getPayload(retval);

		byte[] a;
		a = new byte[23];
		a[0] = '1';
		a[1] = '2';
		a[2] = '3';
		a[3] = '4';
		for (int i = 0; i < ch.length; i++) {
			assertEquals("der Paylod ist nicht 1234", ch[i], a[i]);
		}
	}

	@Test
	public void testGetPayloadtobig() {
		byte[] retval = VS3Messages.getMessage('A',
				"1234567890123456789012345678", (byte) 1,
				System.currentTimeMillis());
		byte[] ch = VS3Messages.getPayload(retval);

		byte[] a;
		a = new byte[28];
		a[0] = '1';
		a[1] = '2';
		a[2] = '3';
		a[3] = '4';
		a[4] = '5';
		a[5] = '6';
		a[6] = '7';
		a[7] = '8';
		a[8] = '9';
		a[9] = '0';
		a[10] = '1';
		a[11] = '2';
		a[12] = '3';
		a[13] = '4';
		a[14] = '5';
		a[15] = '6';
		a[16] = '7';
		a[17] = '8';
		a[18] = '9';
		a[19] = '0';
		a[20] = '1';
		a[21] = '2';
		a[22] = '3';
		a[23] = '4';

		for (int i = 0; i < ch.length; i++) {
			assertEquals("die ersten 23byte sind nicht gleich", ch[i], a[i]);
		}
	}

	@Test
	public void testGetString() {
		String s = "1234                   ";
		byte[] retval = VS3Messages.getMessage('A', s, (byte) 1,
				System.currentTimeMillis());
		String str = VS3Messages.getString(retval);
		assertEquals("Die Nachricht ist zu klein", s, str);
	}

	@Test
	public void testgetNextSlotScheduled() {
		String s = "1234                   ";
		byte[] retval = VS3Messages.getMessage('A', s, (byte) 2,
				System.currentTimeMillis());
		int i = VS3Messages.getNextSlotScheduled(retval);
		assertEquals("Die Nachricht ist zu klein", i, 2);
	}

	@Test
	public void testgetSendTime() {
		long time = System.currentTimeMillis();
		byte[] retval = VS3Messages.getMessage('A', "1234", (byte) 1, time);
		long timeread = 0;
		System.out.println("print time: ");
		timeread = VS3Messages.getSendTime(retval);
		System.out.println(timeread);
		System.out.println(time);
		assertEquals("Die Nachricht ist zu klein", timeread, time);
	}
	
}
