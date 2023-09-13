#ifndef RSOCKET_H
#define RSOCKET_H
#include <stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>

#include<signal.h>
#include<errno.h>
#include<time.h>
#include<sys/time.h>
#include<arpa/inet.h>

// Socket type
#define SOCK_MRP 40

#define sz(a)  int((a).size())
#define pb push_back
#define all(c) (c).begin(),(c).end()
#define tr(c,i) for(typeof((c).begin() i = (c).begin(); i != (c).end(); i++)
#define present(c,x) ((c).find(x) != (c).end())
#define cpresent(c,x) (find(all(c),x) != (c).end())
#define forn(i,n) for(int i=0;i<n;++i)
#define foru(i,n) for(int i=1;i<=n;++i)
#define fortr(i,v) for(auto &i:v)
#define set(n) __builtin_popcount(n)
#define ll long long


// Sending Buffer
typedef struct sendBuffer{
    int front, end, size;
    struct sendPacket **p;
}send_Buffer;

// Receiving buffer
typedef struct recvBuffer{
    int front, end, size;
    struct recvPacket **p;
}recv_Buffer;

// unACKTable
typedef struct unAckTable{
    int size;
    struct unAckPacket **p;
}unAck_Table;

// Received IDs structure
typedef struct recvIDs{
    int size;
    int *IDs;
}recv_ID;

// Structure for storing message
typedef struct data{
    int msg_len;
    void *msg;
}data_msg;

// Structure for storing sending packet
typedef struct sendPacket{
    struct sockaddr_in to;
    int type, seq_id;
    struct data d;
}send_Packet;

// Structure for storing received packets
typedef struct recvPacket{
    struct sockaddr_in from;
    struct data d;
}recv_Packet;

// Element of unACK Table
typedef struct unAckPacket{
    time_t t;
    struct sendPacket p;
}un_ACKPacket;


struct sendBuffer *_sB;   // Buffer of data packets to be sent
struct recvIDs *_recv;   // A table of packets whose packets have been received
struct recvBuffer *_rB;   // Buffer of unique packets received
struct unAckTable *_aT;   // A table of unACKed packets


// Functions available to user

int dropMessage(float p);


int r_sendto(int socket, const void *message, size_t length, 
        int flags, const struct sockaddr *dest_addr, socklen_t dest_len);

int r_bind(int socket, const struct sockaddr *address, socklen_t address_len);


int r_close(int socket);

ssize_t r_recvfrom(int socket,  void * restrict buffer, size_t length, 
        int flags, struct sockaddr * restrict address, socklen_t * restrict address_len);

int r_socket(int domain, int type, int protocol);



#endif
