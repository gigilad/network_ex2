#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <netinet/tcp.h>

#define BUFFER_SIZE 2048

/*
* @brief A random data generator function based on srand() and rand().
* @param size The size of the data to generate (up to 2^32 bytes).
* @return A pointer to the buffer.
*/
char *util_generate_random_data(unsigned int size) {
    char *buffer = NULL;
    // Argument check.
    if (size == 0)
        return NULL;
    buffer = (char *)calloc(size, sizeof(char));
    // Error checking.
    if (buffer == NULL)
        return NULL;
    // Randomize the seed of the random number generator.
    srand(time(NULL));
    for (unsigned int i = 0; i < size; i++)
        *(buffer + i) = ((unsigned int)rand() % 256);
    return buffer;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s -p port -algo ALGO\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[2]);
    char *algorithm = argv[4];

    // Create a TCP socket
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Error in creating socket");
        exit(EXIT_FAILURE);
    }

    // Set TCP congestion control algorithm
    if (setsockopt(server_sock, IPPROTO_TCP, TCP_CONGESTION, algorithm, strlen(algorithm)) < 0) {
        perror("Error setting TCP congestion control algorithm");
        exit(EXIT_FAILURE);
    }

    // Initialize server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    // Bind socket to the address and port
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error in binding");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_sock, 5) < 0) {
        perror("Error in listening");
        exit(EXIT_FAILURE);
    }

    printf("Receiver is waiting for connection...\n");
    printf("Using TCP congestion control algorithm: %s\n", algorithm);

    double total_time = 0.0;
    double total_bandwidth = 0.0;
    int count = 0;
    int exit_requested = 0;

    while (!exit_requested) {
        // Accept incoming connection
        socklen_t client_len;
        int client_sock = accept(server_sock, (struct sockaddr *)NULL, &client_len);
        if (client_sock < 0) {
            perror("Error in accepting connection");
            exit(EXIT_FAILURE);
        }

        printf("Connection established with sender.\n");

        while (1) {
            // Start measuring time
            clock_t start_time = clock();

            // Receive file data
            char buffer[BUFFER_SIZE];
            ssize_t bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0);
            if (bytes_received < 0) {
                perror("Error in receiving file data");
                exit(EXIT_FAILURE);
            } else if (bytes_received == 0) {
                printf("Connection closed by sender. Exiting.\n");
                close(client_sock);
                exit_requested = 1;
                break;
            } else if (bytes_received == strlen("Exit") && strncmp(buffer, "Exit", strlen("Exit")) == 0) {
                printf("Exit message received. Closing connection.\n");
                close(client_sock);
                exit_requested = 1;
                break;
            }

            // Stop measuring time
            clock_t end_time = clock();
            double elapsed_time = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;

            // Save received file (You can modify this part to save the file)
            FILE *file = fopen("received_file.txt", "ab"); // Append mode for continuous writing
            if (file == NULL) {
                perror("Error in opening file");
                exit(EXIT_FAILURE);
            }
            fwrite(buffer, 1, bytes_received, file);
            fclose(file);

            double bandwidth = bytes_received / elapsed_time * 1000.0;
            total_time += elapsed_time;
            total_bandwidth += bandwidth;
            count++;

            // Print individual statistics
            printf("- Run #%d Data: Time=%.2fms, Speed=%.2fMB/s\n", count, elapsed_time, bandwidth);
        }
    }

    if (count > 0) {
        // Print all times and bandwidths
        printf("- * Statistics * -\n");
        printf("- Average time: %.2fms\n", total_time / count);
        printf("- Average bandwidth: %.2fMB/s\n", total_bandwidth / count);
    }

    // Close server socket
    close(server_sock);

    return 0;
}
