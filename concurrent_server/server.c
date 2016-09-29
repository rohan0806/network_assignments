// changed vuffer sizes from 1025 to 1024
// check the defines later, may have some errors
#define MAX_SIZE 1024
#define MESSAGE_LENGTH 100
#define MAX_DIGITS_OF_MESSAGE_LEN numPlaces(MESSAGE_LENGTH)
#define LISTENQ 8 /*maximum number of client connections*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <assert.h>

struct message
{
    int message_type, message_length;
    char Message[MESSAGE_LENGTH];
};

int numPlaces (int n) {
    int r = 1;
    if (n < 0) n = (n == 1) ? 4 : -n;
    while (n > 9) {
        n /= 10;
        r++;
    }
    return r;
}

char* message_to_buffer(struct message msg)
{
    char* prefixbuffer = (char*) malloc(5);
    assert(msg.message_type >= 1 && msg.message_type <= 4);
    prefixbuffer[0] =  msg.message_type+ '0';
    int digits_msglen = numPlaces(msg.message_length);
    assert(digits_msglen >= 1 && digits_msglen <= MAX_DIGITS_OF_MESSAGE_LEN);
    int j=0;
    for(j=0; j<MAX_DIGITS_OF_MESSAGE_LEN; j++)
    {
        prefixbuffer[j+1] = '#';
    }
    int i = digits_msglen;
    while(msg.message_length)
    {
        char digit = (msg.message_length % 10) + '0';
        prefixbuffer[i] = digit;
        i--;
        msg.message_length /= 10; 
    }
    char* buffer = malloc(MESSAGE_LENGTH);
    strcpy(buffer, prefixbuffer);
    strcat(buffer, msg.Message);
    return buffer;
}

char* int_to_str(int x)
{   
    int tmp=x;
    int i=0;
    while(tmp)
    {
        tmp=tmp/10;
        i++;
    }
    char *buff=malloc(i*sizeof(char)+1);
    int len=i;
    buff[i]='\0';
    while(i)
    {
        buff[i-1]=x%10 + '0';
        i--;
        x=x/10;
    }
    return buff;
}

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("check your arguments \n");
        return 0;
    } 
    pid_t childpid;
    socklen_t clilen;
    long serv_port = atoi(argv[1]);
    printf("listening at port number = %ld\n", serv_port);
    int listenfd = 0, connfd = 0, num = 0;
    struct sockaddr_in cliaddr,serv_addr; 

    char recvBuff[MAX_SIZE], *sendBuff;
    time_t ticks; 

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    // memset(sendBuff, '0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(serv_port); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, LISTENQ); 
    printf("server running, waiting for connections \n");

    while(1){
    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen); 
    printf("received request \n");
    // now ready to receive type-1 packet
        static int i=1;
    i++;

  if ( (childpid = fork ()) == 0 ){

    close (listenfd);

    if ((num = recv(connfd, recvBuff, MAX_SIZE,0))== -1) {
        //fprintf(stderr,"Error in receiving message!!\n");
        perror("recv");
        exit(1);
    }   
    else if (num == 0) {
        printf("Connection closed\n");
        return 0;
    }
    recvBuff[num] = '\0';
    printf("Message received: %s\n", recvBuff);
    // type-1 packet received

    // now sending type-2 packet
    printf("Now providing a UDP port to client \n");
    

    int udp_port_to_send=1234+i;
    char c_udp_port_to_send[MESSAGE_LENGTH];
    strcpy(c_udp_port_to_send,int_to_str(udp_port_to_send));
    printf("\n%s\n",c_udp_port_to_send );
    char final_str[]="udp_port=\0";
    strcat(final_str,c_udp_port_to_send);
    
    char m[MESSAGE_LENGTH];
    strcpy(m,final_str);
    printf("final string %s\n",m );

    struct message msg;
    msg.message_type = 2;
    msg.message_length = strlen(m);
    strncpy(msg.Message, m, msg.message_length+1);

    sendBuff = message_to_buffer(msg);

    printf("Server:Message being sent: %s\n",sendBuff);
    if ((send(connfd,sendBuff, strlen(sendBuff),0))== -1) 
    {
        fprintf(stderr, "Failure Sending Message\n");
        close(connfd);
        exit(1);
    }
    free(sendBuff);

    // gracefully closing the connection
    printf("closing TCP connection \n");
    close(connfd);
    sleep(1);
    printf("TCP connection closed \n");

    int sock;
    int addr_len, bytes_read;
    char recv_data[MAX_SIZE],send_data[MAX_SIZE];
    struct sockaddr_in server_addr , client_addr;


    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(udp_port_to_send);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero),8);


    if (bind(sock,(struct sockaddr *)&server_addr,
        sizeof(struct sockaddr)) == -1)
    {
        perror("Bind");
        exit(1);
    }
    int itr=5;
    while(1)
    {
    addr_len = sizeof(struct sockaddr);

    fflush(stdout);
    bytes_read = recvfrom(sock,recv_data,1024,0, (struct sockaddr *)&client_addr, &addr_len);
    recv_data[bytes_read] = '\0';
    printf("message received = %s \n", recv_data);

    char m2[MESSAGE_LENGTH] = "final message";
    struct message msg2;
    msg2.message_type = 4;
    msg2.message_length = strlen(m);
    strncpy(msg2.Message, m2, msg2.message_length+1);
    sendBuff= message_to_buffer(msg2);

    sendto(sock,sendBuff,strlen(sendBuff),0,(struct sockaddr *)&client_addr,sizeof(struct sockaddr));
    fflush(stdout);
    }}
	}
    return 0;
}
