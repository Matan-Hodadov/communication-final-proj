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
        if (setid_done)
            fd = wait_for_input();
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

                n.dest_to_string[atoi(id.c_str())] = payload;
                if(atoi(payloadlen.c_str()) != payload.length()-1)
                {
                    cout << "msg len does not match" << endl;
                    continue;
                }
                if(n.connected.find(atoi(id.c_str())) == n.connected.end())
                {                             
                    char buff [512];
                    int msgid = message::msg_counter;
                    for (auto nei: n.connected)
                    {
                        int to = atoi(id.c_str());
                        message msg(n.id,nei.first,1,8,(char*)&to,4);
                        msg.msg_id = msgid;
                        message::msg_counter--;
                        msg.writetobuffer((void*)buff);
                        send(nei.second, buff, 512, 0);
                    }
                }
                // n.send_(atoi(id.c_str()),(char*)payload.c_str(),atoi(payloadlen.c_str()));
            }
            else if(command  == "exit,")
            {
                close_all();
                return 0;
            }
            else
            {
                cout << "input is not valid. pls try again" << endl;
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
            if(msg.func_id == 1) //ack
            {
                int* p = (int*)msg.payload;
                cout << "ack from " << msg.src_id << " msg confirmed id is: " << *p << endl; 
            }
            if(msg.func_id == 2) //nack
            {
                int* p = (int*)msg.payload;
                cout << "nack from " << msg.src_id << " msg failed id is: " << *p << endl; 
            }
            if(msg.func_id == 32) //send
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
                    if(n.id == msg.dest_id)
                    {
                        cout << all_msgs << endl;
                    }
                    else
                    {
                        cout << "forward to" << n.dest_to_next_node[msg.dest_id] << " " << msg.dest_id << endl;
                        n.send_by_id(n.dest_to_next_node[msg.dest_id],msg.dest_id,(char*)all_msgs.c_str(),all_msgs.length());
                    }
                }
                else
                {

                    if(n.id == msg.dest_id)
                    {
                        int * p = (int*)msg.payload;
                        for(int i =0;i<*p;i++)
                        {
                            printf("%c",msg.payload[4+i]);
                        }

                    }
                    else
                    {
                        int * p = (int*)msg.payload;
                        n.send_by_id(n.dest_to_next_node[msg.dest_id],msg.dest_id,msg.payload+4,*p);
                    }
                }
            }
            if(msg.func_id == 8)//discovery
            {
                int* p = (int*)msg.payload;
                if(n.id_src.find(msg.msg_id) != n.id_src.end())
                {
                    if(n.id_counter[msg.msg_id]< msg.trailing_msg)//trailinmsg is the counter in this case
                    {
                        n.id_counter[msg.msg_id] =  msg.trailing_msg;
                        n.id_src[msg.msg_id] = msg.src_id;
                    }
                    else
                    {
                           n.nack(fd, msg.msg_id, msg.src_id);
                           continue;
                    }
                }
                else
                {
                    n.id_counter[msg.msg_id] =  msg.trailing_msg;
                    n.id_src[msg.msg_id] = msg.src_id;
                }
                if(n.connected.find(*p) != n.connected.end())
                {
                    int route_payload[4];
                    route_payload[0] = msg.msg_id;
                    route_payload[1] = 2;
                    route_payload[2] = n.id;
                    route_payload[3] = *p;
                    n.dest_to_next_node[*p]=*p; //  i need to explain
                    message msg2(n.id, msg.src_id, 0, 16, (char*)route_payload, 16);
                    msg2.msg_id = msg.msg_id;
                    message::msg_counter--;
                    char buffer[512];
                    msg2.writetobuffer((void*)buffer);
                    send(fd, (void*)buffer, 512, 0);
                }
                else
                {
                    if(n.connected.size() == 1)
                    {
                        n.nack(fd, msg.msg_id, msg.src_id);
                        continue;
                    }
                    for(auto neighbor : n.connected)
                    {
                        if(neighbor.first == msg.src_id)
                        {
                            continue;
                        }
                        message msg2(n.id, neighbor.first, msg.trailing_msg+1, 8, msg.payload, 4);
                        msg2.msg_id = msg.msg_id;
                        message::msg_counter--;
                        char buffer[512];
                        msg2.writetobuffer((void*)buffer);
                        send(neighbor.second, (void*)buffer, 512, 0);
                    }
                }
            }
            if(msg.func_id == 16)//route
            {
                int* p = (int*)msg.payload;
                p+=p[1] + 1;
                int final_dest = *p;
                n.dest_to_next_node[final_dest] = msg.src_id;

                p =(int*)msg.payload;
                p++;
                //showing the msg path for debugging prefernce
                // for (size_t i = 1; i <= *p; i++)
                // {
                //     cout << p[i] << " ";
                // }
                // cout << endl;
                if(n.id_src.find(msg.msg_id) != n.id_src.end())
                {
                    int id_to_return_to = n.id_src[msg.msg_id];
                    int * new_route_payload = new int[2+ (*p) +1];
                    new_route_payload[0] = msg.msg_id;
                    new_route_payload[1] = *p+1;
                    new_route_payload[2] = n.id;
                    memcpy((void*)(new_route_payload+3),(void*)(p+1),(*p)*4);
                    message msg2 (n.id,id_to_return_to,0,16,(char*)new_route_payload,(2+ (*p) +1)*4);
                    msg2.msg_id = msg.msg_id;
                    message::msg_counter--;
                    delete new_route_payload;
                    char buffer[512];
                    msg2.writetobuffer((void*)buffer);
                    send(n.connected[id_to_return_to], (void*)buffer, 512, 0);
                }
                else
                {            
                    string str_payload = n.dest_to_string[final_dest];
                    n.send_by_id(p[1],final_dest,(char*)str_payload.c_str(), str_payload.length());
                }
            }
            //if(msg.func_id == )
        }
    }

    return 0;
}