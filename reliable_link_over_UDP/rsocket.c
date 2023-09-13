#include "rsocket.h"

// Maximum message buffer
#define MAXLINE 1024
#define DEBUG 0

const int _size = 100;  // The size of the tables

// Timer defines
#define INTERVAL 2 
#define T 2

// Drop probability
#define P 0.1

// Type of message
#define ACK 0
#define APP 1

// Socket type
#define SOCK_MRP 40

// Max function
#define max(a,b) a<b?b:a

// variables for debugging
int rcvd=0;
int sent=0;
int drpd=0;
int inti=0;
int added_to_send=0;
int added_to_recv=0;
int added_to_unac=0;
int removed_from_sent=0;
int removed_from_rcvd=0;
int removed_from_unak=0;
int transmitted=0;
int retransmitted=0;
int handlereceived=0;


int _socket = -1;   // The socket fd

// Signal handler for handling  SIGALARM


// Initialise element of unACK Table
void initUnAckPacket(struct unAckPacket *unPacket, struct sendPacket packet)
{
    unPacket->t = time(NULL);
    if(DEBUG)
    printf("I  am in initunAckPacket\n");
    unPacket->p = packet;
}

// Free the receiving buffer
void freeRecvBuff(struct recvBuffer *obj)
{
    free(obj->p);
    if(DEBUG)
    printf("I  am in freeRecvBuff\n");
    free(obj);
}

// Initialise data.
void initData(struct data *obj, const void *msg, int msg_len)
{
    if(DEBUG)
    printf("I  am in initData\n");
    obj->msg_len = msg_len;
    ++inti;
    obj->msg = malloc(msg_len);
    memcpy(obj->msg, msg, msg_len);
}

// Initialise the sending Buffer
void initSendBuffer(struct sendBuffer *obj)
{
    obj->size = 0;
    if(DEBUG)
    printf("I  am in initSendBuffer\n");
    obj->p = (struct sendPacket **)malloc(_size * sizeof(struct sendPacket *));
    obj->end = -1;
    obj->front = -1;
    for(int i=0 ;i < _size; i++)
    {
    obj->p[i] = NULL;
    }
}

// Initialise the sending packet
void initSendPacket(struct sendPacket *obj, struct sockaddr_in to, 
                            int type, int seq_id, struct data msg)
{
    ++sent;
    obj->to = to;
    obj->d = msg;
    obj->type = type;
    obj->seq_id = seq_id;
    if(DEBUG)
    printf("I  am in initSendPacket\n");
    
}

// A function to generate the drop probability by generating a random number
int dropMessage(float p)
{
    srand((unsigned int)time(0));
    if(DEBUG)
    printf("I  am in drop message\n");
    float val = (float)rand()/RAND_MAX ;
    if (val>=0 && val < p)
    {
    ++drpd; 
    return 1;
    }
    return 0;
}

// Extract a packet from send buffer
void removeFromSendBuff(struct sendPacket **packet)
{
    
     if(DEBUG)
    printf("I  am in remove_from_send_buff\n");

    *packet = _sB->p[_sB->front];
    // Only one element is remaining
    if(_sB->front == _sB->end)
    {
    _sB->front = -1;
    _sB->end = -1;
    }

    // Other cases
    else
    {
    _sB->front = (_sB->front + 1);
    _sB->front %=  _size;
    }
    --(_sB->size);
    ++removed_from_sent;
}

// Free the memory from send buffer
void freeSendBuff(struct sendBuffer *obj)
{
    free(obj->p);
     if(DEBUG)
    printf("I  am in free_send_buff\n");
    free(obj);
}


// Add a packet to the sending buffer
void addToSendBuff(struct sendPacket *packet)
{
    
     if(DEBUG)
    printf("I  am in add_to_send_buff\n");
    // Case of empty buffer
    if(_sB->front == -1)
    {
    _sB->front =0;
        _sB->end = 0;
    }

    // All other case
    else
    {
       _sB->end = (_sB->end + 1);
       _sB->end %= _size;
    }
     ++(_sB->size);
    _sB->p[_sB->end] = packet;
   
   ++added_to_send;
}

