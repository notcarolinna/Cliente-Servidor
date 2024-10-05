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

#define PORT 8080
#define BUFFER_SIZE 1024
#define MESSAGE_SERVER "Hello from client."
#define FILE_NOTIFICATION "FILE-TRANSFER"
#define FILE_COMPLETE "FILE-COMPLETE"

void errorFunction(const std::string &message)
{
    std::cerr << message << std::endl;
    exit(EXIT_FAILURE);
}

void receiveMessage(int sockfd, struct sockaddr_in &serverAddr, socklen_t &addrLen)
{
    char buffer[BUFFER_SIZE];
    bool fileTransfer = false;
    std::ofstream file;
    while (true)
    {
        int recvLen = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&serverAddr, &addrLen);
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
    char buffer[BUFFER_SIZE];
    struct sockaddr_in serverAddr;
    socklen_t addrLen = sizeof(serverAddr);
    std::string userIdentification = " ";
    std::string messageUser;
    int sendLen;

    // socket()

    sockfd = (socket(AF_INET, SOCK_DGRAM, 0));
    if (sockfd < 0)
    {
        errorFunction("Error to create the socket");
    }

    fcntl(sockfd, F_SETFL, O_NONBLOCK); // setting the socket to non-blocking mode

    // set serverAddr

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(PORT);

    // recvfrom() and sendto()
    std::thread receiveThread(receiveMessage, sockfd, std::ref(serverAddr), std::ref(addrLen));
    receiveThread.detach();

    while (true)
    {
        if (userIdentification == " ")
        {
            std::cout << "Enter your username: ";
            std::getline(std::cin, messageUser);
            userIdentification = messageUser;

            sendLen = sendto(sockfd, messageUser.c_str(), messageUser.length(), 0, (struct sockaddr *)&serverAddr, addrLen);
            if (sendLen < 0)
            {
                std::cerr << "Error sending the username" << std::endl;
            }

            while (true)
            {
                int recvLen = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&serverAddr, &addrLen);
                if (recvLen < 0)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        continue;
                    }
                    else
                    {
                        std::cerr << "Error to receive the connection message" << std::endl;
                        break;
                    }
                }
                else
                {
                    buffer[recvLen] = '\0';
                    std::cout << buffer << std::endl;
                    break;
                }
            }
            continue;
        }
        else
        {
            // std::cout << userIdentification << ": ";
            std::getline(std::cin, messageUser);
        }

        if (messageUser.rfind("/FILE", 0) == 0)
        {

            std::string filename = messageUser.substr(6);
            std::ifstream file(filename, std::ios::binary);
            if (!file.is_open())
            {
                std::cerr << "Error opening the file" << std::endl;
                continue;
            }

            sendLen = sendto(sockfd, FILE_NOTIFICATION, strlen(FILE_NOTIFICATION), 0, (struct sockaddr *)&serverAddr, addrLen); // send the notification to the server
            if (sendLen < 0)
            {
                std::cerr << "Error to notification the file is send" << std::endl;
            }

            char fileBuffer[BUFFER_SIZE];
            while (!file.eof())
            {
                file.read(fileBuffer, BUFFER_SIZE);
                std::streamsize bytes = file.gcount();
                sendLen = sendto(sockfd, fileBuffer, bytes, 0, (struct sockaddr *)&serverAddr, addrLen);
                if (sendLen < 0)
                {
                    std::cerr << "Error sending the file" << std::endl;
                    break;
                }
            }

            file.close();
            std::cout << "The file " << filename << " had been send with sucess" << std::endl;

            sendLen = sendto(sockfd, FILE_COMPLETE, strlen(FILE_COMPLETE), 0, (struct sockaddr *)&serverAddr, addrLen); // send the notification to the server
            if (sendLen < 0)
            {
                std::cerr << "Error to notification the file is complete" << std::endl;
            }

            continue;
        }

        int sendLen = sendto(sockfd, messageUser.c_str(), messageUser.length(), 0, (struct sockaddr *)&serverAddr, addrLen);
        if (sendLen < 0)
        {
            std::cerr << "Error sending message" << std::endl;
        }
    }
    

    close(sockfd);
}