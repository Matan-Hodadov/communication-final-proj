#pragma once

#include <stdio.h>
#include <string.h>
#include <unordered_map>

// using namespace std;

class node
{
    public:
    int id = -1;
    int sock;
    int port;
    char* ip;

    //private:
    //id and the proper socket
    std::unordered_map<int, int> connected;

    node(int port);

    void setid(int id);
    void conn(char* ip, int port);

    //main func
    void send_(int dest_id, char* msg,size_t len);
    // void route(int id);
    void peers();
    void ack(int fd,int msg_id,int dest_id);

};