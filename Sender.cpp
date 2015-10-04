#include "Sender.h"


void Sender::init( int advertisedWindow ){
    sendBase = 1;
    cwnd = 1;
    maxSeqNum = sendBase + cwnd - 1;
    ssthresh = 64; // 64KB
    dupACKcount = 0;
    recvwnd = advertisedWindow;
    phase = SLOW_START;
    dupACK_flag = 0;
}


Sender::Sender( std::string port, int advertisedWindow ){
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    // set receiver info
    int status;
    std::string host = "127.0.0.1";
    if( (status = getaddrinfo(host.c_str(), port.c_str(), &hints, &receiverinfo)) != 0 ){
        std::cout << "sender getaddrinfo(): " << gai_strerror(status) << std::endl;
        exit(1);
    }

    // get fd
    if( (senderFD = socket(receiverinfo->ai_family, receiverinfo->ai_socktype, receiverinfo->ai_protocol)) == -1){
        perror("sender socket(): ");
        exit(1);
    }

    // use setsockopt to set timeout

    init(advertisedWindow);
}


void Sender::send( std::string filename ){
	pktlist_generator(filename);

    while( sendBase <= pktnum ){
        switch(phase){
            case SLOW_START:
                std::cout << "current phase: SLOW_START, cwnd: " << cwnd <<"\n";
                workphase_slowstart();
                std::cout << "\n";
                break;

            case CONGESTION_AVOID:
                std::cout << "current phase: CONGESTION_AVOID, cwnd: " << cwnd <<"\n";
                workphase_congestion_avoid();
                std::cout << "\n";
                break;

            case FAST_RECOVERY:
                std::cout << "current phase: FAST_RECOVERY, cwnd: " << cwnd <<"\n";
                workphase_fase_recovery();
                std::cout << "\n";
                break;

            default:
                break;
        }
    }
    std::cout << "\n\n@@@@ finish sending ! @@@@\n";
    close(senderFD);
}


void Sender::workphase_fase_recovery(){
    // after 3 dup ack
    if( dupACK_flag ){
        sendPkt(&pktlist[sendBase - 1]);
        dupACK_flag = 0;
        return;
    }
    // send all pkt in the cwnd
    for( int i = 0; i < cwnd; i++ ){
        sendPkt(&pktlist[(sendBase-1)+i]);
    }
    sleep(1);
    // should recv number of ack: current cwnd
    int current_cwnd = cwnd;
    for( int j = 0; j < current_cwnd; j++ ){
        int req_num = recvACK();

        if( req_num > sendBase ){
            // maxSeqNum += req_num - sendBase;
            sendBase = req_num;
            cwnd++;
        }else{
            dupACKcount++;
            if( dupACKcount == 3 ){
                ssthresh = cwnd/2;
                cwnd = ssthresh + 3;
                dupACKcount = 0;
                phase = FAST_RECOVERY;
                std::cout << "  ~~~~ sender recv 3 dup ack!! ~~~~\n";
                return;
            }
        }
    }
    // if reach ssthresh
    if( cwnd >= ssthresh ){
        phase = CONGESTION_AVOID;
    }
    sleep(1);
}


void Sender::workphase_congestion_avoid(){
    // send all pkt in the cwnd
    for( int i = 0; i < cwnd; i++ ){
        sendPkt(&pktlist[(sendBase-1)+i]);
    }
    sleep(1);
    // should recv number of ack: current cwnd
    int current_cwnd = cwnd;
    for( int j = 0; j < current_cwnd; j++ ){
        int req_num = recvACK();

        if( req_num > sendBase ){
            // maxSeqNum += req_num - sendBase;
            sendBase = req_num;
        }else{
            dupACKcount++;
            if( dupACKcount == 3 ){
                ssthresh = cwnd/2;
                cwnd = ssthresh + 3;
                dupACKcount = 0;
                phase = FAST_RECOVERY;
                dupACK_flag = 1;
                std::cout << "  ~~~~ sender recv 3 dup ack!! ~~~~\n";
                return;
            }
        }
    }
    cwnd++;
    sleep(1);
}


void Sender::workphase_slowstart(){
    // send all pkt in the cwnd
    for( int i = 0; i < cwnd; i++ ){
        sendPkt(&pktlist[(sendBase-1)+i]);
    }
    sleep(1);
    // should recv number of ack: current cwnd
    int current_cwnd = cwnd;
    for( int j = 0; j < current_cwnd; j++ ){
        int req_num = recvACK();

        if( req_num > sendBase ){
            // maxSeqNum += req_num - sendBase;
            sendBase = req_num;
            cwnd++;
        }else{
            dupACKcount++;
            if( dupACKcount == 3 ){
                ssthresh = cwnd/2;
                cwnd = ssthresh + 3;
                dupACKcount = 0;
                phase = FAST_RECOVERY;
                dupACK_flag = 1;
                std::cout << "  ~~~~ sender recv 3 dup ack!! ~~~~\n";
                return;
            }
        }
    }
    // if reach ssthresh
    if( cwnd >= ssthresh ){
        phase = CONGESTION_AVOID;
    }
    sleep(1);
}


void Sender::sendPkt(struct pkt * packet){
    char temp[PKT_SIZE];
    memcpy( temp, packet, PKT_SIZE );

    if( sendto(senderFD, temp, PKT_SIZE, 0, receiverinfo->ai_addr, receiverinfo->ai_addrlen) == -1){
        perror("sender sendto(): ");
        exit(1);
    }

    std::cout << "--send pkt-- " << "seq_num: " << packet->seq_num << "\n";
}


int Sender::recvACK(){
    BYTE temp[PKT_SIZE];

    if( (recvfrom( senderFD, temp, PKT_SIZE, 0, NULL, NULL )) == -1 ){
        perror("sender recvfrom(): ");
        exit(1);
    }

    struct pkt myPkt;
    memcpy( &myPkt, temp, PKT_SIZE );
    std::cout << "**recv ack** " << "ack_num: " << myPkt.ack_num << "\n";

    checkIfFin(myPkt.ack_num);

    return myPkt.ack_num;
}


void Sender::checkIfFin(int ack_num){
    if( ack_num > pktnum ){
        std::cout << "\n\n@@@@ finish sending !!!!!! @@@@\n";
        close(senderFD);
        exit(1);
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
        pktnum = file_size/988 + 1;
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
        std::cout << pktnum << " packets total.\n\n";
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








