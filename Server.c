// Server Application

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdbool.h>

/////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     SendFileToClient
//  Input :             ClientSocket, FileName
//  Output :            Sends file data to client
//  Description :       Opens requested file and transfers it to client
//  Author :            Rushikesh Baban Dhande
//  Date :              06/01/2026
//
/////////////////////////////////////////////////////////////////////////////
void SendFileToClient(int ClientSocket, char * Filename)
{
    int fd = 0;
    struct stat sobj;
    char Buffer[1024];
    int BytesRead = 0;
    char Header[64] = {'\0'};

    printf("File name is : %s : %lu\n",Filename,strlen(Filename));

    fd = open(Filename, O_RDONLY);

    // Unable to open file
    if(fd < 0)
    {
        printf("Unable to open file\n");

        // Send error message to client
        write(ClientSocket, "ERR\n",4);
        return;
    }

    // Get file details
    stat(Filename,&sobj);

    // Prepare header (OK + file size)
    snprintf(Header,sizeof(Header),"OK %ld\n",(long)sobj.st_size);

    // Send header to client
    write(ClientSocket,Header,strlen(Header));
    
    // Send file contents in chunks
    while((BytesRead = read(fd, Buffer, sizeof(Buffer))) > 0)
    {
        write(ClientSocket,Buffer,BytesRead);
    }

    close(fd);
}

/////////////////////////////////////////////////
//
//  Commandline Argument Application
//  1st Argument : Port Number 
//  Example : ./server 9000
//
/////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     main
//  Input :             Command line arguments
//  Output :            Entry point of server
//  Description :       Creates server, accepts clients and handles requests
//  Author :            Rushikesh Baban Dhande
//  Date :              06/01/2026
//
/////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    int ServerSocket = 0;
    int ClientSocket = 0;
    int Port = 0;
    int iRet = 0;

    char FileName[50] = {'\0'};
    pid_t pid = 0;

    struct sockaddr_in ServerAddr;
    struct sockaddr_in ClientAddr;
     
    socklen_t AddrLen = sizeof(ClientAddr);

    // Validate arguments
    if((argc < 2) || (argc > 2))
    {
        printf("Unable to process: invalid number of arguments\n");
        printf("Please provide the port number\n");
        return -1;
    }

    // Get port number
    Port = atoi(argv[1]);

    //////////////////////////////////////////////////
    //  Step 1 : Create TCP socket
    //////////////////////////////////////////////////
    ServerSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(ServerSocket < 0)
    {
        printf("Unable to create server socket\n");
        return -1;
    }

    //////////////////////////////////////////////////
    //  Step 2 : Bind socket to IP and Port
    //////////////////////////////////////////////////
    memset(&ServerAddr, 0,sizeof(ServerAddr));

    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_port = htons(Port);
    ServerAddr.sin_addr.s_addr = INADDR_ANY;

    iRet = bind(ServerSocket, (struct sockaddr *)&ServerAddr, sizeof(ServerAddr));

    if(iRet == -1)
    {
        printf("Unable to bind\n");
        close(ServerSocket);
        return -1;
    }

    //////////////////////////////////////////////////
    //  Step 3 : Listen for client connections
    //////////////////////////////////////////////////
    iRet = listen(ServerSocket,11);

    if(iRet == -1)
    {
        printf("Server unable to listen the request\n");
        close(ServerSocket);
        return -1;
    }

    printf("Server is running on port : %d\n",Port);
    
    //////////////////////////////////////////////////
    //  Loop to accept multiple client requests
    //////////////////////////////////////////////////
    while(1)
    {
        //////////////////////////////////////////////////
        //  Step 4 : Accept client request
        ////////////////////////////////////////////////// 
        memset(&ClientAddr, 0,sizeof(ClientAddr));

        printf("Server is waiting for client request\n");
        
        ClientSocket = accept(ServerSocket, (struct sockaddr *)&ClientAddr, &AddrLen);

        if(ClientSocket < 0)
        {
            printf("Unable to accept client request\n");
            continue;
        }

        printf("Client gets connected : %s\n",inet_ntoa(ClientAddr.sin_addr));

        //////////////////////////////////////////////////
        //  Step 5 : Create new process for client
        //////////////////////////////////////////////////
        pid = fork();

        if(pid < 0)
        {
            printf("Unable to create new process\n");
            close(ClientSocket);
            continue;
        }

        // Child process
        if(pid == 0)
        {
            printf("New process created for client request\n");

            close(ServerSocket);
            
            // Read file name from client
            iRet = read(ClientSocket, FileName,sizeof(FileName));

            printf("Requested file by client : %s\n",FileName);            
            
            FileName[strcspn(FileName, "\r\n")] = '\0';

            // Send file
            SendFileToClient(ClientSocket, FileName);

            close(ClientSocket);

            printf("File transfer done & client disconnected\n");           
        
            exit(0);
        }
        else    // Parent process
        {
            close(ClientSocket);
        }
    }

    close(ServerSocket);
    return 0;
}