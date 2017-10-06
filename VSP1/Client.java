/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

import java.rmi.RemoteException;
import java.rmi.Naming;
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

    Stub stub = new Stub();
    Button receiveBtn = new Button("receive next message");
    Button sendBtn = new Button("send Message");
    Button connectBtn = new Button("connect");
    Button showAllBtn = new Button("show all");

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
            System.out.println("3 = 2 + 1 = " + server.add(2, 1));
        } catch (RemoteException e) {
            System.out.println("Client.main()");
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
            System.out.println("receiving message");
            try {
                nextMsgField.setText(server.nextMessage(this.toString()));
            } catch (RemoteException ex) {
                Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
            }
        });

        sendBtn.setOnAction(e -> {
            System.out.println(newMsgField.getText());
            try {
                server.newMessage(this.toString(), newMsgField.getText());
            } catch (RemoteException ex) {
                Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
            }
        });

        connectBtn.setOnAction(e -> {
                System.out.println("connecting to server");
        });

        showAllBtn.setOnAction(e -> {
            stub.printAll();
        });

        StackPane root = new StackPane();
        root.getChildren().add(grid);

        Scene scene = new Scene(root, 550, 150);

        primaryStage.setTitle("Message Client");
        primaryStage.setScene(scene);
        primaryStage.show();
    }

    public static void main(String[] args) {
        try {
            registry = LocateRegistry.getRegistry();
            server = (MessageService) Naming.lookup("MessageService");
        } catch (Exception e) {
            e.printStackTrace();
        }
        Application.launch(args);
    }
}