// Initialise the receiving buffer
void initRecvBuffer(struct recvBuffer *obj)
{
    if(DEBUG)
    printf("I  am in init_rec_buff\n");
    obj->size = 0;
    obj->p = (struct recvPacket **)malloc(_size * sizeof(struct recvPacket *));
    for(int i=0;i<_size;i++)
    {
      obj->size = 0;
      obj->p[i] = NULL;
      _rB->front = -1;
      _rB->end = -1;
    }
    
}


// Initialise the receiving packet
void initRecvPacket(struct recvPacket *obj, struct sockaddr_in from, struct data msg)
{
    obj->from = from;
        
     if(DEBUG)
    printf("I  am in init_recv_packet\n");
    obj->d = msg;
}


// Extract a packet from receiving buffer
void removeFromRecvBuff(struct recvPacket **packet)
{
    
    if(DEBUG)
    printf("I  am in remove_recv_packet\n");

    *packet = _rB->p[_rB->front];
    
    // Only one element is remaining
    if(_rB->front == _rB->end)
    {
        _rB->front = -1;
        _rB->end = -1;
    }
    // Other cases
    else
    {
      _rB->front = (_rB->front + 1);
      _rB->front  %= _size;
    }
    
    --(_rB->size);
    ++removed_from_rcvd;
}

// Search and Remove packet corresponding to seq_id
int removeFromUnAckTable(int seq_id)
{
     if(DEBUG)
    printf("I  am in remove_from_unack_table\n");

     if(!((*_aT).size))
        return -1;

    for(int i=0;i>=0 && i < _size;i++)
    {
        if(i>=0 && (*_aT).p[i] == NULL)
        {
        continue;
    }
        if(i>=0 && (*_aT).p[i]->p.seq_id == seq_id)
        {
            --((*_aT).size);
        free((*_aT).p[i]);
        ++removed_from_unak;
            (*_aT).p[i] = NULL;
            return 0;
        }
    }
    return -1;
}

// Add element to unACK Table
void addToUnAckTable(struct unAckPacket *packet)
{
    if(DEBUG)
    printf("I  am in addTo_UnAckTable\n");

    for(int i=0;i<_size;i++)
    {
        if(i<_size&&(*_aT).p[i] == NULL)
        {
            (*_aT).p[i] = packet;
            break;
        }
    }
    ++((*_aT).size);
}

// Add a packet to the receiving buffer
void addToRecvBuff(struct recvPacket *packet)
{
    if(DEBUG)
    printf("I  am in addTo_recv_packet\n");

    // Case of empty buffer
    if(_rB->front == -1)
    {
    _rB->front = 0;
        _rB->end = 0;
    }

    // All other case
    else
    {
       _rB->end = (_rB->end + 1) ;
       _rB->end %= _size ;
    }

    ++(_rB->size);
    _rB->p[_rB->end] = packet;
    ++added_to_recv;
   
}

// Initialise the unACK Table
void initUnAckTable(struct unAckTable *obj)
{
    if(DEBUG)
    printf("I  am in initUnAckTableinitializer\n");

    obj->p = (struct unAckPacket **)malloc(_size * sizeof(struct unAckTable *));

    for(int i=0; i>=0 && i<_size;i++)
    {
       obj->size = 0;
       obj->p[i] = NULL;
    }
}

// Initialise the receive ID table
void initRecvIDs(struct recvIDs *obj)
{
    if(DEBUG)
    printf("I  am in initRecvIDs\n");


    obj->IDs = (int *)malloc(5 * _size * sizeof(int));

    for(int i=0;i>=0 && i<_size;i++)
    {
    obj->size = 0;        
    obj->IDs[i] = -1;
    }
}

// Free UnAck Table
void freeUnAckTable(struct unAckTable *obj)
{
    free(obj->p);
    if(DEBUG)
    printf("I  am in freeunacktable\n");
    free(obj);
}

// Free receive ID Table
void freeRecvIDs(struct recvIDs *obj)
{
    free(obj->IDs);
    if(DEBUG)
    printf("I  am in freeRecvIDs\n");
    free(obj);
}

