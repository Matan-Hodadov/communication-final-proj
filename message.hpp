#pragma once

#include <stdio.h>
// #include <string.h>
// #include <unordered_map>
#include <list>

// using namespace std;

class message
{
    public:
    static int msg_counter;
    int msg_id;
    int src_id;
    int dest_id;
    int trailing_msg;
    int func_id;
    char payload[492];

    message(message& msg);
    message(void* buffer);
    message(int src_id, int dest_id, int trailing_msg, int func_id, char payload[492], size_t payload_len);

    void Ack(int msg_id, int dest_id);
    void Nack(int msg_id, int dest_id);
    void Connect();  //maybe need to change it to return message*
    void Discover(int id);  //maybe need to change it to return message*
    void Route(int id, int msg_len, std::list<int> nodes);
    void send(int msg_len, char* msg);
    void relay(int next_node, int num_of_incoming_msg);

    void writetobuffer(void* buffer);
};