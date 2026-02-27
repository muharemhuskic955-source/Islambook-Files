#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 9000
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef enum {
    MSG_TEXT = 1,
    MSG_TYPING,
    MSG_READ_RECEIPT,
    MSG_NOTIFICATION
} MessageType;

typedef struct {
    int socket;
    char username[32];
} Client;

Client clients[MAX_CLIENTS];

// Initialize clients array
void initClients() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = 0;
        clients[i].username[0] = '\0';
    }
}

// Add client to clients array
int addClient(int clientSocket, const char* username) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket == 0) {
            clients[i].socket = clientSocket;
            strncpy(clients[i].username, username, sizeof(clients[i].username) - 1);
            clients[i].username[sizeof(clients[i].username) - 1] = '\0';
            return i;
        }
    }
    return -1;
}

// Remove client
void removeClient(int index) {
    close(clients[index].socket);
    clients[index].socket = 0;
    clients[index].username[0] = '\0';
}

// Broadcast message to all clients except sender
void broadcastMessage(int senderIndex, MessageType type, const char* message) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%d|%s: %s", type, clients[senderIndex].username, message);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (i != senderIndex && clients[i].socket != 0) {
            send(clients[i].socket, buffer, strlen(buffer), 0);
        }
    }
}

// Send message to a specific client by username
void sendMessageToUser(const char* username, MessageType type, const char* sender, const char* message) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%d|%s: %s", type, sender, message);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != 0 && strcmp(clients[i].username, username) == 0) {
            send(clients[i].socket, buffer, strlen(buffer), 0);
            break;
        }
    }
}

int main() {
    int serverSocket, newSocket, maxSd, activity, valread, sd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    fd_set readfds;

    initClients();

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 3) < 0) {
        perror("Listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);
        maxSd = serverSocket;

        // Add client sockets to set
        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = clients[i].socket;
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > maxSd) maxSd = sd;
        }

        activity = select(maxSd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0)) {
            perror("select error");
        }

        // Incoming connection
        if (FD_ISSET(serverSocket, &readfds)) {
            if ((newSocket = accept(serverSocket, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // Receive username from client
            valread = read(newSocket, buffer, sizeof(buffer));
            if (valread <= 0) {
                close(newSocket);
                continue;
            }
            buffer[valread] = '\0';

            int clientIndex = addClient(newSocket, buffer);
            if (clientIndex == -1) {
                char* msg = "Server full\n";
                send(newSocket, msg, strlen(msg), 0);
                close(newSocket);
            } else {
                printf("New connection: %s\n", buffer);
                char welcomeMsg[BUFFER_SIZE];
                snprintf(welcomeMsg, sizeof(welcomeMsg), "Welcome %s!\n", buffer);
                send(newSocket, welcomeMsg, strlen(welcomeMsg), 0);
            }
        }

        // IO operation on clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = clients[i].socket;
            if (sd == 0) continue;

            if (FD_ISSET(sd, &readfds)) {
                valread = read(sd, buffer, sizeof(buffer));
                if (valread == 0) {
                    // Client disconnected
                    printf("%s disconnected\n", clients[i].username);
                    removeClient(i);
                } else {
                    buffer[valread] = '\0';
                    // Simple protocol: message format "type|target|message"
                    // type: 1=text, 2=typing, 3=read receipt, 4=notification
                    // target: username or "all" for broadcast
                    char* typeStr = strtok(buffer, "|");
                    char* target = strtok(NULL, "|");
                    char* msg = strtok(NULL, "|");

                    if (!typeStr || !target || !msg) {
                        send(sd, "Invalid message format\n", 22, 0);
                        continue;
                    }

                    int msgType = atoi(typeStr);

                    if (strcmp(target, "all") == 0) {
                        broadcastMessage(i, msgType, msg);
                    } else {
                        sendMessageToUser(target, msgType, clients[i].username, msg);
                    }
                }
            }
        }
    }

    return 0;
}