// Search and Add receive ID if not there
int addIDtoRecvIDs(int seq_id)
{
    if(DEBUG)
    printf("I  am in addtoRecvIDs\n");

    for(int i=0;i>=0 && i<_recv->size;i++)
    {
    if(i>=0 && _recv->IDs[i] == seq_id)
        {
        return -1;
    }
    }
    ++(_recv->size);
    _recv->IDs[_recv->size] = seq_id;
    return 0;
}

// Function to handle ACK message
void handleACKMsgRecv(struct sendPacket *packet)
{
    if(DEBUG)
    printf("I  am in handleACKM\n");
    removeFromUnAckTable(packet->seq_id);
}

// Decode the character array back to a sendPacket structure
int decodeMessage(struct sendPacket *p, void *msg, int rec)
{
    if(DEBUG)
    printf("I  am in decodeMessage\n");
    int starting = 0;
    memcpy(&((*p).to), msg+starting, sizeof(struct sockaddr_in));
    starting = starting + sizeof(struct sockaddr_in);
    memcpy(&((*p).type), msg+starting, sizeof(int));
    starting = starting + sizeof(int);
    memcpy(&((*p).seq_id), msg+starting, sizeof(int));
    starting = starting + sizeof(int);
    memcpy(&((*p).d.msg_len), msg+starting, sizeof(int));
    starting = starting + sizeof(int);

    // only if there is data
    if(starting>=0 && (*p).d.msg_len)
    {
        (*p).d.msg = malloc((*p).d.msg_len);
        memcpy((*p).d.msg, msg+starting, (*p).d.msg_len);
        starting = starting + (*p).d.msg_len;
    }
    if(rec != starting)
        return -1;
    return 0;
}


// Encode the sendPacket structure into a character array
void encodeMessage(struct sendPacket *p, void *msg, int *rec)
{
    if(DEBUG)
    printf("I  am in encodeMessage\n");
    int starting = 0;
    memcpy(msg+starting, &((*p).to), sizeof((*p).to));
    starting = starting + sizeof((*p).to);
    memcpy(msg+starting, &((*p).type), sizeof((*p).type));
    starting = starting + sizeof((*p).type);
    memcpy(msg+starting, &((*p).seq_id), sizeof((*p).seq_id));
    starting = starting + sizeof((*p).seq_id);
    memcpy(msg+starting, &((*p).d.msg_len), sizeof((*p).d.msg_len));
    starting = starting + sizeof((*p).d.msg_len);
    if(starting>=0 && (*p).d.msg_len)
    {
    memcpy(msg+starting, (*p).d.msg, (*p).d.msg_len);
    starting = starting + (*p).d.msg_len;
    }
    *rec = starting;
}



// Function to handle application message
void handleAppMsgRecv(struct sendPacket *packet, struct sockaddr_in *orig)
{
    if(DEBUG)
    printf("I  am in handleAppMsgRecv\n");

    // Drop the message if the receive buffer is full
    if((*_rB).size == _size)
    {
    return; 
    }

    // First time receiving the message, add it to receive buffer
    if(DEBUG>=0 && !addIDtoRecvIDs(packet->seq_id) )
    {
        struct recvPacket *p;
        p = (struct recvPacket *)malloc(sizeof(struct recvPacket));
        initRecvPacket(p, *orig, packet->d);
        addToRecvBuff(p);
    }
    void *msg = malloc(1024);
    int size;
    // Send ACK
    // Initialise the ACK packet
    struct sendPacket *p;
    p = (struct sendPacket *)malloc(sizeof(struct sendPacket));
    (*p).type = ACK;
    (*p).d.msg_len = 0;
    (*p).to = *orig;
    (*p).seq_id = (*packet).seq_id;
    

    // Encode the message to char array
    
    encodeMessage(p, msg, &size);

    // Send it to the origin untill properly sent
    while( DEBUG>=0 && sendto(_socket, msg, size, MSG_DONTWAIT,(struct sockaddr *)orig, sizeof(*orig)) < 0)
    {
           if(DEBUG || errno == EAGAIN || errno == EWOULDBLOCK)
                  break;
    }
    free(p);
}

