#include <stdio.h>
#include <string.h>
#include <vector>
#include <sstream>
#include <sys/socket.h>
#include <iostream>
#include <sys/types.h>
#include <arpa/inet.h>
#include "select.hpp"
#include "message.hpp"
#include "node.hpp"

#include <string.h>

using namespace std;

int main(int argc, char** argv)
{
    if(argc < 2)
        return 1;
    int port = atoi(argv[1]);
    node n(port);
    char cmd [1024];
    bool setid_done = false;
    int fd = 0;
    while(1)
    {
        // printfds();
        if (setid_done)
            fd = wait_for_input();
        // cout << "chosen fd is " << fd << endl;
        if(fd == 0)
        {
            fgets(cmd,1024,stdin);
            string s {cmd};
            string command = s.substr(0,s.find(","));
            if(command == "connect" && setid_done)
            {
                s.erase(0,s.find(",")+1);
                printf("inside connect\n");
                string temp = s;
                int p = temp.find(":");
                string ip = temp.substr(0,p);
                string port_t = temp.substr(p+1,temp.length());
                stringstream ss;
                ss << port_t;
                int port;
                ss >> port;
                //cout << "here\n";
                n.conn((char *)ip.c_str(),port);
            }
            else if(command == "setid")
            {
                s.erase(0,s.find(",")+1);
                setid_done = n.setid(atoi(s.c_str()));;
            }
            else if(command  == "send" && setid_done)
            {
                s.erase(0,s.find(",")+1);
                string id = s.substr(0, s.find(","));
                s.erase(0,s.find(",")+1);
                string payloadlen = s.substr(0, s.find(","));
                s.erase(0,s.find(",")+1);
                string payload = s;
                if(atoi(payloadlen.c_str()) != payload.length()-1)
                {
                    cout << "msg len does not match" << endl;
                    continue;
                }
                if(n.connected.find(atoi(id.c_str())) == n.connected.end())
                // if(n.discover())
                {
                    cout << "does not connected to such a id" << endl;
                    continue;
                }
                n.send_(atoi(id.c_str()),(char*)payload.c_str(),atoi(payloadlen.c_str()));
            }
            else if(command == "exit")
            {
                close_all();
                return 0;
            }
        }
        else if(fd == n.sock)
        {
            struct sockaddr_in addr;
            bzero((void*)&addr,sizeof(addr));
            socklen_t len;
            int newsock = accept(fd,(struct sockaddr*)&addr,&len);
            add_fd_to_monitoring(newsock);

        }
        else
        {
            char buffer[512];
            recv(fd,(void*)buffer,512,0);
            message msg(buffer);
            if(msg.func_id == 4)
            {
                n.connected[msg.src_id] = fd;
                cout << "got connection from " << msg.src_id << endl;
                n.ack(fd, msg.msg_id, msg.src_id);
            }
            if(msg.func_id == 1)
            {
                int* p = (int*)msg.payload;
                cout << "ack from " << msg.src_id << " msg confirmed id is: " << *p << endl; 
            }
            if(msg.func_id == 32)
            {
                if(msg.trailing_msg != 0)
                {
                    n.ack(fd, msg.msg_id, msg.src_id);
                    string all_msgs = msg.payload;
                    for (int i = 0; i < msg.trailing_msg; i++)
                    {
                        char buffer[512];
                        recv(fd,(void*)buffer,512,0);
                        message msg2(buffer);

                        all_msgs += msg2.payload;
                        n.ack(fd, msg2.msg_id, msg2.src_id);
                    }
                    cout << all_msgs << endl;
                }
                else
                {
                    printf("%s\n", msg.payload);
                    n.ack(fd, msg.msg_id, msg.src_id);
                }
            }
            if(msg.func_id == 8)
            {
                int* p = (int*)msg.payload;
                if(n.table.find(msg.msg_id) != n.table.end())
                {
                    n.nack(fd, msg.msg_id, msg.src_id);
                }
                else if(n.connected.find(*p) != n.connected.end())
                {
                    int route_payload[4];
                    route_payload[0] = msg.msg_id;
                    route_payload[1] = 2;
                    route_payload[2] = n.id;
                    route_payload[3] = *p;
                    message msg2(n.id, msg.src_id, 0, 16, (char*)route_payload, 16);
                    msg2.msg_id = msg.msg_id;
                    message::msg_counter--;
                    char buffer[512];
                    msg2.writetobuffer((void*)buffer);
                    send(fd, (void*)buffer, 512, 0);
                }
                else
                {
                    n.table[msg.msg_id] = msg.src_id;
                    for(auto neighbor : n.connected)
                    {
                        if(neighbor.first == msg.src_id)
                        {
                            continue;
                        }
                        message msg2(n.id, neighbor.first, 0, 16, msg.payload, 4);
                        msg2.msg_id = msg.msg_id;
                        message::msg_counter--;
                        char buffer[512];
                        msg2.writetobuffer((void*)buffer);
                        send(neighbor.second, (void*)buffer, 512, 0);
                    }
                }
            }
            if(msg.func_id == 16)
            {
                int * p =(int*) msg.payload;
                p++;
                int id_to_return_to = n.table[msg.msg_id];
                int * new_route_payload = new int[2+ (*p) +1];
                new_route_payload[0] = msg.msg_id;
                new_route_payload[1] = *p+1;
                new_route_payload[2] = n.id;
                memcpy((void*)(new_route_payload+3),(void*)(p+1),*p);
                message msg2 (n.id,id_to_return_to,0,16,(char*)new_route_payload,(2+ (*p) +1)*4);
                msg2.msg_id = msg.msg_id;
                message::msg_counter--;
                delete new_route_payload;
                char buffer[512];
                msg2.writetobuffer((void*)buffer);
                send(n.connected[id_to_return_to], (void*)buffer, 512, 0);
            }
        }
    }

    return 0;
}