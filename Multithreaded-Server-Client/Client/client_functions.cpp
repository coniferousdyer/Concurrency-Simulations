#include "client.h"

// Function to read data from server socket
std::pair<std::string, int> read_string_from_socket(int socket_fd)
{
    char buffer[MAX_BUFFER_SIZE];
    int bytes_read = read(socket_fd, buffer, MAX_BUFFER_SIZE - 1);
    if (bytes_read <= 0)
    {
        pthread_mutex_lock(&print_lock);
        std::cout << "Error reading from server\n";
        pthread_mutex_unlock(&print_lock);
        exit(EXIT_FAILURE);
    }
    std::string data(buffer);

    return std::make_pair(data, bytes_read);
}

// Function to write data to server socket
void send_string_on_socket(std::string &string, int socket_fd)
{
    int bytes_sent = write(socket_fd, string.c_str(), string.length());
    if (bytes_sent < 0)
    {
        pthread_mutex_lock(&print_lock);
        std::cerr << "Error writing to socket" << std::endl;
        pthread_mutex_unlock(&print_lock);
        exit(EXIT_FAILURE);
    }
}

// Function to create a socket, connect to the server and return the file descriptor
int get_socket_fd(struct sockaddr_in *address)
{
    struct sockaddr_in server_address_obj = *address;

    // Creating socket file descriptor
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        pthread_mutex_lock(&print_lock);
        std::cerr << "Error creating socket" << std::endl;
        pthread_mutex_unlock(&print_lock);
        exit(EXIT_FAILURE);
    }

    // Initial setup of the socket
    int port_number = 8001;
    memset(&server_address_obj, 0, sizeof(server_address_obj));
    server_address_obj.sin_family = AF_INET;
    server_address_obj.sin_port = htons(port_number);

    // Connecting to the server
    if (connect(socket_fd, (struct sockaddr *)&server_address_obj, sizeof(server_address_obj)) < 0)
    {
        pthread_mutex_lock(&print_lock);
        perror("Error in connecting to server");
        pthread_mutex_unlock(&print_lock);
        exit(EXIT_FAILURE);
    }

    return socket_fd;
}

// The function run by the client thread
void *client_thread(void *arg)
{
    // Extracting the client data from the argument
    client_data client_data_obj = *(client_data *)arg;
    int index = client_data_obj.index;
    std::string command = client_data_obj.command;
    int time = client_data_obj.time;

    // Connecting the user thread to server
    struct sockaddr_in server_address_obj;
    int server_socket_fd = get_socket_fd(&server_address_obj);

    // Sleeping for the specified time
    sleep(time);

    // Sending data to server
    send_string_on_socket(command, server_socket_fd);

    int num_bytes_read;
    std::string response;

    // Read response from server
    std::tie(response, num_bytes_read) = read_string_from_socket(server_socket_fd);
    pthread_mutex_lock(&print_lock);
    std::cout << index << ":" << response << "\n";
    pthread_mutex_unlock(&print_lock);

    pthread_exit(NULL);
}

// Function to create the thread for each command
void begin_process(std::vector<std::pair<int, std::string>> &commands, int n)
{
    client_data client_data_objs[n];

    // Creating the threads
    for (int i = 0; i < n; i++)
    {
        client_data_objs[i].index = i;
        client_data_objs[i].command = commands[i].second;
        client_data_objs[i].time = commands[i].first;

        pthread_create(&client_data_objs[i].tid, NULL, client_thread, (void *)&client_data_objs[i]);
    }

    // Joining the threads
    for (int i = 0; i < n; i++)
        pthread_join(client_data_objs[i].tid, NULL);

    pthread_mutex_destroy(&print_lock);
}