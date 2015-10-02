#include "Sender.h"

Sender::Sender( std::string port ){
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    // getaddrinfo
    int status;
    if( (status = getaddrinfo("0.0.0.0", port.c_str(), &hints, &serverinfo)) != 0 ){
        std::cout << "getaddrinfo() function error:" << gai_strerror(status) << std::endl;
        exit(1);
    }

    // get fd
    if( (serverFD = socket(serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol)) == -1){
        perror("server socket(): ");
        exit(1);
    }

    // bind to port
    if( bind(serverFD, serverinfo->ai_addr, serverinfo->ai_addrlen) == -1 ){
        perror("server bind(): ");
        exit(1);
    }
}


void Sender::work( std::string filename ){

}


Sender::~Sender(){
    //dtor
}











// pseudo code

// ----------------------------------------- receive thread, myHandle()
lastByteSent = initSeqNum - 1;
lastByteAcked = initSeqNum - 1;

while(there is data to send)

    read header event

    event: ack

