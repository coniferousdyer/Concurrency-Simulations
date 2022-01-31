#ifndef __SERVER_H__
#define __SERVER_H__

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
#include <iostream>
#include <assert.h>
#include <tuple>
#include <vector>
#include <queue>
#include <map>
#include <pthread.h>
#include <semaphore.h>

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

// Global entities
extern int thread_num;
extern pthread_t *client_threads;
extern std::queue<int> client_fd_queue;
extern sem_t queue_sem;
extern pthread_mutex_t queue_mutex;

// The dictionary that stores data
extern std::vector<std::string> dictionary;
extern std::vector<pthread_mutex_t> key_mutexes;
extern std::vector<bool> key_exists;

// Function declarations
std::pair<std::string, int> read_from_socket(int sockfd);
int send_to_socket(int sockfd, std::string response);
std::vector<std::string> command_parser(std::string command);
std::string execute_command(std::vector<std::string> tokens);
void *handle_client(void *arg);
void create_server_threads(int n);
void init_server(int n);

#endif