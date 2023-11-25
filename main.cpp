#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>  // Added for inet_pton
#include <unistd.h>
#include <cstdlib>

const int PORT = 8002;
const int BUFFER_SIZE = 1024;
const char *SERVER_LIST_FILE = "server_list.txt";

// Function to read server list from file
std::vector<std::string> readServerList() {
    std::vector<std::string> serverList;
    std::ifstream file(SERVER_LIST_FILE);

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            serverList.push_back(line);
        }
        file.close();
    }

    return serverList;
}

// Function to write server list to file
void writeServerList(const std::vector<std::string>& serverList) {
    std::ofstream file(SERVER_LIST_FILE);

    if (file.is_open()) {
        for (const std::string& server : serverList) {
            file << server << '\n';
        }
        file.close();
    }
}

// Function to check if a server is already in the list
bool isServerInList(const std::vector<std::string>& serverList, const std::string& newServer) {
    return std::find(serverList.begin(), serverList.end(), newServer) != serverList.end();
}

// Function to remove a server from the list
bool removeServerFromList(const std::string& server, std::vector<std::string>& serverList) {
    auto it = std::find(serverList.begin(), serverList.end(), server);
    if (it != serverList.end()) {
        serverList.erase(it);
        return true;
    } else {
        return false;
    }
}

// Function to handle the "remove_server" request
void handleRemoveServer(const std::string& paramValue, int client_socket) {
    // Check if the server is in the list
    std::vector<std::string> serverList = readServerList();

    if (removeServerFromList(paramValue, serverList)) {
        // Update the file with the new server list
        writeServerList(serverList);

        // Send a response indicating that the remove_server request was successful
        const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nRemove Server request successful";
        send(client_socket, response, strlen(response), 0);
    } else {
        // If the server is not in the list, send a response indicating that it does not exist
        const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nServer does not exist in the list";
        send(client_socket, response, strlen(response), 0);
    }
}

// Function to check if the given string is a valid IP address
bool isValidIPAddress(const std::string& ipAddress) {
    bool ret = false;
    struct sockaddr_in sa;
    ret = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr)) != 0;
    std::string out = ret?"good":"bad";
    std::cout << __func__ << ": " << out << ", ip: " << ipAddress<< std::endl;
    return ret;
}

// Function to check if the given IP address is a private (internal) address
bool isPrivateIPAddress(const std::string& ipAddress) {
    bool ret = false;
    struct sockaddr_in sa;
    if (inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr)) != 0) {
        uint32_t ip = ntohl(sa.sin_addr.s_addr);
        ret = ((ip >> 24) == 10) ||                           // 10.0.0.0/8
               ((ip >> 20) == 0xAC1) ||                         // 172.16.0.0/12
               ((ip >> 16) == 0xC0A8);                          // 192.168.0.0/16
    }
    std::string out = ret?"good":"bad";
    std::cout << __func__ << ": " << out << ", ip: " << ipAddress<< std::endl;
    return ret;
}

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
                        // Read server list from file
                        std::vector<std::string> serverList = readServerList();

                        // Send the JSON response with server list
                        std::ostringstream response;
                        response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n[";
                        for (size_t i = 0; i < serverList.size(); ++i) {
                            response << "\"" << serverList[i] << "\"";
                            if (i < serverList.size() - 1) {
                                response << ",";
                            }
                        }
                        response << "]";
                        send(client_socket, response.str().c_str(), response.str().length(), 0);
                    } else if (strcmp(param, "add_server") == 0) {
                        // Process the add_server parameter
                        // For simplicity, we'll just print the IP address
                        std::cout << "Add Server: " << paramValue << std::endl;

                        // Extract IP address from the parameter (remove port if present)
                        size_t colonPos = paramValue.find(':');
                        std::string ipAddress = (colonPos != std::string::npos) ? paramValue.substr(0, colonPos) : paramValue;

                        // Check if the parameter is a valid IP address and not a private (internal) address
                        if (isValidIPAddress(ipAddress) && !isPrivateIPAddress(ipAddress)) {
                            // Check if the server is in the list
                            std::vector<std::string> serverList = readServerList();
                            if (!isServerInList(serverList, paramValue)) {
                                // If not in the list, add to the list
                                serverList.push_back(paramValue);
                                // Update the file with the new server list
                                writeServerList(serverList);

                                // Send a response indicating that the add_server request was successful
                                const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nAdd Server request successful";
                                send(client_socket, response, strlen(response), 0);
                            } else {
                                // If the server is already in the list, send a response indicating that it already exists
                                const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nServer already exists in the list";
                                send(client_socket, response, strlen(response), 0);
                            }
                        } else {
                            // If the parameter is not a valid IP address or is a private address, send an error response
                            const char *response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nInvalid or private IP address";
                            send(client_socket, response, strlen(response), 0);
                        }
                    } else if (strcmp(param, "remove_server") == 0) {
                        // Process the remove_server parameter
                        // For simplicity, we'll just print the IP address
                        std::cout << "Remove Server: " << paramValue << std::endl;

                        // Handle the remove_server request
                        handleRemoveServer(paramValue, client_socket);
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

