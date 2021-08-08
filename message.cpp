#include <stdio.h>
#include <string.h>

#include "message.hpp"

#include "iostream"

using namespace std;

int message::msg_counter=1;

//copy ctor
message::message(message& msg)
{
    this->msg_id = msg.msg_id;
    this->src_id = msg.src_id;
    this->dest_id = msg.dest_id;
    this->trailing_msg = msg.trailing_msg;
    this->func_id = msg.func_id;
    memset(this->payload,0,492);
    memcpy((void*)this->payload, (void*)msg.payload, 492);
}

//ctor from buffer to msg
message::message(void* buffer)
{
    int* p = (int*)buffer;
    this->msg_id = p[0];
    this->src_id = p[1];
    this->dest_id = p[2];
    this->trailing_msg = p[3];
    this->func_id = p[4];
    memset(this->payload,0,492);
    memcpy((void*)this->payload, ((char*)buffer) + 20 , 492);

}

//ctor
message::message(int src_id, int dest_id, int trailing_msg,
                 int func_id, char* payload, size_t payload_len)
{
    this->msg_id = msg_counter;
    msg_counter++;
    this->src_id = src_id;
    this->dest_id = dest_id;
    this->trailing_msg = trailing_msg;
    this->func_id = func_id;
    memset(this->payload,0,492);
    if(payload_len <= 492)
    {
        if (func_id == 32)
        {
            int * p = (int*)this->payload;
            *p = payload_len;
            memcpy((void*)(p+1), payload, payload_len);
        }
        else
            memcpy(this->payload, payload, payload_len);
    }
}

void message::Ack(int msg_id, int dest_id)
{
    printf("ack");
}

void message::Nack(int msg_id, int dest_id)
{
    printf("nack");
}

void message::Connect()
{

}

void message::Discover(int id)
{

}

void message::Route(int id, int msg_len, std::list<int> nodes)
{

}

void message::send(int msg_len, char* msg)
{

}

void message::relay(int next_node, int num_of_incoming_msg)
{

}

void message::writetobuffer(void* buffer)
{
    //memset(buffer,0,512);
    char * c = (char*)buffer;
    for(int i = 0;i<512;i++)
        c[i] = 0;

    int* p = (int*)buffer;

    p[0] = this->msg_id;
    p[1] = this->src_id;
    p[2] = this->dest_id;
    p[3] = this->trailing_msg;
    p[4] = this->func_id;
    memcpy(((char*)buffer) + 20 , this->payload, 492);
}
