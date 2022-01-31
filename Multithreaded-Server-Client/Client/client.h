#ifndef __CLIENT_H__
#define __CLIENT_H__

// Header files
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <iostream>
#include <semaphore.h>
#include <assert.h>
#include <queue>
#include <vector>
#include <tuple>
#include <pthread.h>

// Colours
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"
#define RESET "\x1B[0m"

// Constants
#define MAX_BUFFER_SIZE 1024

// Structure definition
struct client_data
{
    pthread_t tid;
    int index;
    std::string command;
    int time;
};

// Global print lock
extern pthread_mutex_t print_lock;

// Function declarations
std::pair<std::string, int> read_string_from_socket(int socket_fd);
void send_string_on_socket(std::string &string, int socket_fd);
int get_socket_fd(struct sockaddr_in *address);
void begin_process(std::vector<std::pair<int, std::string>> &commands, int n);

#endif