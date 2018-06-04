#include <unistd.h>  
#include "stdio.h"
#include <iostream>  
#include <sys/socket.h>  
#include <arpa/inet.h>  
#include <errno.h>  
#include <string.h>
#include <cstdlib>
#include<stdlib.h>
    
class my_client  
{  
private:  
    int socket_fd;  
    char message[4096];  
    struct sockaddr_in server_addr;  
    
public:  
    void init_socket(char* server_ip,char* server_port);
    void send_msg(char* toserver);  
    void close_socket();
};  