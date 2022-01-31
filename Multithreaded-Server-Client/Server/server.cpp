#include "server.h"

// Global entities
int *client_server_fds = NULL;
int thread_num;
pthread_t *client_threads = NULL;
std::queue<int> client_fd_queue;
sem_t queue_sem;
pthread_mutex_t queue_mutex;

// The dictionary that stores data
std::vector<std::string> dictionary(101, "");
std::vector<pthread_mutex_t> key_mutexes(101, PTHREAD_MUTEX_INITIALIZER);
std::vector<bool> key_exists(101, false);

int main(int argc, char *argv[])
{
    // Two arguments must be specified
    if (argc != 2)
    {
        std::cout << "Incorrect number of arguments specified. Please try again.\n";
        return 1;
    }

    int wel_socket_fd, client_socket_fd, port_number;
    socklen_t client_length;
    struct sockaddr_in server_address_obj, client_address_obj;

    // Intialise the server entities
    init_server(atoi(argv[1]));
    create_server_threads(atoi(argv[1]));

    // Creating server socket for arbitrary clients to make initial contact
    wel_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (wel_socket_fd < 0)
    {
        printf("Error creating welcoming socket\n");
        return 1;
    }

    bzero((char *)&server_address_obj, sizeof(server_address_obj));

    port_number = 8001;
    // Specifying IPv4
    server_address_obj.sin_family = AF_INET;
    // Binding the port to all available interfaces
    server_address_obj.sin_addr.s_addr = INADDR_ANY;
    // Translates host byte order to network byte order
    server_address_obj.sin_port = htons(port_number);

    // Binding the socket to the port
    if (bind(wel_socket_fd, (struct sockaddr *)&server_address_obj, sizeof(server_address_obj)) < 0)
    {
        printf("Error binding socket\n");
        return 1;
    }

    // Listening for incoming connections
    listen(wel_socket_fd, atoi(argv[1]));
    client_length = sizeof(client_address_obj);

    // Running a loop to keep accepting connections
    while (true)
    {
        // Accepting incoming connections
        client_socket_fd = accept(wel_socket_fd, (struct sockaddr *)&client_address_obj, &client_length);
        if (client_socket_fd < 0)
        {
            printf("Error accepting connection.\n");
            return 1;
        }

        // Storing the client socket ID in the queue
        pthread_mutex_lock(&queue_mutex);
        client_fd_queue.push(client_socket_fd);
        pthread_mutex_unlock(&queue_mutex);
        sem_post(&queue_sem);
    }

    // Closing the socket, performing cleanup and exiting the program
    close(wel_socket_fd);
    return 0;
}