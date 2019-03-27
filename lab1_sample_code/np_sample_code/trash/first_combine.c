#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while (0)

#define buffsize 60000

int udp_send(int sock);
int udp_recv(int sock);
int tcp_send();
int tcp_recv();

struct hostent *server;
struct sockaddr_in peeraddr;
struct sockaddr_in servaddr;
char ip[256]={0};
int port;
char file_name[256]={0};
int ret;

time_t rawtime;
struct tm * timeinfo;
char log_message[256];
int log_size;

FILE *fp, *fp2;
int file_size, part_size, remain_size;

int tcp_recv(){
    int sockfd, newsockfd, port;
    socklen_t clilen;
    char buffer[buffsize];
    int n;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        perror("ERROR opening socket");
    bzero((char *)&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *)&servaddr,
             sizeof(servaddr)) < 0)
        perror("ERROR on binding");
    listen(sockfd, 5);
    clilen = sizeof(peeraddr);
    newsockfd = accept(sockfd,
                       (struct sockaddr *)&peeraddr,
                       &clilen);
    if (newsockfd < 0)
        perror("ERROR on accept");

    FILE *fp;

    fp = fopen(file_name, "w");
    if( fp == NULL )
    {
        perror ("Error opening file");
        return(-1);
    }
    int i=0;
    while(1)
    {
        ++i;
        bzero(buffer, buffsize);
        n = read(newsockfd, buffer, buffsize-1);
        if (n < 0)
            perror("ERROR reading from socket");
        if(strcmp(buffer,"EOF")==0){
            puts("EOF");
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

    char buffer[buffsize];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        perror("ERROR opening socket");
    bzero((char *) &servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&servaddr.sin_addr.s_addr,
         server->h_length);
    servaddr.sin_port = htons(port);
    if (connect(sockfd,(struct sockaddr *) &servaddr,sizeof(servaddr)) < 0)
        perror("ERROR connecting");

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

    FILE *fp2;
    fp2 = fopen("logfile.txt", "w");
    if( fp == NULL )
    {
        perror ("Error opening file");
        return(-1);
    }

    int part_size = file_size / 20;
    int remain_size = file_size % 20;
    int log_size;
    if (n < 0)
         perror("ERROR writing to socket");

    fp = fopen(file_name, "r");
    if( fp == NULL )
    {
        printf("%s\n",file_name);
        perror ("Error opening file");
        return(-1);
    }
    bzero(buffer,buffsize);
    for(int i = 0; i < 19; i++)
    {
        fread(&buffer, part_size, 1, fp);
        n = write(sockfd,buffer,part_size);
        if (n < 0)
            perror("ERROR writing to socket");

        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        log_size = sprintf(log_message, "%d%% %d/%02d/%d %d:%d:%d\n", ( i + 1 ) * 5, timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        fwrite(log_message, log_size, 1, fp2);
    }
    fread(&buffer, part_size + remain_size, 1, fp);
    n = write(sockfd,buffer,part_size + remain_size);
    if (n < 0)
        perror("ERROR writing to socket");

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    log_size = sprintf(log_message, "100%% %d/%02d/%d %d:%d:%d\n", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    fwrite(log_message, log_size, 1, fp2);

    for(int i=0;i<10000;i++){
        for(int j=0;j<10000;j++);
    }
    bzero(buffer,buffsize);
    sprintf(buffer,"EOF");
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0)
         perror("ERROR writing to socket");
    fclose(fp);
    fclose(fp2);
    close(sockfd);
    return 0;
}

int main(int argc, char *argv[])
{

    if (argc < 6)
    {
        fprintf(stderr, "wrong input");
        exit(1);
    }
    strcpy(ip, argv[3]);
    port = atoi(argv[4]);
    strcpy(file_name, argv[5]);
    printf("%s %d %s\n",ip,port, file_name);
    if(strcmp(argv[1],"tcp")==0){
        if(strcmp(argv[2],"send")==0){
            server = gethostbyname(argv[3]);
            if (server == NULL) {
                fprintf(stderr,"ERROR, no such host\n");
                exit(0);
            }
            int x = tcp_send();
        }else{
            int x = tcp_recv();
        }
    }else if(strcmp(argv[1],"udp")==0){
        int sock;
        if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
            ERR_EXIT("socket perror");
        struct sockaddr_in servaddr;
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        if(strcmp(argv[2],"send")==0){
            servaddr.sin_addr.s_addr = inet_addr(ip);
            int x = udp_send(sock);
        }else{
            servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
            if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
                ERR_EXIT("bind perror");
            int x = udp_recv(sock);
        }
    }

    return 0;
}

int udp_send(int sock)
{
    char sendbuf[buffsize];
    memset(sendbuf, 0, sizeof(sendbuf));

    // get size of file
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

    // seperate into 20 times for sending.
    part_size = file_size / 20;
    remain_size = file_size % 20;

    // open read file pointer
    fp = fopen(file_name, "r");
    if( fp == NULL )
    {
        perror ("Error opening file");
        return(-1);
    }

    // open write file pointer for log message
    fp2 = fopen("logfile.txt", "w");
    if( fp == NULL )
    {
        perror ("Error opening file");
        return(-1);
    }

    // read, write, log
    for(int i = 0; i < 19; i++)
    {
        // read from file
        fread(&sendbuf, part_size, 1, fp);
        // write to socket
        sendto(sock, sendbuf, part_size, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
        // write log message
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        log_size = sprintf(log_message, "%d%% %d/%02d/%d %d:%d:%d\n", ( i + 1 ) * 5, timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        fwrite(log_message, log_size, 1, fp2);
        // clear buffer
        memset(sendbuf, 0, sizeof(sendbuf));
    }

    // the last time should include the remaining part
    fread(&sendbuf, part_size + remain_size, 1, fp);
    sendto(sock, sendbuf, part_size + remain_size, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    log_size = sprintf(log_message, "100%% %d/%02d/%d %d:%d:%d\n", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    fwrite(log_message, log_size, 1, fp2);
    memset(sendbuf, 0, sizeof(sendbuf));

    // wait for a while and send EOF
    for(int i=0;i<10000;i++){
        for(int j=0;j<10000;j++);
    }
    sprintf(sendbuf,"EOF");
    sendto(sock, sendbuf, sizeof(sendbuf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

    close(sock);
    fclose(fp);
    fclose(fp2);
}

int udp_recv(int sock)
{
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
        puts("ba");
        peerlen = sizeof(peeraddr);
        memset(recvbuf, 0, sizeof(recvbuf));
        n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0,
                     (struct sockaddr *)&peeraddr, &peerlen);
        if (n == -1)
        {
        puts("aa");
            if (errno == EINTR)
                continue;

            ERR_EXIT("recvfrom perror");
        }
        else if(n > 0)
        {
            if(strcmp(recvbuf,"EOF")==0){
                puts("receive EOF");
                break;
            }
            puts("bla");
            fputs(recvbuf, stdout);
            fwrite(recvbuf, n, 1, fp);
        }
    }
    fclose(fp);
    close(sock);
}