package vsp1;

import java.rmi.Naming;

public class HelloClient {

  public static void main(String args[]) {

    try {
      MessageService server = (MessageService)Naming.lookup("hello-server");
      String result = server.sayHello();
      System.out.println(result);
    } catch(Exception ex) {
      ex.printStackTrace();
    }
  }

}
