#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include<time.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    time_t rawtime;
    struct tm * timeinfo;

    int buffsize = 60000;
    char buffer[buffsize];
    char log_message[256];
    if (argc < 4) {
       fprintf(stderr,"usage %s hostname port file_name\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    // get size of file
    FILE *fp;
    int file_size;

    fp = fopen(argv[3], "r");
    if( fp == NULL )
    {
        printf("%s\n",argv[3]);
        perror ("Error opening file");
        return(-1);
    }
    fseek(fp, 0, SEEK_END);

    file_size = ftell(fp);
    fclose(fp);

    FILE *fp2;
    fp2 = fopen("logfile.txt", "w");
    if( fp == NULL )
    {
        perror ("Error opening file");
        return(-1);
    }

    // printf("Total size of file.txt = %d bytes\n", file_size);

    int part_size = file_size / 20;
    int remain_size = file_size % 20;
    int log_size;
    if (n < 0)
         error("ERROR writing to socket");

    fp = fopen(argv[3], "r");
    if( fp == NULL )
    {
        printf("%s\n",argv[3]);
        perror ("Error opening file");
        return(-1);
    }
    bzero(buffer,buffsize);
    for(int i = 0; i < 19; i++)
    {
        fread(&buffer, part_size, 1, fp);
        n = write(sockfd,buffer,part_size);
        // printf("%d %d %s\n", i + 1, n, buffer);
        if (n < 0)
            error("ERROR writing to socket");

        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        log_size = sprintf(log_message, "%d%% %d/%02d/%d %d:%d:%d\n", ( i + 1 ) * 5, timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        fwrite(log_message, log_size, 1, fp2);

        // bzero(buffer,buffsize);
        // n = read(sockfd,buffer,buffsize-1);
        // if (n < 0)
        //     error("ERROR reading from socket");
        // printf("%s\n",buffer);
    }
    fread(&buffer, part_size + remain_size, 1, fp);
        // fwrite(buffer, part_size + remain_size, 1, fp2);
    n = write(sockfd,buffer,part_size + remain_size);
    if (n < 0)
        error("ERROR writing to socket");

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    log_size = sprintf(log_message, "100%% %d/%02d/%d %d:%d:%d\n", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    fwrite(log_message, log_size, 1, fp2);

    // bzero(buffer,buffsize);
    // n = read(sockfd,buffer,buffsize-1);
    // if (n < 0)
    //     error("ERROR reading from socket");
    // printf("%s\n",buffer);

    for(int i=0;i<10000;i++){
        for(int j=0;j<10000;j++);
    }
    bzero(buffer,buffsize);
    sprintf(buffer,"EOF");
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0)
         error("ERROR writing to socket");

    // bzero(buffer,buffsize);
    // n = read(sockfd,buffer,buffsize-1);
    // if (n < 0)
    //      error("ERROR reading from socket");
    // printf("%s\n",buffer);

    fclose(fp);
    fclose(fp2);

    // send message to server
    // printf("Please enter the message: ");
    // bzero(buffer,buffsize);
    // fgets(buffer,buffsize-1,stdin);
    // n = write(sockfd,buffer,strlen(buffer));
    // if (n < 0)
    //      error("ERROR writing to socket");

    // bzero(buffer,buffsize);
    // n = read(sockfd,buffer,buffsize-1);
    // if (n < 0)
    //      error("ERROR reading from socket");
    // printf("%s\n",buffer);
    close(sockfd);
    return 0;
}
