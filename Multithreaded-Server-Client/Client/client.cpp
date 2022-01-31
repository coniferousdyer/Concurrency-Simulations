#include "client.h"

pthread_mutex_t print_lock;

int main(int argc, char *argv[])
{
    pthread_mutex_init(&print_lock, NULL);
    
    int n;
    std::cin >> n;

    std::vector<std::pair<int, std::string>> commands;

    // Storing commands entered by the user
    for (int i = 0; i < n; i++)
    {
        std::string command_string;
        int time;
        std::cin >> time;
        std::getline(std::cin >> std::ws, command_string);
        commands.push_back({time, command_string});
    }

    begin_process(commands, n);
    return 0;
}