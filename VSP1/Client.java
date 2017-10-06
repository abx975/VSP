


import java.rmi.Naming;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;

public class Client {

    public static void main(String args[]) {

        try {
            Registry registry = LocateRegistry.getRegistry();
            MessageService server = (MessageService) Naming.lookup("MessageService");
            String result;
            server.newMessage("client1", "Nachricht1");
            result = server.nextMessage("client2");
            System.out.println("Client2 : " + result);
            server.newMessage("client1", "Nachricht4");
            server.newMessage("client1", "Nachricht5");
            server.newMessage("client1", "Nachricht6");
            result = server.nextMessage("client2");
            System.out.println("Client2 : " + result);
            server.newMessage("client1", "Nachricht7");
            server.newMessage("client1", "Nachricht8");
            server.newMessage("client1", "Nachricht9");
            server.newMessage("client1", "Nachricht10");
            result = server.nextMessage("client2");
            System.out.println("Client2 : " + result);
            server.newMessage("client1", "Nachricht11");
            server.newMessage("client1", "Nachricht13");
            server.newMessage("client1", "Nachricht14");
            server.newMessage("client1", "Nachricht15");
            server.newMessage("client1", "Nachricht16");
            server.newMessage("client1", "Nachricht17");
            server.newMessage("client1", "Nachricht18");
            server.newMessage("client1", "Nachricht19");
            server.newMessage("client2", "Nachricht20");
            result = server.nextMessage("client2");
            System.out.println("Client2 : " + result);
            server.newMessage("client3", "Nachricht19");
            server.newMessage("client4", "Nachricht20");
            result = server.nextMessage("client1");
            System.out.println("Client2 : " + result);
            //result = server.nextMessage("client2");
            //System.out.println("Client2 : " + result);

//            result = server.nextMessage("client1");
//            System.out.println("Client1 : " + result);
//            result = server.nextMessage("client1");
//            System.out.println("Client1 : " + result);
//            result = server.nextMessage("client1");
//            System.out.println("Client1 : " + result);
//            result = server.nextMessage("client1");
//            System.out.println("Client1 : " + result);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

}
