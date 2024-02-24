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
    if (argc != 7) {
        printf("Usage: %s -ip IP -p PORT -algo ALGO\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *ip = NULL;
    int port = 0;
    char *algorithm = NULL;

    // Parse command line arguments
    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-ip") == 0) {
            ip = argv[i + 1];
        } else if (strcmp(argv[i], "-p") == 0) {
            port = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-algo") == 0) {
            algorithm = argv[i + 1];
        } else {
            printf("Invalid argument: %s\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }

    // Check if required arguments are provided
    if (ip == NULL || port == 0 || algorithm == NULL) {
        printf("Missing required arguments.\n");
        printf("Usage: %s -ip IP -p PORT -algo ALGO\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Check if the congestion control algorithm is valid
    if (strcmp(algorithm, "reno") != 0 && strcmp(algorithm, "cubic") != 0) {
        printf("Invalid congestion control algorithm. Use 'reno' or 'cubic'.\n");
        exit(EXIT_FAILURE);
    }

    // Create random data
    char *random_data = util_generate_random_data(BUFFER_SIZE);
    if (random_data == NULL) {
        perror("Error generating random data");
        exit(EXIT_FAILURE);
    }

    // Create a TCP socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Initialize server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    // Connect to receiver
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting");
        exit(EXIT_FAILURE);
    }

    printf("Connected to receiver.\n");
    printf("Using TCP congestion control algorithm: %s\n", algorithm);

    while (1) {
        // Send the file data
        ssize_t bytes_sent = send(sock, random_data, BUFFER_SIZE, 0);
        if (bytes_sent < 0) {
            perror("Error sending file data");
            exit(EXIT_FAILURE);
        }

        printf("File sent successfully.\n");

        // Ask user if they want to send the file again
        char response[5];
        printf("Do you want to send the file again? (yes/no): ");
        scanf("%s", response);
        if (strcmp(response, "no") == 0) {
            // Send exit message and close connection
            if (send(sock, "Exit", strlen("Exit"), 0) != strlen("Exit")) {
                perror("Error sending exit message");
                exit(EXIT_FAILURE);
            }
            close(sock);
            printf("Exit message sent. Connection closed.\n");
            break;
        }
    }

    return 0;
}

