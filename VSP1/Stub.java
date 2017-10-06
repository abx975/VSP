/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


import java.rmi.RemoteException;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.Iterator;
import java.util.NoSuchElementException;

/**
 *
 * @author lennarthartmann
 */
public class Stub {

    String msg = "It's my stub";
    LinkedBlockingQueue<String> lbq;
    Iterator<String> it;

    public Stub() {
        lbq = new LinkedBlockingQueue<>();

        try {
            lbq.put("s1");
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        try {
            lbq.put("s2");
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        try {
            lbq.put("s3");
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
    
    public void printAll(){
        it=lbq.iterator();
        String msg=null;
        while(it.hasNext()){
            msg = it.next();
            System.out.println(msg);
        }
        try{
            System.out.println(it.next());
        }catch(NoSuchElementException e){
            System.out.println("kein nächstes vorhanden!");
        }
    }
    
    
    public String nextMessage(String clientID) throws RemoteException{
        return lbq.poll();
    }

    public void newMessage(String clientID, String message) throws RemoteException{
        try {
            lbq.put(message);
        } catch (InterruptedException e) {
            System.out.println("Error: Queue voll");
        }
    }
    

    @Override
    public String toString() {
        return msg;
    }
}
