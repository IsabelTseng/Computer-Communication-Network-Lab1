#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

char ip[256]={0};
int port;
char file_name[256]={0};

int tcp_recv(){
    int buffsize = 60000;
    int sockfd, newsockfd;
    socklen_t clilen;
    char buffer[buffsize];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    // socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
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

    // open file
    FILE *fp;

    fp = fopen(file_name, "w");
    if( fp == NULL )
    {
        perror ("Error opening file");
        return(-1);
    }
    int i=0;

    // read data and write file
    while(1)
    {
        ++i;
        bzero(buffer, buffsize);
        n = read(newsockfd, buffer, buffsize-1);
        if (n < 0)
            error("ERROR reading from socket");
        if(strcmp(buffer,"EOF")==0){
            // puts("complete");
            break;
        }

        fwrite(buffer, n, 1, fp);
    }
    fclose(fp);
    close(newsockfd);
    close(sockfd);
    return 0;
}

int tcp_send(){
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    time_t rawtime;
    struct tm * timeinfo;

    int buffsize = 60000;
    char buffer[buffsize];
    char log_message[256];

    // socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(ip);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(port);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    // get size of file
    FILE *fp;
    int file_size;

    fp = fopen(file_name, "r");
    if( fp == NULL )
    {
        printf("%s\n",file_name);
        perror ("Error opening file");
        return(-1);
    }
    fseek(fp, 0, SEEK_END);

    file_size = ftell(fp);
    fclose(fp);

    // open log file
    FILE *fp2;
    fp2 = fopen("logfile.txt", "w");
    if( fp == NULL )
    {
        perror ("Error opening file");
        return(-1);
    }

    // count segment size
    int part_size = file_size / 20;
    int remain_size = file_size % 20;
    int log_size;

    // read file
    fp = fopen(file_name, "r");
    if( fp == NULL )
    {
        printf("%s\n",file_name);
        perror ("Error opening file");
        return(-1);
    }

    // send data
    bzero(buffer,buffsize);
    for(int i = 0; i < 19; i++)
    {
        fread(&buffer, part_size, 1, fp);
        n = write(sockfd,buffer,part_size);
        if (n < 0)
            error("ERROR writing to socket");

        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        // write log
        log_size = sprintf(log_message, "%d%% %d/%02d/%d %d:%d:%d\n", ( i + 1 ) * 5, timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        fwrite(log_message, log_size, 1, fp2);
    }
    fread(&buffer, part_size + remain_size, 1, fp);
    n = write(sockfd,buffer,part_size + remain_size);
    if (n < 0)
        error("ERROR writing to socket");

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    log_size = sprintf(log_message, "100%% %d/%02d/%d %d:%d:%d\n", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    fwrite(log_message, log_size, 1, fp2);

    // wait and send EOF
    for(int i=0;i<10000;i++){
        for(int j=0;j<10000;j++);
    }
    bzero(buffer,buffsize);
    sprintf(buffer,"EOF");
    puts("complete");
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0)
         error("ERROR writing to socket");

    fclose(fp);
    fclose(fp2);
    close(sockfd);
    return 0;
}

int udp_recv(int sock)
{
    int buffsize = 60000;
    char recvbuf[buffsize];
    struct sockaddr_in peeraddr;
    socklen_t peerlen;
    int n;

    FILE *fp;

    fp = fopen(file_name, "w");
    if( fp == NULL )
    {
        printf("%s\n",file_name);
        perror ("Error opening file");
        return(-1);
    }

    while (1)
    {
        // recv data
        peerlen = sizeof(peeraddr);
        memset(recvbuf, 0, sizeof(recvbuf));
        n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0,
                     (struct sockaddr *)&peeraddr, &peerlen);
        if (n == -1)
        {

            if (errno == EINTR)
                continue;

            ERR_EXIT("recvfrom error");
        }
        else if(n > 0)
        {
            if(strcmp(recvbuf,"EOF")==0){
                // puts("complete");
                break;
            }
            // write file
            fwrite(recvbuf, n, 1, fp);
        }
    }
    fclose(fp);
    close(sock);
}

int udp_send(int sock)
{
    int buffsize = 60000;
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5188);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret;
    char sendbuf[buffsize];
    // char recvbuf[buffsize];

    time_t rawtime;
    struct tm * timeinfo;
    char log_message[256];
    int log_size;

    // get size of file
    FILE *fp;
    int file_size;

    fp = fopen(file_name, "r");
    if( fp == NULL )
    {
        printf("%s\n",file_name);
        perror ("Error opening file");
        return(-1);
    }
    fseek(fp, 0, SEEK_END);

    file_size = ftell(fp);
    fclose(fp);

    int part_size = file_size / 20;
    int remain_size = file_size % 20;

    // send data
    fp = fopen(file_name, "r");
    if( fp == NULL )
    {
        printf("%s\n",file_name);
        perror ("Error opening file");
        return(-1);
    }

    FILE *fp2;
    fp2 = fopen("logfile.txt", "w");
    if( fp == NULL )
    {
        perror ("Error opening file");
        return(-1);
    }

    for(int i = 0; i < 19; i++)
    {
        // read data from file
        fread(&sendbuf, part_size, 1, fp);
        sendto(sock, sendbuf, part_size, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        log_size = sprintf(log_message, "%d%% %d/%02d/%d %d:%d:%d\n", ( i + 1 ) * 5, timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        fwrite(log_message, log_size, 1, fp2);

        memset(sendbuf, 0, sizeof(sendbuf));
    }

    // read data and write file
    fread(&sendbuf, part_size + remain_size, 1, fp);
    sendto(sock, sendbuf, part_size + remain_size, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    log_size = sprintf(log_message, "100%% %d/%02d/%d %d:%d:%d\n", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    fwrite(log_message, log_size, 1, fp2);

    memset(sendbuf, 0, sizeof(sendbuf));

    for(int i=0;i<10000;i++){
        for(int j=0;j<10000;j++);
    }
    sprintf(sendbuf,"EOF");
    sendto(sock, sendbuf, sizeof(sendbuf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

    puts("complete");
    close(sock);
    fclose(fp);
    fclose(fp2);

}

int main(int argc, char *argv[])
{
    // ip, port, file_name
    strcpy(ip, argv[3]);
    port = atoi(argv[4]);
    strcpy(file_name, argv[5]);

    //tcp or udp ? send or recv ?
    if(strcmp(argv[1],"tcp")==0){
        if(strcmp(argv[2],"send")==0){
            int x = tcp_send();
        }else{
            int x = tcp_recv();
        }
    }else if(strcmp(argv[1],"udp")==0){
        if(strcmp(argv[2],"send")==0){
            int sock;
            if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
                ERR_EXIT("socket");

            int x = udp_send(sock);
        }else{
            int sock;
            if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
                ERR_EXIT("socket error");

            struct sockaddr_in servaddr;
            memset(&servaddr, 0, sizeof(servaddr));
            servaddr.sin_family = AF_INET;
            servaddr.sin_port = htons(5188);
            servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

            if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
                ERR_EXIT("bind error");
            int x = udp_recv(sock);
        }
    }
    return 0;
}
