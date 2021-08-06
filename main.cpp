#include <stdio.h>
#include <string.h>
#include <vector>
#include <sstream>
#include <sys/socket.h>
#include <iostream>
#include <sys/types.h>

#include "select.h"
#include "message.hpp"
#include "node.hpp"

using namespace std;

int main(int argc, char** argv)
{
    if(argc < 2)
        return 1;
    int port = atoi(argv[1]);
    node n(port);
    char cmd [1024];
    bool setid_done = false;
    while(1)
    {
        cout << "start waiting for input" << endl;
        int fd=0;
        if (setid_done)
            fd = wait_for_input();
        cout << "start waiting for input2" << endl;
        cout << fd << endl;
        cout << "start waiting for input3" << endl;
        if(fd == 0)
        {
            fgets(cmd,1024,stdin);
            string s {cmd};
            string command = s.substr(0,s.find(","));
            cout << command << endl;
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
                cout << "here\n";
                n.conn((char *)ip.c_str(),port);
            }
            else if(command == "setid")
            {
                printf("inside setid\n");
                s.erase(0,s.find(",")+1);
                n.setid(atoi(s.c_str()));
                setid_done = true;
            }
        }
        else
        {
            char buffer[512];
            recv(fd,(void*)buffer,512,0);
            message msg(buffer);
            if(msg.func_id == 4)
            {
                n.ack(fd, msg.msg_id, msg.src_id);
            }
            if(msg.func_id == 1)
            {
                printf("ack\n");
            }
        }
    }

    return 0;
}