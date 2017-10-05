
import java.rmi.Remote;
import java.rmi.RemoteException;

public interface MessageService extends Remote {

    public int add(int x, int y) throws RemoteException;

    public String nextMessage(String clientID) throws RemoteException;

    public void newMessage(String clientID, String message) throws RemoteException;
}
