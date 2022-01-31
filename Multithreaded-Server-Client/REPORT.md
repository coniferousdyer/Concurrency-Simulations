# Q3: Multithreaded Client and Server

<b>Context: </b>Multiple clients making requests to a single server program.

## Logic

The server program consists of two parts: a main thread (the handler) and a thread pool (the workers). The handler handles incoming requests and indirectly hands over client socket IDs to the workers. The workers are responsible for reading and processing the requests, and sending back the response to the assigned client.

The main queue is used to store the client socket IDs. The main queue is a shared resource between the handler and the workers. The handler only pushes client socket IDs to the queue and the worker threads only pop the client socket IDs from the queue. In this manner, the worker threads are assigned the clients whenever they become free.

The following is the flow of the entire client-server communication:

* The client program creates a thread for each request and creates a socket, sending a connection request to the server.

* The main server thread (the handler) has the `accept()` system call, which listens for connections, accepts new incoming connections and returns the client socket ID. The `accept()` system call also comes with the benefit of ensuring that it gets blocked until one of the worker threads becomes free.

* The handler then pushes this new client socket ID to the main queue.

* Each worker thread then pops a client socket ID from the front of the queue. Thus, the worker thread gets assigned a client thread request. A queue lock has been used to ensure that there are no issues during this process.

* The queueing system has been supported by the use of a semaphore. This ensures that when the queue is empty, the client threads pause execution until the handler pushes a new client socket ID to the queue. That is, the worker threads use `sem_wait()` and the handler uses `sem_post()`.

* The worker threads then read the request from the client socket ID and process it.

* The dictionary, which has been implemented as a vector of strings (rather than a map, which is thread-unsafe), is then updated accordingly. The presence of the key in the dictionary is indicated by a Boolean vector `key_exists`, and an vector of mutexes `key_mutexes` ensures the thread-safety of the dictionary.

* The worker threads then send the response back to the client socket ID, and the client socket ID prints it.

## File Structure

```
q3/
|___Client/
|   |___client.cpp
|   |___client.h
|   |___client_functions.cpp
|
|___Server/
|   |___server.cpp
|   |___server.h
|   |___server_functions.cpp
|
|___REPORT.md
|___Makefile
```

## Compilation

1. To compile both the server and client programs,
```bash
$ make server; make client;
```
2. Run the server first,
```bash
$ ./server <number of threads>
```
3. Then run the client program.
```bash
$ ./client
```