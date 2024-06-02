#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include <signal.h>
#include <netdb.h>

#define SA struct sockaddr 

using namespace std;

bool running = true;


void server(int port, bool handle_output = false, bool udp = false) {
    struct sockaddr_in server_addr;
    int server_sock, new_socket;
    if (!udp) server_sock = socket(AF_INET, SOCK_STREAM, 0);
    else server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_sock < 0) exit(EXIT_FAILURE);

    int opt = 1;
    if (!udp)
        if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) 
            exit(EXIT_FAILURE);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    int addrlen = sizeof(server_addr);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        exit(EXIT_FAILURE);

    if (!udp){
        if (listen(server_sock, 1) < 0)
            exit(EXIT_FAILURE);
        if ((new_socket = accept(server_sock, (SA*)&server_addr, (socklen_t*)&addrlen)) < 0) 
            exit(EXIT_FAILURE);
        if (handle_output)
            if (dup2(new_socket, STDOUT_FILENO) < 0) 
                exit(EXIT_FAILURE);
        if (dup2(new_socket, STDIN_FILENO) < 0) 
            exit(EXIT_FAILURE);
    }
    else{
        struct sockaddr_in client_addr = {};
        socklen_t client_len = sizeof(client_addr);
        char buffer[1024];
        ssize_t n = recvfrom(server_sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_len);
        if (n < 0)
            exit(EXIT_FAILURE);
        if (dup2(server_sock, STDIN_FILENO) < 0) 
            exit(EXIT_FAILURE);
    }
        
}





void client(const string &hostname, int port, bool udp = false)
{
    struct sockaddr_in server_addr;
    int client_sock;
    if (!udp) client_sock = socket(AF_INET, SOCK_STREAM, 0);
    else client_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_sock < 0) exit(EXIT_FAILURE);

    struct hostent *server = gethostbyname(hostname.c_str());
    if (server == nullptr) exit(EXIT_FAILURE);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (!udp){
        if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
            exit(EXIT_FAILURE);
        dup2(client_sock, STDOUT_FILENO);

    }
    else {
        dup2(client_sock, STDOUT_FILENO);
        char buffer[1024];
        ssize_t n;
        while ((n = read(STDIN_FILENO, buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[n] = '\0';
            sendto(client_sock, buffer, n, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        }
    }
}

void ud_server(const string &path, bool stream = false)
{
    //ud = UNIX DOMAIN
    int server_sock;
    struct sockaddr_un server_addr;

    if (stream) server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    else server_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (server_sock < 0) exit(EXIT_FAILURE);

    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, path.c_str());

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        exit(EXIT_FAILURE);

    if (stream){
        if (listen(server_sock, 1) < 0)
            exit(EXIT_FAILURE);
        int client_sock = accept(server_sock, nullptr, nullptr);
        if (client_sock < 0)
            exit(EXIT_FAILURE);
        if (dup2(client_sock, STDIN_FILENO) < 0) exit(EXIT_FAILURE);
        
        close(server_sock);
    }
    else {
        struct sockaddr_un client_addr = {};
        socklen_t client_len = sizeof(client_addr);
        char buffer[1024];
        ssize_t n = recvfrom(server_sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_len);
        if (n < 0)
            exit(EXIT_FAILURE);
        if (dup2(server_sock, STDIN_FILENO) < 0) exit(EXIT_FAILURE);
    }
}



void ud_client(const string &path, bool stream = false)
{
    int client_sock;
    struct sockaddr_un client_addr;

    if (stream) client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    else client_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (client_sock < 0)
        exit(EXIT_FAILURE);

    client_addr.sun_family = AF_UNIX;
    strcpy(client_addr.sun_path, path.c_str());

    if (stream) {
        if (connect(client_sock, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
            exit(EXIT_FAILURE);
        if (dup2(client_sock, STDOUT_FILENO) < 0) 
            exit(EXIT_FAILURE);
    }
    else {
        if (dup2(client_sock, STDOUT_FILENO) < 0) 
            exit(EXIT_FAILURE);
        char buffer[1024];
        ssize_t n;
        while ((n = read(STDIN_FILENO, buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[n] = '\0';
            sendto(client_sock, buffer, n, 0, NULL, 0);
        }
        close(client_sock);
    }

}

void alarm_handler(int signal)
{
    if (signal == SIGALRM)
    {
        running = false;
    }
}


int main(int argc, char* argv[]) {

    signal(SIGALRM, alarm_handler);

    int opt;
    string input_source, output_dest;
    string command;
    int timeout = -1;
    while ((opt = getopt(argc, argv, "e:i:o:b:t:")) != -1) {
        switch (opt) {
            case 'i':
                input_source = optarg;
                break;
            case 'o':
                output_dest = optarg;
                break;
            case 'b':
                input_source = optarg;
                output_dest = optarg;
                break;
            case 'e':
                command = string(optarg);
                break;
            case 't':
                timeout = atoi(optarg);
                break;
            default:
                break;
        }
    }
    if (timeout > 0) alarm(timeout);

    if (command.empty()) command = "ttt 123456789";

    command = "./" + command;
    stringstream s(command);
    
    vector<char*> args;
    for (string arg; s >> arg;)
        args.push_back(strdup(arg.c_str()));
    args.push_back(nullptr);
    
    pid_t pid = fork();
    if (pid == 0) {
        // server

        // if o is filled
        if (!output_dest.empty()) {
            if (output_dest[2] != 'S') {
                
                size_t comma_pos = output_dest.find(',');
                string ip = output_dest.substr(4, comma_pos - 4);
                int port = stoi(output_dest.substr(comma_pos + 1));
                bool udp = output_dest.substr(0, 3) == "UDP";
                client(ip, port, udp);
            }
            else {
                bool stream = output_dest[4] == 'S';
                string path = output_dest.substr(5);
                ud_client(path, stream);
            }
        }

        // if i is filled
        if (!input_source.empty()) {
            if (input_source[2] != 'S') {
                int port = stoi(input_source.substr(4));
                bool udp = input_source.substr(0, 3) == "UDP";
                bool handle_output = (input_source == output_dest);
                server(port, handle_output, udp);
            } else {
                bool stream = input_source[4] == 'S';
                string path = input_source.substr(5);
                ud_server(path, stream);
            }
        }

        setvbuf(stdout, nullptr, _IOLBF, BUFSIZ);

        execvp(args[0], args.data());
        return 1;
    }
    else if(pid < 0) {
        cerr << "Error: Failed to fork" << endl;
        return 1;
    }
    for (;running && waitpid(pid, nullptr, 0); sleep(1));
    if (!running) kill(pid, SIGTERM);

    return 0;
}