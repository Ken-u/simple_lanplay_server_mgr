#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdlib>

const int PORT = 8080;
const int BUFFER_SIZE = 1024;

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

    if (bytes_received == -1) {
        std::cerr << "Error receiving data from client" << std::endl;
        close(client_socket);
        return;
    }

    buffer[bytes_received] = '\0'; // Null-terminate the received data

    // Find the start of the query string
    char *query_start = strstr(buffer, "GET /");
    if (query_start != nullptr) {
        // Move to the start of the actual query string
        query_start += 5;

        // Find the end of the query string
        char *query_end = strchr(query_start, ' ');
        if (query_end != nullptr) {
            // Null-terminate the query string
            *query_end = '\0';

            // Parse and process the query parameters
            char *param = strtok(query_start, "&");
            std::string paramValue;

            while (param != nullptr) {
                // Split the parameter into name and value
                char *equalSign = strchr(param, '=');
                if (equalSign != nullptr) {
                    *equalSign = '\0';
                    paramValue = equalSign + 1;

                    // Check the parameter name
                    if (strcmp(param, "get_server_list") == 0) {
                        // Process the get_server_list parameter
                        // Send the JSON response
                        const char *response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n[\"112.22\",\"233.33\"]";
                        send(client_socket, response, strlen(response), 0);
                    } else if (strcmp(param, "start_lanplay") == 0) {
                        // Process the start_lanplay parameter
                        // For simplicity, we'll just print the parameter value
                        std::cout << "Start LANPlay: " << paramValue << std::endl;

                        // TODO: Add your code to handle the start_lanplay parameter
                        // You might want to parse the IP address and port and perform some action
                        // For example, you could call a function like startLanPlay(paramValue.c_str());
                        // where startLanPlay is a function you implement to handle LANPlay requests.

                        // Send a response indicating that the LANPlay request was received
                        const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nLANPlay request received";
                        send(client_socket, response, strlen(response), 0);
                    } else if (strcmp(param, "add_server") == 0) {
                        // Process the add_server parameter
                        // For simplicity, we'll just print the IP address
                        std::cout << "Add Server: " << paramValue << std::endl;

                        // TODO: Add your code to handle the add_server parameter
                        // You might want to parse the IP address and perform some action
                        // For example, you could call a function like addServer(paramValue.c_str());
                        // where addServer is a function you implement to handle add_server requests.

                        // Send a response indicating that the add_server request was received
                        const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nAdd Server request received";
                        send(client_socket, response, strlen(response), 0);
                    }
                }

                param = strtok(nullptr, "&");
            }
        }
    }

    close(client_socket);
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Error binding socket" << std::endl;
        close(server_socket);
        return -1;
    }

    if (listen(server_socket, 5) == -1) {
        std::cerr << "Error listening on socket" << std::endl;
        close(server_socket);
        return -1;
    }

    std::cout << "Server listening on port " << PORT << "..." << std::endl;

    while (true) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1) {
            std::cerr << "Error accepting client connection" << std::endl;
            continue;
        }

        handle_client(client_socket);
    }

    close(server_socket);

    return 0;
}

