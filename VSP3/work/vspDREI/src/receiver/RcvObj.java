package src.receiver;

public class RcvObj {
	
	private byte[] msg;
	private long timeRcvd;

	public RcvObj(byte[] msg, long timeRcvd) {
		super();
		this.msg = msg;
		this.timeRcvd = timeRcvd;
	}
	
	public byte[] getMsg() {
		return this.msg;
	}

	public long getTimeRcvd() {
		return this.timeRcvd;
	}
}