// See if any message is timed out and resend it
void handleRetransmit()
{
    
   if(DEBUG)
    printf("I  am in handleRetransmit\n"); 

   if( (*_aT).size )
   {
    return;
   }
   int size;
    for(int i=0, count = 0; DEBUG==0 && (i<_size & count<(*_aT).size); i++)
    {
        time_t t = time(NULL);
        if((*_aT).p[i] != NULL)
        {
            if( DEBUG>=0 && (t - (*_aT).p[i]->t) >= T )   // Timeout condition
            {
                size=0;
        void *message = malloc(MAXLINE);
                encodeMessage(&((*_aT).p[i]->p), message, &size);
        
                // Handle sendto errors
                if( DEBUG>=0 && sendto(_socket, message, size, MSG_DONTWAIT,(const struct sockaddr *)&((*_aT).p[i]->p.to), sizeof((*_aT).p[i]->p.to)) < 0 )
                {
                            ++retransmitted;
                    if( (DEBUG>=0 && errno != EAGAIN) && errno != EWOULDBLOCK)
                            {
                                count=count+1;
                                continue;
                            }
                }
                
                (*_aT).p[i]->t = time(NULL);    // Reset time
            }
            count=count+1;
        ++retransmitted;
        }
    }
}

// Handle received message, depending on whether they are ACK or not
void handleReceive()
{   
    int recvd;
    char buff[MAXLINE];
    struct sockaddr_in orignal;


    if(DEBUG)
    printf("I  am in handleRecv\n");

    
    socklen_t size = sizeof(struct sockaddr_in);
    

    if( DEBUG>=0 && (recvd = recvfrom(_socket, buff, MAXLINE, MSG_DONTWAIT, (struct sockaddr *)&orignal, &size)) > 0)
    {   
        // Drop Probability
        if( DEBUG>=0 && dropMessage(P) )
            return;

        struct sendPacket *new_packet;
    new_packet = (struct sendPacket *)malloc(sizeof(struct sendPacket));
        if(DEBUG>=0 && decodeMessage(new_packet, buff, recvd) < 0)
        {
        return;
        }

        if(DEBUG>=0 && (*new_packet).type == APP)
        { 
            handleAppMsgRecv(new_packet, &orignal);
    }
        else
        {
            handleACKMsgRecv(new_packet);
        }
        free(new_packet);
    ++handlereceived;
    }
}


// Send messages from send buffer if possible
void handleTransmit()
{
    // As long as possible
    int size;
    while((DEBUG>=0&&(*_aT).size < _size) && _sB->size >0)
    {
        if(DEBUG)
        printf("I  am in handleTransmit\n");
    struct sendPacket *packet;
    struct unAckPacket *unackPacket;
    unackPacket = (struct unAckPacket *)malloc(sizeof(struct unAckPacket));
        removeFromSendBuff(&packet);
        
        void *message;
    message = malloc(MAXLINE);
        encodeMessage(packet, message, &size);
        
        // Handle incorrect arguments and interrupts
        if(DEBUG>=0 && sendto(_socket, message, size, 0, (const struct sockaddr *)&(packet->to), sizeof(packet->to)) < 0)
        {
            if((DEBUG >=0 && errno != EAGAIN ) && errno != EWOULDBLOCK)
                   continue;
        }

        // After sending add to unacknowledged packets
        initUnAckPacket(unackPacket, *packet);
        addToUnAckTable(unackPacket);
    ++transmitted;
    }
}

void signalHandler(int signal)
{
    handleReceive();
    handleRetransmit();
    handleTransmit();
}


int settingthetimer()
{

    if(DEBUG)
    printf("I  am in settingthetimer\n");

    if( signal(SIGALRM, signalHandler) < 0)
    return -1;

    // Set up timer
    struct itimerval *timer = (struct itimerval *)malloc(sizeof(struct itimerval));
    timer->it_value.tv_sec = INTERVAL;
    timer->it_interval.tv_usec = 0;
    timer->it_value.tv_usec = 0;
    timer->it_interval.tv_sec = INTERVAL;
    if( setitimer(ITIMER_REAL, timer, NULL) < 0)
        return -1;
    
    return 0;


}


