/**
 * Verteilte Systeme Aufgabe 1: Client/Server-Anwendung 
 * "Verteilte Nachrichten-Queue"
 * Gruppe 3
 * Nils.eggebrecht@haw-hamburg.de
 * Lennart.hartmann@haw-hamburg.de
 */

import java.sql.Timestamp;
import java.util.Date;

/**
 * Haelt Inhalt und Verwaltungsinformationen einer Nachricht
 * @author Nils Eggebrecht
 * @author Lennart Hartmann
 * @version 06.10.2017
 */
public class Message {
    /*
    Laufende Nummer, die die reihenfolge des Eintreffens beim Server 
    repräsentiert
    */
    int messageId;
    /*
    Die IP-Adresse des Absenders
    */
    String clientId;
    /*
    Der Inhalt der Nachricht
    */
    String message;
    /*
    Der Zeitpunkt des Eintreffens
    */
    Timestamp timestamp;

    /**
     * @param messageIdlLaufende Nummer der Nachricht
     * @param clientId  IP-Adresse des Absenders
     * @param message   Inhalt der Nachricht
     * @param timestamp Der Zeitpunkt des Eintreffens der Nachricht
     */
    public Message(int messageId, String clientId, String message, Timestamp timestamp) {
        this.messageId = messageId;
        this.clientId = clientId;
        this.message = message;
        this.timestamp = timestamp;
    }

    @Override
    public String toString() {
        return this.messageId + " " + this.clientId + ": " + this.message + " " + this.timestamp;
    }
}
