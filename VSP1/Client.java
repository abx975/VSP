/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

import java.net.InetAddress;
import java.net.MalformedURLException;
import java.net.UnknownHostException;
import java.rmi.RemoteException;
import java.rmi.Naming;
import java.rmi.NotBoundException;
import java.rmi.registry.LocateRegistry;
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
 *
 * @author lennart
 */
public class Client extends Application {

    static MessageService server;
    static Registry registry;
    public static int MAX_FAILIURE_TIME = 10000;
    private int failiureTime = 0;
    private String CLIENT_ID;

    String currentMsg;
    Stub stub = new Stub();
    Button receiveBtn = new Button("receive next message");
    Button sendBtn = new Button("send Message");
    Button connectBtn = new Button("connect");
    Button showAllBtn = new Button("show all");

    public Client() throws UnknownHostException{
        CLIENT_ID=InetAddress.getLocalHost().toString();
        CLIENT_ID=CLIENT_ID.split("/")[1];
        System.out.println("Client created: "+this);
    }
    
    /**
     * For testing purposes only
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

    private void sendMsg() {
        System.out.println(currentMsg);

        try {
            server.newMessage(this.toString(), currentMsg);
            failiureTime = 0;

        } catch (RemoteException ex) {
            Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
            if (failiureTime < MAX_FAILIURE_TIME) {
                sendMsg();
            }
            try {
                this.wait(500);
            } catch (InterruptedException ex1) {
                Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex1);
                failiureTime += 500;
            }
        }
    }

 

    @Override
    public void start(Stage primaryStage) {
//        initTestadata();

        //Creating a GridPane container
        GridPane grid = new GridPane();
        grid.setPadding(new Insets(10, 10, 10, 10));
        grid.setVgap(5);
        grid.setHgap(5);
        //Defining the Name text field
        final TextField nextMsgField = new TextField();

        nextMsgField.setMinWidth(400);

        nextMsgField.setPromptText("");
        nextMsgField.setPrefColumnCount(10);
        nextMsgField.getText();
        GridPane.setConstraints(nextMsgField, 0, 0);
        grid.getChildren().add(nextMsgField);
        //Defining the Last Name text field
        final TextField newMsgField = new TextField();
        newMsgField.setPromptText("Enter your message here");

        GridPane.setConstraints(newMsgField, 0, 1);
        grid.getChildren().add(newMsgField);
        //Defining the Comment text field
        final TextField serverField = new TextField();
        serverField.setPrefColumnCount(15);
        serverField.setPromptText("Enter your serverField.");
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

        receiveBtn.setOnAction(e -> {
            System.out.println(this);
            try {
                nextMsgField.setText(server.nextMessage(this.toString()));
          
            } catch (RemoteException ex) {
                Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
            }
        });

        sendBtn.setOnAction(e -> {
            currentMsg = newMsgField.getText();
            sendMsg();
        });

        connectBtn.setOnAction(e -> {
            String servChoice = serverField.getText();
            if (servChoice.length() == 0) {
                reconnect();
            } else {
                reconnect(servChoice);
            }
            System.out.println("connecting to server");
        });

        showAllBtn.setOnAction(e -> {
            try {
                //            stub.printAll();
                System.out.println(server.showAll());
            } catch (RemoteException ex) {
                System.out.println("request failed");
                Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
            }
        });

        StackPane root = new StackPane();
        root.getChildren().add(grid);

        Scene scene = new Scene(root, 550, 150);

        primaryStage.setTitle("Message Client");
        primaryStage.setScene(scene);
        primaryStage.show();
    }

    public static void reconnect() {
        try {
            server = (MessageService) Naming.lookup("//127.0.0.1:1099/Server");
        } catch (NotBoundException ex) {
            Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
        } catch (MalformedURLException ex) {
            Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
        } catch (RemoteException ex) {
            Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

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

    @Override
    public String toString() {
        return this.CLIENT_ID;
    }
    
    

    public static void main(String[] args) {
        reconnect();
        System.out.println("Client:connected");
        Application.launch(args);
    }
}
