#include "Sender.h"


void Sender::init( int advertisedWindow ){
    sendBase = 0;
    cwnd = 1;
    maxSeqNum = sendBase + cwnd;
    ssthresh = 64; // 64KB
    dupACKcount = 0;
    recvwnd = advertisedWindow;
    phase = 0;
}


Sender::Sender( std::string port, int advertisedWindow ){
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    bzero(&receiver_addr,sizeof(receiver_addr));
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    receiver_addr.sin_port=htons(3480);

    // getaddrinfo
    int status;
    if( (status = getaddrinfo("0.0.0.0", port.c_str(), &hints, &senderinfo)) != 0 ){
        std::cout << "getaddrinfo() function error:" << gai_strerror(status) << std::endl;
        exit(1);
    }

    // get fd
    if( (senderFD = socket(senderinfo->ai_family, senderinfo->ai_socktype, senderinfo->ai_protocol)) == -1){
        perror("sender socket(): ");
        exit(1);
    }

    // use setsockopt to set timeout

    init(advertisedWindow);
}


void Sender::send( std::string filename ){
	pktlist_generator(filename);

    //while(1){
        switch(phase){
            case SLOW_START:
                workphase_slowstart();
                break;

            case CONGESTION_AVOID:

                break;

            case FAST_RECOVERY:
                break;

            default:
                break;
        }
    //}

    close(senderFD);
}


void Sender::workphase_slowstart(){
    // send all pkt in the cwnd
    while( sendBase <= maxSeqNum - 1 ){
        int size = sizeof(pktlist[sendBase]);
        std::cout << "sending out bytes: " << size << std::endl;
        char temp[size];
        memcpy(temp, &pktlist[sendBase], size );
        if( sendto(senderFD, temp, size, 0, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr)) == -1){
            perror("sender sendto(): ");
            exit(1);
        }
        sendBase++;
    }
}


void Sender::pktlist_generator(std::string filename){
	FILE * pFile;

    if( (pFile = fopen(filename.c_str() ,"r")) == NULL ){
        // not found
        perror("sender fopen(): ");
        exit(1);
    }else{
        // obtain file size:
        fseek (pFile , 0 , SEEK_END);
        long file_size = ftell(pFile);
        rewind (pFile);
        // calculate number of packet
        int pktnum = file_size/988 + 1;
        // read the data into pktlist
        BYTE temp[988];
        for( int i = 0; i < pktnum; i++ ){
            memset(temp, 0, 988);
        	fread(temp, 1, 988, pFile);

        	struct pkt mypacket;
        	mypacket.seq_num = i+1;
        	mypacket.ack_num = -1;
        	mypacket.ack_flag = FALG_DATA;
        	memcpy(mypacket.payload, temp, 988);

        	pktlist.push_back(mypacket);
        }
        // close file
        fclose(pFile);
    }

    // checker
    // for(unsigned int i = 0; i < pktlist.size(); i++){
    // 	std::cout << pktlist[i].seq_num << " " << pktlist[i].ack_num;
    // 	std::cout << " " << pktlist[i].ack_flag << std::endl;
    // 	std::cout << pktlist[i].payload << "\n\n";
    // }
}


Sender::~Sender(){
    //dtor
}








