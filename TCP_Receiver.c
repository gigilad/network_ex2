#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 6000
#define MAX_BUFFER_SIZE 2048
#define EXIT_MESSAGE "exit"

int main(int args, char *argv[]) {
    printf("noooo5");
    if(args!=5){
        fprintf(stderr, "Usage: %s <ip> <port> <algorithm>\n", argv[0]);
        exit(EXIT_FAILURE);

    }
    char *ip = "127.0.0.1";
    int port =atoi(argv[2]);
    char *algorithm =argv[4];
    // Step 1: Create a TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    printf("i am listening ...");


    // Specify the address to bind
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    printf("i am listening 0 ...");

    // Step 2: Bind the socket
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error binding");
        exit(EXIT_FAILURE);
    }

    // Step 3: Listen for incoming connections
    if (listen(sockfd, 1) < 0) {
        perror("Error listening");
        exit(EXIT_FAILURE);
    }

    // Declare variables for measuring time
    clock_t start_time, end_time;
    double total_time = 0.0;
    unsigned long long total_bytes_received = 0;
    unsigned int file_count = 0;

    printf("i am listening ...");
    do {
        // Step 4: Accept connection from the sender
        struct sockaddr_in sender_addr;
        socklen_t sender_len = sizeof(sender_addr);
        int newsockfd = accept(sockfd, (struct sockaddr *)&sender_addr, &sender_len);
        if (newsockfd < 0) {
            perror("Error accepting connection");
            exit(EXIT_FAILURE);
        }

        // Measure the time it takes to receive the file
        start_time = clock();

        // Step 5: Receive the file
        char buffer[MAX_BUFFER_SIZE];
        ssize_t bytes_received;
        while ((bytes_received = recv(newsockfd, buffer, MAX_BUFFER_SIZE, 0)) > 0) {
            total_bytes_received += bytes_received;
        }
        if (bytes_received < 0) {
            perror("Error receiving file");
            exit(EXIT_FAILURE);
        }else if (bytes_received == 0) {
            // Connection closed by sender
            printf("Connection closed by sender\n");
            break;
        }

        // Calculate the time taken to receive the file
        end_time = clock();
        double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
        total_time += elapsed_time;

        // Increment file count
        file_count++;

        // Print out time taken to receive the file
        printf("Time taken to receive file %u: %.2f seconds\n", file_count, elapsed_time);

        // Step 6: Wait for Sender response
        char response[MAX_BUFFER_SIZE];
        bytes_received = recv(newsockfd, response, MAX_BUFFER_SIZE, 0);
        if (bytes_received < 0) {
            perror("Error receiving response from Sender");
            exit(EXIT_FAILURE);
        }

        // Check if Sender sent exit message
        if (strncmp(response, EXIT_MESSAGE, strlen(EXIT_MESSAGE)) == 0) {
            break;
        }

        close(newsockfd);

    } while (1);

    // Calculate average time and total average bandwidth
    double average_time = total_time / file_count;
    double total_average_bandwidth = (total_bytes_received / (1024.0 * 1024.0)) / total_time;

    // Print out results
    printf("Total average bandwidth: %.2f MB/s\n", total_average_bandwidth);
    printf("Average time taken to receive each file: %.2f seconds\n", average_time);

    // Close the socket
    close(sockfd);

    return 0;
}



