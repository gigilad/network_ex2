#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <time.h>

#define PORT 6000
#define BUFFER_SIZE 2048
#define FILE_SIZE 2097152 // == 2MB
#define TCP_RENO 1
#define TCP_CUBIC 10


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

int main(int args, char *argv[]){
    if(args !=7 ){
        printf("Eroor");
        exit(EXIT_FAILURE);

    }
    char *ip = argv[2];
    int port =atoi(argv[4]);
    char *algorithm =argv[6];

    char user_decision ='y';
    //genreate randim data;
    char *random_data = util_generate_random_data(FILE_SIZE);
    if(random_data == NULL){
        printf("COULD NOT GET DATA");
        return 1;
    }
    //Creating tcp socket
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock<0){
        perror("Eroor in creating socket");
        return 1;
    }

    if(setsockopt(sock,IPPROTO_TCP,TCP_CONGESTION , algorithm, sizeof(algorithm))<0){

        perror("Eroor when setting tcp congestion control algo");
        return 1;
    }

    //init the address of the receiver
    struct sockaddr_in serv_addr, client_addr;
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family =htons(port);
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);

    // connect to the reciebver:
    if(connect(sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
        perror("Eroor connecting");
        return 1;
    }

    do{
    char buffer[BUFFER_SIZE];
    size_t bytes_sent =0;
    while(bytes_sent <FILE_SIZE){
        size_t bytes_to_send = (FILE_SIZE-bytes_sent)<BUFFER_SIZE ? (FILE_SIZE-bytes_sent) :BUFFER_SIZE;
        if(send(sock,random_data+bytes_sent,bytes_to_send,0)!= bytes_to_send){
            perror("Eroor sending file data");
            return 1;
        }
        bytes_sent+=bytes_to_send;
    }
    printf("Do you want to send it again? (y/n): ");
    scanf(" %c", &user_decision);
    } while (user_decision =='y' || user_decision =='Y');


    if(send(sock,"Exit" , strlen("exit"),0) != strlen("exit")){
        perror("error sending exit message");
        return 1;
    }
    close(sock);
    free(random_data);


    return 0;

}