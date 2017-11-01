
/**
 * Verteilte Systeme Aufgabe 1: Client/Server-Anwendung "Verteilte
 * Nachrichten-Queue" Gruppe 3 Nils.eggebrecht@haw-hamburg.de
 * Lennart.hartmann@haw-hamburg.de
 */
import java.net.InetAddress;
import java.net.MalformedURLException;
import java.net.UnknownHostException;
import java.rmi.RemoteException;
import java.rmi.Naming;
import java.rmi.NotBoundException;
import java.rmi.registry.Registry;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.application.Application;
import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.TextField;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.StackPane;
import javafx.stage.Stage;

/**
 * Client mit GUI als Front End zur Nutzung eines Servers zum Schreiben/Abrufen
 * von Nachrichten unter Nutzung von Java RMI
 *
 * @author Nils Eggebrecht
 * @author Lennart Hartmann
 * @version 06.10.2017
 */
public class Client extends Application {

    static MessageService server;
    static Registry registry;
    public static int MAX_FAILIURE_TIME = 5000;
    private int failiureTime = 0;
    private String CLIENT_ID;

    String currentMsg;
    //Stub stub = new Stub();
    Button receiveBtn = new Button("receive next message");
    Button sendBtn = new Button("send Message");
    Button connectBtn = new Button("connect");
    Button showAllBtn = new Button("show all");

    public Client() throws UnknownHostException {
        CLIENT_ID = InetAddress.getLocalHost().toString();
        CLIENT_ID = CLIENT_ID.split("/")[1];
        System.out.println("Client created: " + this);
    }

    /**
     * Verschickt eine neue Nachricht an einen Message Client mit
     * at-least-once-Semantik durch periodische Wiederholung im Fehlerfall
     *
     * @return
     */
    private synchronized void sendMsg() {
        System.out.println(currentMsg);

        boolean exceptionPending = true;
        failiureTime = 0;
        while (exceptionPending && failiureTime < MAX_FAILIURE_TIME) {
            try {
                server.newMessage(this.toString(), currentMsg);
                exceptionPending = false;

            } catch (RemoteException ex) {
                Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
                exceptionPending = true;
                try {
                    this.wait(500);
                    failiureTime += 500;
                } catch (InterruptedException ex1) {
                    Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex1);

                }
            }

        }
        if (exceptionPending == true) {
            System.out.println("Connection lost, please try to connect again!");
        }

    }

    /**
     * Erzeugt die Grafische Oberflaeche
     *
     * @param primaryStage GUI Einstieg
     */
    @Override
    public void start(Stage primaryStage) {
        //Container fuer grundlegendes Layout
        GridPane grid = new GridPane();
        grid.setPadding(new Insets(10, 10, 10, 10));
        grid.setVgap(5);
        grid.setHgap(5);
        //Textfeld fuer abgeholte Nachricht
        final TextField nextMsgField = new TextField();
        nextMsgField.setMinWidth(400);
        nextMsgField.setPromptText("");
        nextMsgField.setPrefColumnCount(10);
        nextMsgField.getText();
        GridPane.setConstraints(nextMsgField, 0, 0);
        grid.getChildren().add(nextMsgField);
        //Textfeld fuer abzuschickende NAchricht
        final TextField newMsgField = new TextField();
        newMsgField.setPromptText("Enter your message here");
        GridPane.setConstraints(newMsgField, 0, 1);
        grid.getChildren().add(newMsgField);
        //Fextfeld zur Angabe eines neuen Servers
        final TextField serverField = new TextField();
        serverField.setPrefColumnCount(15);
        serverField.setPromptText("Enter the desired Server");

        //Zuordnung der Buttons
        GridPane.setConstraints(serverField, 0, 2);
        grid.getChildren().add(serverField);
        GridPane.setConstraints(receiveBtn, 1, 0);
        grid.getChildren().add(receiveBtn);
        GridPane.setConstraints(sendBtn, 1, 1);
        grid.getChildren().add(sendBtn);
        GridPane.setConstraints(connectBtn, 1, 2);
        grid.getChildren().add(connectBtn);
        GridPane.setConstraints(showAllBtn, 1, 3);
        grid.getChildren().add(showAllBtn);

        /*
         Definiert das Verhalten des Buttons zur Abholung der naechsten 
         Nachricht
         */
        receiveBtn.setOnAction(e -> {
            System.out.println(this);
            try {
                nextMsgField.setText(server.nextMessage(this.toString()));

            } catch (RemoteException ex) {
                Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
            }
        });

        /*
         Definiert das Verhalten des Buttons zum senden einer neuen Nachricht
         */
        sendBtn.setOnAction(e -> {
            currentMsg = newMsgField.getText();
            sendMsg();
        });

        /* 
         Definiert das Verhalten des Buttons zum (neu-)verbinden mit einem
         Server 
         */
        connectBtn.setOnAction(e -> {
            String servChoice = serverField.getText();
            if (servChoice.length() == 0) {
                reconnect();
            } else {
                reconnect(servChoice);
            }
            System.out.println("connecting to server");
        });

        /*
         Definiert das Verhalten des Buttons zur Ausgabe aller Nachrichten
         auf der Konsole
         */
        showAllBtn.setOnAction(e -> {
            try {
                //            stub.printAll();
                System.out.println(server.showAll());
            } catch (RemoteException ex) {
                System.out.println("request failed");
                Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
            }
        });

        //Zusammenbau der GUI
        StackPane root = new StackPane();
        root.getChildren().add(grid);
        Scene scene = new Scene(root, 550, 150);
        primaryStage.setTitle("Message Client");
        primaryStage.setScene(scene);
        primaryStage.show();
    }

    /**
     * Verbindet den Client mit ein neuen Server
     *
     * @param servStr Der Server der nun geneutzt werden soll
     */
    public static void reconnect(String servStr) {
        try {
            server = (MessageService) Naming.lookup(servStr);
        } catch (NotBoundException ex) {
            Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
        } catch (MalformedURLException ex) {
            Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
        } catch (RemoteException ex) {
            Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    /**
     * Verbindet lokal mit dem Default-Server
     */
    public static void reconnect() {
        reconnect("//127.0.0.1:1099/MessageService");
    }

    @Override
    public String toString() {
        return this.CLIENT_ID;
    }

    public static void main(String[] args) {
        reconnect();
        System.out.println("Client:connected");
        Application.launch(args);
    }

    /**
     * Nur zu Testzwecken
     */
    public static void initTestadata() {
        String result;
        try {
            server.newMessage("client1", "Nachricht1");
            server.newMessage("client2", "Nachricht2");
            server.newMessage("client3", "Nachricht3");
            server.newMessage("client1", "Nachricht4");
            result = server.nextMessage("client1");
            System.out.println(result);
            result = server.nextMessage("client1");
            System.out.println(result);
            result = server.nextMessage("client1");
            System.out.println(result);
            result = server.nextMessage("client1");
            System.out.println(result);
            result = server.nextMessage("client1");
            System.out.println(result);
            result = server.nextMessage("client2");
            System.out.println(result);
            result = server.nextMessage("client2");
            System.out.println(result);
        } catch (RemoteException e) {
            System.out.println("Client.main()");
        }
    }
}
