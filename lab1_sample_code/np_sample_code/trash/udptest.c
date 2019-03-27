#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

char ip[256]={0};
int port;
char file_name[256]={0};

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
                puts("receive EOF");
                break;
            }
            // fputs(recvbuf, fp);
            fwrite(recvbuf, n, 1, fp);
            // sendto(sock, recvbuf, n, 0,
            //        (struct sockaddr *)&peeraddr, peerlen);
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
    // printf("part size = %d\n",part_size);
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
        // printf("%d %s\n", i + 1, sendbuf);
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        // for(int j=0;j<10000;++j)
        //     for(int k=0;k<10000;++k);
        log_size = sprintf(log_message, "%d%% %d/%02d/%d %d:%d:%d\n", ( i + 1 ) * 5, timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        fwrite(log_message, log_size, 1, fp2);

        memset(sendbuf, 0, sizeof(sendbuf));
    }

    // read data from file
    fread(&sendbuf, part_size + remain_size, 1, fp);
    sendto(sock, sendbuf, part_size + remain_size, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    // printf("%d %s\n", 20, sendbuf);
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
    // printf("END %s\n", sendbuf);

    // while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
    // {

    //     sendto(sock, sendbuf, strlen(sendbuf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

    //     ret = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);
    //     if (ret == -1)
    //     {
    //         if (errno == EINTR){
    //             continue;
    //         }
    //         ERR_EXIT("recvfrom");
    //     }

    //     fputs(recvbuf, stdout);
    //     printf("GET:%s\n",recvbuf);
    //     memset(sendbuf, 0, sizeof(sendbuf));
    //     memset(recvbuf, 0, sizeof(recvbuf));
    // }

    close(sock);


}

int main(int argc, char *argv[])
{
    strcpy(ip, argv[3]);
    port = atoi(argv[4]);
    strcpy(file_name, argv[5]);
    if(strcmp(argv[1],"udp")==0){
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