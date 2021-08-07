#ifndef SELECT_H
#define SELECT_H

int add_fd_to_monitoring(const unsigned int fd);
int wait_for_input();

//our funcs
void printfds();
void close_all();
#endif