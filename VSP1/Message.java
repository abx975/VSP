/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

import java.sql.Timestamp;
import java.util.Date;

/**
 *
 * @author nilsegge
 */
public class Message {

    int messageId;
    String clientId;
    String message;
    Timestamp timestamp;

    /**
     *
     * @param messageId ID der Nachricht
     * @param clientId ID des Clients
     * @param message Nachricht
     * @param timestamp Uhrzeit der Nachricht
     */
    public Message(int messageId, String clientId, String message, Timestamp timestamp) {
        this.messageId = messageId;
        this.clientId = clientId;
        this.message = message;
        this.timestamp = timestamp;
    }

    @Override
    public String toString() {
        return this.messageId + " " + this.clientId + ": " + this.message + this.timestamp;
    }
}
