#include "server.h"

// Function to receive command from client
std::pair<std::string, int> read_from_socket(int sockfd)
{
    std::string command;
    command.resize(MAX_BUFFER_SIZE);

    int bytes_received = read(sockfd, &command[0], MAX_BUFFER_SIZE - 1);
    if (bytes_received <= 0)
        std::cerr << "Error reading from socket"
                  << "\n";

    command[bytes_received] = '\0';
    command.resize(bytes_received);
    return std::make_pair(command, bytes_received);
}

// Function to return a response to the client
int send_to_socket(int sockfd, std::string response)
{
    int bytes_sent = write(sockfd, response.c_str(), response.size());
    if (bytes_sent < 0)
        std::cerr << "Error writing to socket"
                  << "\n";

    return bytes_sent;
}

// Function to parse the command from the client
std::vector<std::string> command_parser(std::string command)
{
    std::vector<std::string> tokens;
    std::string delimiter = " ";
    size_t pos = 0, prev = 0;
    std::string token;

    // Parsing the string to get the command and the arguments
    while ((pos = command.find(delimiter, pos)) != std::string::npos)
    {
        token = command.substr(prev, pos - prev);
        tokens.push_back(token);
        prev = ++pos;
    }

    tokens.push_back(command.substr(prev, pos));
    return tokens;
}

// Function to execute the parsed command
std::string execute_command(std::vector<std::string> tokens)
{
    std::string response;

    // Insert key-value pair
    if (tokens[0] == "insert")
    {
        // Checking if the key exists
        if (!key_exists[std::stoi(tokens[1])])
        {
            pthread_mutex_lock(&key_mutexes[std::stoi(tokens[1])]);
            dictionary[std::stoi(tokens[1])] = tokens[2];
            key_exists[std::stoi(tokens[1])] = true;
            response = std::to_string(pthread_self()) + ":Insertion successful";
            pthread_mutex_unlock(&key_mutexes[std::stoi(tokens[1])]);
        }
        else
            response = std::to_string(pthread_self()) + ":Key already exists";
    }
    // Delete key-value pair
    else if (tokens[0] == "delete")
    {
        // Checking if the key exists
        if (key_exists[std::stoi(tokens[1])])
        {
            pthread_mutex_lock(&key_mutexes[std::stoi(tokens[1])]);
            dictionary[std::stoi(tokens[1])] = "";
            key_exists[std::stoi(tokens[1])] = false;
            response = std::to_string(pthread_self()) + ":Deletion successful";
            pthread_mutex_unlock(&key_mutexes[std::stoi(tokens[1])]);
        }
        else
            response = std::to_string(pthread_self()) + ":No such key exists";
    }
    // Fetch value corresponding to key
    else if (tokens[0] == "fetch")
    {
        // Checking if the key exists
        if (key_exists[std::stoi(tokens[1])])
        {
            pthread_mutex_lock(&key_mutexes[std::stoi(tokens[1])]);
            response = std::to_string(pthread_self()) + ":" + dictionary[std::stoi(tokens[1])] + "\n";
            pthread_mutex_unlock(&key_mutexes[std::stoi(tokens[1])]);
        }
        else
            response = std::to_string(pthread_self()) + ":Key does not exist";
    }
    // Concatenate key-value pairs
    else if (tokens[0] == "concat")
    {
        if (key_exists[std::stoi(tokens[1])] && key_exists[std::stoi(tokens[2])])
        {
            pthread_mutex_lock(&key_mutexes[std::stoi(tokens[1])]);
            pthread_mutex_lock(&key_mutexes[std::stoi(tokens[2])]);
            std::string temp_1 = dictionary[std::stoi(tokens[1])];
            std::string temp_2 = dictionary[std::stoi(tokens[2])];
            dictionary[std::stoi(tokens[1])] = temp_1 + temp_2;
            dictionary[std::stoi(tokens[2])] = temp_2 + temp_1;
            response = std::to_string(pthread_self()) + ":" + dictionary[std::stoi(tokens[2])];
            pthread_mutex_unlock(&key_mutexes[std::stoi(tokens[2])]);
            pthread_mutex_unlock(&key_mutexes[std::stoi(tokens[1])]);
        }
        else
            response = std::to_string(pthread_self()) + ":Concat failed as at least one of the keys does not exist";
    }
    // Update key-value pair
    else if (tokens[0] == "update")
    {
        // Checking if the key exists
        if (key_exists[std::stoi(tokens[1])])
        {
            pthread_mutex_lock(&key_mutexes[std::stoi(tokens[1])]);
            dictionary[std::stoi(tokens[1])] = tokens[2];
            response = std::to_string(pthread_self()) + ":" + dictionary[std::stoi(tokens[1])];
            pthread_mutex_unlock(&key_mutexes[std::stoi(tokens[1])]);
        }
        else
            response = std::to_string(pthread_self()) + ":Key does not exist";
    }
    else
        response = std::to_string(pthread_self()) + ":Invalid command";

    return response;
}

// The thread that reads the command from the client socket and parses it
void *handle_client(void *arg)
{
    // One iteration of the loop services one client
    while (true)
    {
        // Getting the latest client socket ID from the queue and popping it
        sem_wait(&queue_sem);
        pthread_mutex_lock(&queue_mutex);
        int client_socket_fd = client_fd_queue.front();
        client_fd_queue.pop();
        pthread_mutex_unlock(&queue_mutex);

        int received_bytes = 0, sent_bytes = 0;
        std::string command;
        std::tie(command, received_bytes) = read_from_socket(client_socket_fd);

        if (received_bytes <= 0)
        {
            std::cout << "Server could not read message from the client.\n";
            break;
        }

        // Parse the command
        std::vector<std::string> tokens = command_parser(command);
        std::string response = execute_command(tokens);

        sleep(2);

        sent_bytes = send_to_socket(client_socket_fd, response);
        if (sent_bytes <= 0)
        {
            std::cout << "Server could not send message to the client (socket may have been closed).\n";
            break;
        }
    }

    pthread_exit(NULL);
}

// Function to create a pool of worker threads to serve client requests
void create_server_threads(int n)
{
    for (int i = 0; i < n; i++)
    {
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, NULL);
        client_threads[i] = thread_id;
    }
}

// The initialisation function
void init_server(int n)
{
    thread_num = n;
    client_threads = new pthread_t[n];
    sem_init(&queue_sem, 0, 0);
    pthread_mutex_init(&queue_mutex, NULL);
}

// The cleanup function
void cleanup_server()
{
    for (int i = 0; i < 101; i++)
        pthread_mutex_destroy(&key_mutexes[i]);
    sem_destroy(&queue_sem);
    pthread_mutex_destroy(&queue_mutex);
    for (int i = 0; i < thread_num; i++)
        pthread_join(client_threads[i], NULL);
    delete[] client_threads;
}