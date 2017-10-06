
import java.rmi.server.UnicastRemoteObject;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.RemoteServer;
import java.sql.Timestamp;
import java.util.Date;
import java.util.HashMap;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.Iterator;
import java.util.NoSuchElementException;
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
        String retVal = "null junge";
        Message msg;
        while (it.hasNext()) {
            msg = it.next();
            try {
                if (null == lastMessagesDelivered.get(clientID)) {
                    retVal = msg.toString();
                    System.out.println("erste Nachricht retVal: " + msg);
                    lastMessagesDelivered.put(clientID, msg);
                    return retVal;

                } else if (msg.messageId == (lastMessagesDelivered.get(clientID).messageId)) {
                    try {
                        msg = it.next();
                        retVal = msg.toString();
                    } catch (NoSuchElementException e) {
                        retVal = null;
                    }
                    System.out.println("weitere Nachricht retVal: " + msg);
                    lastMessagesDelivered.put(clientID, msg);
                    return retVal;
                }

            } catch (NullPointerException ne) {
                ne.printStackTrace();
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
        System.out.println(message);
        if (fifo.remainingCapacity() == 0) {
            fifo.poll();
        }
        try {
            Message msg = new Message(getNextMessageId(), clientID, message, new Timestamp(new Date().getTime()));
            fifo.put(msg);
            //System.out.println("Fifosize++; fifosize = " + fifo.size() + " " + "die neuste Nachricht ist: " + msg);

        } catch (InterruptedException ex) {
            Logger.getLogger(Server.class
                    .getName()).log(Level.SEVERE, null, ex);
        }
    }

    private int getNextMessageId() {
        messageID++;
        return messageID;
    }

    private void recordAccessTime(String clientID) {
        try {
            if (inactiveClient.equals(clientID)) {
                getNewInactiveClient();
            }
        } catch (NullPointerException e) {
            inactiveClient = clientID;
            System.out.println("CATCH inactive Client wird gesezt!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        }
        try {
            latestClientAcceses.put(clientID, new Timestamp(new Date().getTime()));
            Timestamp currentTime_minus_T = new Timestamp(new Date().getTime() - t);
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
        System.out.println("Der Älteste ist: " + inactiveClient);
    }

    public static void main(String[] args) throws RemoteException {
        LocateRegistry.createRegistry(Registry.REGISTRY_PORT);

        Server server = new Server();
        // gibt die Lognachrichten jedes Aufrufs auf der Konsole aus, kann sicher auch in ein Log file schreiben
        // RemoteServer.setLog(System.out);
        Registry registry = LocateRegistry.getRegistry();
        registry.rebind("MessageService", server);
        System.out.println("Server angemeldet");
    }

    @Override
    public int add(int x, int y) {
        return x + y;
    }
}
