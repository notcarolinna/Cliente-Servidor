#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <map>
#include <thread>
#include <mutex>

#define PORT 8081
#define BUFFER_SIZE 1024
#define MESSAGE_SERVER "Connected to the server || /FILE to send a file"
#define FILE_NOTIFICATION "FILE-TRANSFER"
#define FILE_COMPLETE "FILE-COMPLETE"
#define USERNAME_INFORMATION "Please inform your username: "

void errorFunction(const std::string &message)
{
    std::cerr << message << std::endl;
    exit(EXIT_FAILURE);
}

std::string getAddressString(const struct sockaddr_in &address)
{
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(address.sin_addr), ip, INET_ADDRSTRLEN);
    return std::string(ip) + ":" + std::to_string(ntohs(address.sin_port));
}

void handleClient(int clientSockfd, struct sockaddr_in clientAddr, std::map<int, std::string> &clients, std::mutex &clientsMutex)
{
    char buffer[BUFFER_SIZE];
    bool fileTransfer = false;
    std::string newClientMessage;
    int sendLen;
    int recvLen;

    std::string addressClient = getAddressString(clientAddr);

    sendLen = send(clientSockfd, USERNAME_INFORMATION, strlen(USERNAME_INFORMATION), 0);
    if (sendLen < 0)
    {
        std::cerr << "Error sending username message" << std::endl;
    }

    recvLen = recv(clientSockfd, buffer, BUFFER_SIZE, 0);
    buffer[recvLen] = '\0';

    {
    std::lock_guard<std::mutex> lock(clientsMutex);
    clients[clientSockfd] = buffer;
    }

    // Send welcome message
    sendLen = send(clientSockfd, MESSAGE_SERVER, strlen(MESSAGE_SERVER), 0);
    if (sendLen < 0)
    {
        std::cerr << "Error sending welcome message" << std::endl;
    }
    std::cout << "New client " << clients[clientSockfd] << " added: " << addressClient << std::endl;

    // Send welcome message to the other clients
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        newClientMessage = clients[clientSockfd] + " has joined the server.";
        for (const auto &client : clients)
        {
            if (client.first != clientSockfd)
            {
                sendLen = send(client.first, newClientMessage.c_str(), newClientMessage.length(), 0);
                if (sendLen < 0)
                {
                    std::cerr << "Error sending message to client: " << client.second << std::endl;
                }
            }
        }
    }

    while (true)
    {
        recvLen = recv(clientSockfd, buffer, BUFFER_SIZE, 0);
        if (recvLen <= 0)
        {
            if (recvLen == 0)
            {
                std::cout << "Client disconnected: " << addressClient << std::endl;
            }
            else
            {
                std::cerr << "Error receiving data from client: " << addressClient << std::endl;
            }
            close(clientSockfd);
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                newClientMessage = clients[clientSockfd] + " has left the server.";
                for (const auto &client : clients)
                {
                    if (client.first != clientSockfd)
                    {
                        sendLen = send(client.first, newClientMessage.c_str(), newClientMessage.length(), 0);
                        if (sendLen < 0)
                        {
                            std::cerr << "Error sending message to client: " << client.second << std::endl;
                        }
                    }
                }
                clients.erase(clientSockfd);
            }
            break;
        }

        buffer[recvLen] = '\0'; // Null-terminate the received data

        std::cout << "Received from client " << clients[clientSockfd] << ": " << buffer << std::endl;

        if (strcmp(buffer, FILE_NOTIFICATION) == 0)
        {
            newClientMessage = FILE_NOTIFICATION;
            fileTransfer = true;
        }
        else if (strcmp(buffer, FILE_COMPLETE) == 0)
        {
            newClientMessage = FILE_COMPLETE;
            fileTransfer = false;
        }
        else
        {
            if (fileTransfer)
            {
                newClientMessage = buffer;
            }
            else
            {
                newClientMessage = clients[clientSockfd] + ": " + buffer;
            }
        }

        // Broadcast message to other clients
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (const auto &client : clients)
        {
            if (client.first != clientSockfd)
            {
                sendLen = send(client.first, newClientMessage.c_str(), newClientMessage.length(), 0);
                if (sendLen < 0)
                {
                    std::cerr << "Error sending message to client: " << client.second << std::endl;
                }
            }
        }
    }
}

int main()
{
    int sockfd;
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    std::map<int, std::string> Clients;
    std::mutex clientsMutex;

    // socket() -- create socket

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        errorFunction("Error to create the socket");
    }

    // set serverAddr -- setting up the servers address structures

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // bind() -- binding the socket to the server address

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        close(sockfd);
        errorFunction("Error to bind the socket");
    }

    if(listen(sockfd, 5) < 0){
        close(sockfd);
        errorFunction("Error to listen the socket");
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    // recvfrom() and sendto() -- functions to receive and send messages

    while (true)
     {
        int clientSockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &addrLen);
        if (clientSockfd < 0)
        {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }

        std::thread clientThread(handleClient, clientSockfd, clientAddr, std::ref(Clients), std::ref(clientsMutex));
        clientThread.detach();
    }


    close(sockfd);
    return 0;
}