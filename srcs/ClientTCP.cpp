#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <thread>
#include <fcntl.h>
#include <fstream>

#define PORT 8081
#define BUFFER_SIZE 1024
#define MESSAGE_SERVER "Hello from client."
#define FILE_NOTIFICATION "FILE-TRANSFER"
#define FILE_COMPLETE "FILE-COMPLETE"
#define USERNAME_INFORMATION "Please inform your username: "

void errorFunction(const std::string &message)
{
    std::cerr << message << std::endl;
    exit(EXIT_FAILURE);
}

void receiveMessage(int sockfd)
{
    char buffer[BUFFER_SIZE];
    bool fileTransfer = false;
    std::ofstream file;
    while (true)
    {
        int recvLen = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if (recvLen < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            else
            {
                std::cerr << "Error to receive the message" << std::endl;
            }
        }
        else
        {
            buffer[recvLen] = '\0';

            if (strcmp(buffer, FILE_NOTIFICATION) == 0)
            {
                std::cout << "You will receive a file" << std::endl;    
                fileTransfer = true;
                std::string filename = "received_file.txt";
                file.open(filename, std::ios::binary);
                if(!file.is_open()){
                    std::cerr << "Error opening the file" << std::endl;
                }
                continue;
            }
            else if (strcmp(buffer, FILE_COMPLETE) == 0)
            {
                fileTransfer = false;
                std::cout << "You received the file with sucess" << std::endl;
                if(file.is_open()){
                    file.close();
                }
                continue;
            }

            if (fileTransfer)
            {
                std::cout << "Receiving file" << std::endl;
                if(!file.is_open()){
                    std::cerr << "Error opening the file" << std::endl;
                }
                file.write(buffer, recvLen);
            }
            else
            {
            std::cout << buffer << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main()
{
    int sockfd;
    struct sockaddr_in serverAddr;

    int sendLen;
    std::string messageUser;

    // socket()

    sockfd = (socket(AF_INET, SOCK_STREAM, 0));
    if (sockfd < 0)
    {
        errorFunction("Error to create the socket");
    }

    // set serverAddr

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(PORT);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        close(sockfd);
        errorFunction("Error to connect the socket");
    }

    //Receive thread
    std::thread receiveThread(receiveMessage, sockfd);
    receiveThread.detach();

    while (true)
    {

        std::getline(std::cin, messageUser);
 
        
        if (messageUser.rfind("/FILE", 0) == 0)
        {

            std::string filename = messageUser.substr(6);
            std::ifstream file(filename, std::ios::binary);
            if (!file.is_open())
            {
                std::cerr << "Error opening the file" << std::endl;
                continue;
            }

            sendLen = send(sockfd, FILE_NOTIFICATION, strlen(FILE_NOTIFICATION), 0); // send the notification to the server
            if (sendLen < 0)
            {
                std::cerr << "Error to notification the file is send" << std::endl;
            }

            char fileBuffer[BUFFER_SIZE];
            while (!file.eof())
            {
                file.read(fileBuffer, BUFFER_SIZE);
                std::streamsize bytes = file.gcount();
                sendLen = send(sockfd, fileBuffer, bytes, 0);
                if (sendLen < 0)
                {
                    std::cerr << "Error sending the file" << std::endl;
                    break;
                }
            }

            file.close();
            std::cout << "The file " << filename << " had been send with sucess" << std::endl;

            sleep(1);

            sendLen = send(sockfd, FILE_COMPLETE, strlen(FILE_COMPLETE), 0); // send the notification to the server
            if (sendLen < 0)
            {
                std::cerr << "Error to notification the file is complete" << std::endl;
            }

            continue;
        }

        sendLen = send(sockfd, messageUser.c_str(), messageUser.length(), 0);
        if (sendLen < 0)
        {
            std::cerr << "Error sending message" << std::endl;
        }
    }
    

    close(sockfd);
}