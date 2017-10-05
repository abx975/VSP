
import java.rmi.Naming;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;

public class Client {

  public static void main(String args[]) {

    try {
      Registry registry = LocateRegistry.getRegistry();
      MessageService server = (MessageService)Naming.lookup("MessageService");
      String result;
      server.newMessage("client1", "Ich bin Peter");
      server.newMessage("client2", "Ich bin Hans");
      result = server.nextMessage("client1");
      System.out.println(result);
      result = server.nextMessage("client2");
      System.out.println(result);
      result = server.nextMessage("client2");
      System.out.println(result);
      result = server.nextMessage("client1");
      System.out.println(result);
        System.out.println("3 = 2 + 1 = " + server.add(2, 1));
      
    } catch(Exception ex) {
      ex.printStackTrace();
    }
  }

}
