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
#include <vector>
#include <cstring>

// for phase
#define SLOW_START 0
#define CONGESTION_AVOID 1
#define FAST_RECOVERY 2
// 
#define INIT_SEQ_NUM 1
// for ack_flag
#define FLAG_ACK 0
#define FALG_DATA 1

typedef char BYTE;

// pkt max length = 1500-20(ip)-20(udp) = 1460
// now the pkt size is 988+10 bytes, 1KB
struct pkt{
    int seq_num;
    int ack_num;
    int ack_flag;
    BYTE payload[988];
};


class Sender
{
    public:
        Sender( std::string port, int advertisedWindow );
        virtual ~Sender();
        // api for the application layer
        void send( std::string filename );

    private:
        // for udp transmit
        struct addrinfo hints;
        struct addrinfo *receiverinfo;
        int senderFD;

        // for reliable transmit
        void init( int advertisedWindow ); // initialize those parameters
        int maxSeqNum; // next seq# to send
        int sendBase;
        int cwnd;
        int ssthresh;
        int dupACKcount;
        int recvwnd; // AdvertisedWindow
        int phase; 
        std::vector<pkt> pktlist; 

        // functions
        // onTimeout();
        // onThreeDupACK();
        // mySend();
        // myHandle(); // processing ACKs received from the receiver
        // calcCwnd();
        void pktlist_generator( std::string filename ); // generate segments from layer 5 data, put into pktlist
        void workphase_slowstart();
};

#endif // SENDER_H
