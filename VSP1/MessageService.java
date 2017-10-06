/**
 * Verteilte Systeme Aufgabe 1: Client/Server-Anwendung 
 * "Verteilte Nachrichten-Queue"
 * Gruppe 3
 * Nils.eggebrecht@haw-hamburg.de
 * Lennart.hartmann@haw-hamburg.de
 */

import java.rmi.Remote;
import java.rmi.RemoteException;

/**
 * Interface fuer entfernte Methodenaufrufe am Nachrichten-Server 
 * @author Nils Eggebrecht
 * @author Lennart Hartmann
 * @version 06.10.2017
 */
public interface MessageService extends Remote {

    /**
     * Gibt alle Nachrichten auf der Konsole aus
     * @return  alle in der Queue noch gespeicherten Nachrichten
     * @throws RemoteException 
     */
    public String showAll() throws RemoteException;

    /**
     * Holt die naechste noch nicht gezeigte Nachricht mit at-most-once
     * Fehlersemantik
     * @param clientID  die IP-Adresse des Clients als String
     * @return  die aelteste noch nicht vom Client abgerufene Nachricht
     * @throws RemoteException 
     */
    public String nextMessage(String clientID) throws RemoteException;

    /**
     * Sendet eine neue Nachricht an den Server
     * @param clientID  die IP-Adresse des Clients als String
     * @param message   der Text der zu sendenen Nachricht
     * @throws RemoteException 
     */
    public void newMessage(String clientID, String message) throws RemoteException;
}