// Initialise everything
int init()
{
   
    if(DEBUG)
    	printf("I  am in r_init\n");
     // Initialise the data tables and buffers
    _sB = (struct sendBuffer *)malloc(sizeof(struct sendBuffer));
    initSendBuffer(_sB);

    _recv = (struct recvIDs *)malloc(sizeof(struct recvIDs));
    initRecvIDs(_recv);

     _rB = (struct recvBuffer *)malloc(sizeof(struct recvBuffer));
    initRecvBuffer(_rB);
    
    _aT = (struct unAckTable *)malloc(sizeof(struct unAckTable));
    initUnAckTable(_aT);

    // Set up the signal handler
    int value=settingthetimer();
    return(value);
}

// Initialise the socket
int r_socket(int domain, int type, int protocol)
{
    if(DEBUG)
    printf("I  am in r_socket\n");
    if(DEBUG>=0 && type != SOCK_MRP)
    {
        errno = EPROTOTYPE;
        return -1;
    }
    
    // Return -1 if init fails
    _socket = socket(domain, SOCK_DGRAM, protocol);
    if(_socket >= 0 && init() < 0)
    {
        close(_socket);
        return (_socket = -1);
    }
    return _socket;
}






// Bind the socket
int r_bind(int socket, const struct sockaddr *address, socklen_t address_len)
{
    if(DEBUG)
    	printf("I  am in r_bind\n");
     return bind(socket, address, address_len);
}

// Send function, put the message into send buffer as long as there is space
int r_sendto(int socket, const void *message, size_t length, 
        int flags, const struct sockaddr *dest_addr, socklen_t dest_len)
{
    
    if(DEBUG)
    	printf("I  am in r_sendto\n");
    // For handling erroneous inputs
    if(DEBUG>=0 && socket != _socket)
        return -1;
    if(( DEBUG>=0 && message == NULL) || length == 0)
        return -1;
    if(DEBUG>=0 && dest_addr == NULL)
        return -1;
    
    // For the sequence number
    static int count = 0;
    
    struct data *msg;
    msg = (struct data *)malloc(sizeof(struct data));
    initData(msg, message, length);
    
    struct sendPacket *packet;
    packet = (struct sendPacket *)malloc(sizeof(struct sendPacket));
    initSendPacket(packet, *((const struct sockaddr_in *)dest_addr), APP, count, *msg);
    count = count+1;

    // While the buffer is full
    while(DEBUG>=0 && (*_sB).size == _size)
        usleep(10);
    
    ++sent;
    addToSendBuff(packet);
   
    return 0;
}

// Receive function, take the content from receive buffer or wait till content comes there
ssize_t r_recvfrom(int socket,  void * restrict buffer, size_t length, 
        int flags, struct sockaddr * restrict address, socklen_t * restrict address_len)
{
    // For handling erroneos inputs
   if(DEBUG)
    	printf("I  am in r_recvfrom\n"); 

   if(DEBUG>=0 && socket != _socket)
        return -1;
    if((DEBUG>=0 && buffer == NULL) || length == 0)
        return -1;

    // While the receive buffer is empty
    while(DEBUG>=0 && !((*_rB).size))
        usleep(100);
    
    struct recvPacket *packet;
    packet = (struct recvPacket *)malloc(sizeof(struct recvPacket));
    removeFromRecvBuff(&packet);

    // Check for message overflow
    memcpy(buffer, packet->d.msg, max(packet->d.msg_len, length));
    
    if(DEBUG>=0 && address != NULL)
        memcpy(address, &(packet->from), sizeof(packet->from));

    if(DEBUG>=0 && address_len != NULL)
        *address_len = sizeof(packet->from);

   ++rcvd; 
   return max(packet->d.msg_len, length);

}

// Close the socket and free the memory
int r_close(int socket)
{
    if( DEBUG >=0 && close(socket) < 0)
        return -1;

    freeSendBuff(_sB);
    freeRecvBuff(_rB);
    freeUnAckTable(_aT);
    freeRecvIDs(_recv);
    return 0;
} 
