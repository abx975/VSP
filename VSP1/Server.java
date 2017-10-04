
import java.rmi.server.UnicastRemoteObject;
import java.rmi.RemoteException;
import java.rmi.Naming;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.sql.Timestamp;
import java.util.Date;
import java.util.HashMap;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.Iterator;
import java.util.logging.Level;
import java.util.logging.Logger;

//import HelloServer;
public class Server extends UnicastRemoteObject implements MessageService {

    static int messageID = 0;
    int t;
    String inactiveClient;
    LinkedBlockingQueue<Message> fifo;
    HashMap<String, Message> lastMessagesDelivered;
    HashMap<String, Timestamp> latestClientAcceses;
    Iterator<Message> it;
    Iterator<String> iter;

    public Server() throws RemoteException {
        int deliveryQueueSize = 100;
        this.fifo = new LinkedBlockingQueue(deliveryQueueSize);
        this.t = 600000; // 10min
        this.lastMessagesDelivered = new HashMap<>();
        this.latestClientAcceses = new HashMap<>();
    }

    public Server(int t, int deliveryQueueSize) throws RemoteException {
        this.fifo = new LinkedBlockingQueue(deliveryQueueSize);
        this.t = t;
        this.lastMessagesDelivered = new HashMap<>();
        this.latestClientAcceses = new HashMap<>();
    }

    public String sayHello() {
        return "Hello World";
    }

    @Override
    public String nextMessage(String clientID) throws RemoteException {
        recordAccessTime(clientID);
        it = fifo.iterator();
        String retVal = null;
        Message msg;
        while (it.hasNext()) {
            msg = it.next();
            if (msg.messageId == (lastMessagesDelivered.get(clientID).messageId)) {
                retVal = msg.toString();
                lastMessagesDelivered.put(clientID, msg);
                return retVal;
            }
        }
        return retVal;
    }

    /**
     *
     * @param clientID
     * @param message
     * @throws RemoteException
     */
    @Override

    public void newMessage(String clientID, String message) throws RemoteException {

        recordAccessTime(clientID);

        if (fifo.remainingCapacity() == 0) {
            fifo.poll();
        }
        try {
            fifo.put(new Message(getNextMessageId(), clientID, message, new Timestamp(new Date().getTime())));
        } catch (InterruptedException ex) {
            Logger.getLogger(Server.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    private int getNextMessageId() {
        messageID++;
        return messageID;
    }

    private void recordAccessTime(String clientID) {

        latestClientAcceses.put(clientID, new Timestamp(new Date().getTime()));
        Timestamp currentTime_minus_T = new Timestamp(new Date().getTime() - t);
        try {
            if (currentTime_minus_T.after(latestClientAcceses.get(inactiveClient))) {
                latestClientAcceses.remove(inactiveClient);
                lastMessagesDelivered.remove(inactiveClient);
                getNewInactiveClient();
            }
        } catch (NullPointerException ne) {
            ne.printStackTrace();
        }
    }

    private void getNewInactiveClient() {
        Timestamp oldestTime;
        String currentClient;
        String oldestClientKey = null;

        oldestTime = new Timestamp(new Date().getTime());
        iter = latestClientAcceses.keySet().iterator();
        while (iter.hasNext()) {
            currentClient = iter.next();
            if (latestClientAcceses.get(currentClient).before(oldestTime)) {
                oldestTime = latestClientAcceses.get(currentClient);
                oldestClientKey = currentClient;
            }
        }
        inactiveClient = oldestClientKey;
    }

    public static void main(String args[]) {
        System.out.println("server läuft");
        int t = 600000;
        int deliveryQueueSize = 100;

        try {
            LocateRegistry.createRegistry(Registry.REGISTRY_PORT);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        try {
            //Naming.rebind("MessageService", new Server(t,deliveryQueueSize));
            Naming.rebind("MessageService", new Server());
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }
}
