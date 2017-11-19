package src.sender;

import java.io.IOException;

public class SendBuff extends Thread {

	private byte[] data;

	public SendBuff() {
		this.data = new byte[23];
	}

	public synchronized byte[] getData() {
		return this.data;
	}

	public byte[] getSourceData() {
		byte[] res = new byte[24];
		int i = 0;
		try {
			while (i <= 23)
				i += System.in.read(res, i, 24 - i);
		} catch (IOException e) {
			e.printStackTrace();
		}
		return res;
	}

	public synchronized void setData(byte[] data) {
		this.data = data;
	}

	public void run() {
		super.run();
		while (!this.isInterrupted())
			this.setData(getSourceData());
	}
}
