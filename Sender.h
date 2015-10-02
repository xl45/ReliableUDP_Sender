#ifndef SENDER_H
#define SENDER_H

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <stdio.h>
#include <sys/time.h>
#include <arpa/inet.h>

#define 0 SLOW_START
#define 1 CONGESTION_AVOID
#define 2 FAST_RECOVERY
#define 1 INIT_SEQ_NUM

typedef char BYTE;

// header length = 10 bytes
struct header{
    int seq_num;
    int ack_num;
    short ack_flag;
};

// pkt max length = 1500-20(ip)-20(udp) = 1460
// now the pkt size is 900+10 bytes
struct pkt{
    struct header;
    BYTE payload[900];
};


class Sender
{
    public:
        Sender( std::string port );
        void work( std::string filename );
        virtual ~Sender();

    private:
        // for udp transmit
        struct addrinfo hints;
        struct addrinfo *serverinfo;
        struct sockaddr_storage client_addr;
        socklen_t addr_len;
        int serverFD;

        // for reliable transmit
        int nextSeqNum;
        int sendBase;
        int cwnd;
        int ssthresh;
        int dupACKcount;
        int recvwnd;
        int state; // (0 - slow start, 1 - congestion avoidance, 2 - fast recovery )

        // functions
        onTimeout();
        onThreeDupACK();
        mySend();
        myHandle(); // processing ACKs received from the receiver
        calcCwnd();
};

#endif // SENDER_H
