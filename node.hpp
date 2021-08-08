#pragma once

#include "message.hpp"
#include <stdio.h>
#include <string.h>
#include <unordered_map>
#include <utility>

// using namespace std;

class node
{
    struct pair_
    {
        int a, b;
        // pair_():a(-1),b(-1){
        pair_() = default;
        // }
    };

    public:
    int id = -1;
    int sock;
    int port;
    char* ip;

    //private:
    //id and the proper socket
    std::unordered_map<int, int> connected;

    //key is msg id and value is node id
    std::unordered_map<int, int> id_src;  

    //key is msg id and value is counter
    std::unordered_map<int, int>id_counter;

    //key is id and value is queue of msgs
    std::unordered_map<int , std::string> dest_to_string;
    
    std::unordered_map<int, int> dest_to_next_node;



    node(int port);

    bool setid(int id);
    void conn(char* ip, int port);

    //main func
    void send_(int dest_id, char* msg,size_t len);
    void send_by_id(int to,int dst_id, char * msg,int len);

    // void route(int id);
    void peers();
    void ack(int fd,int msg_id,int dest_id);
    void nack(int fd,int msg_id,int dest_id);

};