#include <stdio.h>
#include <iostream>
#include <string.h>

#include "node.hpp"
#include "message.hpp"
#include "select.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unordered_map>

using namespace std;
#define PAYLOAD_SIZE 488

node::node(int port)
{
    this->port = port;
}

bool node::setid(int id = -1)
{
    this->id = id;
    
    //open socket
    this->sock = socket(AF_INET, SOCK_STREAM, 0);
    //bind
    struct sockaddr_in addr;
    bzero((void*)&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    //change to big indian
    addr.sin_port = htons(this->port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(this->sock, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        printf("nack\n");
        return false;
    }

    if(listen(this->sock, 100) == -1)
    {
        printf("nack\n");
        return false;
    }
    add_fd_to_monitoring(this->sock);
    printf("ack\n");
    return true;
}

void node::conn(char* ip, int port)
{
    printf("%s %d \n",ip,port);
    int sock_2 = socket(AF_INET, SOCK_STREAM, 0);
    //bind
    struct sockaddr_in addr;
    bzero((void*)&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    //change to big indian
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, (void*)&addr.sin_addr.s_addr);
    add_fd_to_monitoring(sock_2);
    cout << "connecting..." << endl;
    if(connect(sock_2, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        printf("nack\n");
        printf("failed to connect\n");
        return;
    }
    cout << "finished connecting" << endl;
    message msg(this->id, 0, 0, 4, NULL, 0);
    char buffer[512] = {0};
    msg.writetobuffer((void*)buffer);
    send(sock_2, (void*)buffer, 512, 0);
    int len = recv(sock_2, (void*)buffer, 512, 0);
    if(len != 512)
    {
        printf("nack\n");
        printf("failed to recive\n");
        return;
    }
    message msg_recived((void*)buffer);
    int dest_id = msg_recived.src_id;
    connected[dest_id] = sock_2;
}



//main func

void node::send_(int dest_id, char* msg,size_t len)
{
    if(connected.find(dest_id) == connected.end())
        return;
    char * p = msg;
    int packets;
    char buffer [512] = {0};
    if (len%PAYLOAD_SIZE == 0)
    {
        packets = len/PAYLOAD_SIZE;
    }
    else
    {
        packets = len/PAYLOAD_SIZE + 1;
    }
    for(int i = 0; i < packets;i++)
    {
        message m (this->id,dest_id,packets-i-1,32,p,(i == packets-1)?len%PAYLOAD_SIZE :PAYLOAD_SIZE);
        p+=PAYLOAD_SIZE;
        m.writetobuffer(buffer);
        send(connected[dest_id],buffer,512,0);
    }
}
void node::send_by_id(int to,int dest_id, char * msg,int len)
{
    char * p = msg;
    int packets;
    char buffer [512] = {0};
    if (len%PAYLOAD_SIZE == 0)
    {
        packets = len/PAYLOAD_SIZE;
    }
    else
    {
        packets = len/PAYLOAD_SIZE + 1;
    }
    for(int i = 0; i < packets;i++)
    {
        message m (this->id,dest_id,packets-i-1,32,p,(i == packets-1)?len%PAYLOAD_SIZE :PAYLOAD_SIZE);
        p+=PAYLOAD_SIZE;
        m.writetobuffer(buffer);
        send(connected[to],buffer,512,0);
    }
}

// void node::route(int id){}

void node::peers()
{
    for(auto p: connected)
    {
        cout <<  p.first << " ";
    }
    cout << endl;
}

void node::ack(int fd,int msg_id,int dest_id)
{
    message msg(this->id, dest_id, 0, 1, (char*)&msg_id, 4);
    char buffer [512] = {0};
    msg.writetobuffer((void*)buffer);
    send(fd, (void*)buffer, 512, 0);   
}
void node::nack(int fd,int msg_id,int dest_id)
{
    message msg(this->id, dest_id, 0, 2, (char*)&msg_id, 4);
    char buffer [512] = {0};
    msg.writetobuffer((void*)buffer);
    send(fd, (void*)buffer, 512, 0);   
}