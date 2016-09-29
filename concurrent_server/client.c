// check the defines later, may have some errors
#define MAX_SIZE 1024
#define MESSAGE_LENGTH 100
#define MAX_DIGITS_OF_MESSAGE_LEN numPlaces(MESSAGE_LENGTH)

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

struct message
{
    int message_type, message_length;
    char Message[MESSAGE_LENGTH];
};

int numPlaces (int n) {							////Number of digits in message length
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
    prefixbuffer[0] =  msg.message_type + '0';		///////////DOubt
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
    printf("%s lello\n",buffer );
    return buffer;
}

int main(int argc, char *argv[])
{
    int sockfd = 0, n = 0, num = 0;
    char recvBuff[MAX_SIZE], *sendBuff;
    struct sockaddr_in serv_addr; 

    if(argc != 3)
    {
        printf("\n Usage: %s <Server IP Address> <Server Port number>\n",argv[0]);
        return 1;
    }
    printf("%s\n", argv[2]);
    int port_num = atoi(argv[2]);		//assign port number
    printf("connecting to port number %d \n", port_num);

    memset(recvBuff, '0',sizeof(recvBuff));		
    // memset(sendBuff, '0',sizeof(sendBuff));

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_num); 

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    } 

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    } 

    // send the message to reqest UDP port here
    char m[MESSAGE_LENGTH] = "provide udp port";
    struct message msg;
    msg.message_type = 1;
    msg.message_length = strlen(m);
    strncpy(msg.Message, m, msg.message_length+1);
    sendBuff = message_to_buffer(msg);

    printf("Client:Message being sent: %s\n",sendBuff);
    if ((send(sockfd,sendBuff, strlen(sendBuff),0))== -1) 
    {
        fprintf(stderr, "Failure Sending Message\n");
        close(sockfd);
        exit(1);
    }
    free(sendBuff);
    // sending done

    // read the messages received from the server

    if ((num = recv(sockfd, recvBuff, MAX_SIZE,0))== -1) {
        //fprintf(stderr,"Error in receiving message!!\n");
        perror("recv");
        exit(1);
    }   
    else if (num == 0) {
        printf("Connection closed\n");
        return 0;
    }
    printf("received response \n");
    recvBuff[num] = '\0';
    printf("Message received: %s\n", recvBuff);
    // reading done
    // extract the UDP port number from the packet
    int index=num-1;
    while(recvBuff[index] != '=')
    {
        index--;
    }
    char sub_port[num-index+1];
    int i;
    for(i=0;i<num-index;i++)
    {
        sub_port[i] = recvBuff[i+index+1];
    }
    // printf("sub_port = %s\n", sub_port);
    int udp_port = atoi(sub_port);
    printf("udp port = %d \n", udp_port);
    // done extracting

    // printf("closing TCP connection \n");
    close(sockfd);
    sleep(1);				//////////////////////////doubt
    printf("TCP connection closed \n");
    

    int sock,bytes_recv,sin_size;
    struct sockaddr_in server_addr;
    struct hostent *host;
    char recv_data[1024];

    host= (struct hostent *) gethostbyname(argv[1]);


    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
    perror("socket");
    exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(udp_port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(server_addr.sin_zero),8);
    sin_size = sizeof(struct sockaddr);

    char mess_to_send[100];
    while(1)
    {
    printf("Message to send\n");
    gets(mess_to_send);
    char m2[MESSAGE_LENGTH];
    strcpy(m2,mess_to_send);
    struct message msg2;
    msg2.message_type = 3;
    msg2.message_length = strlen(m2);
    strncpy(msg2.Message, m2, msg2.message_length+1);
    sendBuff= message_to_buffer(msg2);
    printf("Client:Message being sent UDP: %s\n",sendBuff);

    sendto(sock, sendBuff, strlen(sendBuff), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    bytes_recv = recvfrom(sock,recv_data,MAX_SIZE,0,(struct sockaddr *)&server_addr,&sin_size);
    recv_data[bytes_recv]= '\0';
    printf("Received :%s\n",recv_data);
    free(sendBuff);
	}
    return 0;
}