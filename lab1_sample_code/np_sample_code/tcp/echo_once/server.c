/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int buffsize = 60000;
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[buffsize];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 3)
    {
        fprintf(stderr, "ERROR. usage: %s port file_name\n", argv[0]);
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,
                       (struct sockaddr *)&cli_addr,
                       &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");

    FILE *fp;

    fp = fopen(argv[2], "w");
    if( fp == NULL )
    {
        // printf("%s\n",argv[2]);
        perror ("Error opening file");
        return(-1);
    }
    int i=0;
    while(1)
    {
        ++i;
        bzero(buffer, buffsize);
        n = read(newsockfd, buffer, buffsize-1);
        // n=0;
        // printf("i=%d n=%d\n",i+1,n);
        if (n < 0)
            error("ERROR reading from socket");
        if(strcmp(buffer,"EOF")==0){
            puts("EOF");
            // bzero(buffer,buffsize);
            // sprintf(buffer,"OK,EOF");
            // n = write(sockfd,buffer,strlen(buffer));
            // if (n < 0)
            //     error("ERROR writing to socket");
            break;
        }
        // printf("Here is the message: %s\n", buffer);

        fwrite(buffer, n, 1, fp);

        // bzero(buffer,buffsize);
        // sprintf(buffer,"%d%%\n",i*5);
        // n = write(sockfd,buffer,strlen(buffer));
        // if (n < 0)
        //     error("ERROR writing to socket");
    }
    fclose(fp);
    // bzero(buffer, buffsize);
    // n = read(newsockfd, buffer, buffsize-1);
    // if (n < 0)
    //     error("ERROR reading from socket");
    // // printf("Here is the message: %s\n", buffer);
    // n = write(newsockfd, "I got your message", 18);
    // if (n < 0)
    //     error("ERROR writing to socket");
    close(newsockfd);
    close(sockfd);
    return 0;
}
