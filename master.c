
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "dnp3.h"

int main(int argc, char const *argv[])
{
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    struct dnp3_message_request dnp3_message;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(DNP3_SERVER_PORT);
    // Convert IPv4 from text to binary
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        perror("invalid address");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection failure");
        exit(EXIT_FAILURE);
    }

    memset(&dnp3_message, 0, sizeof(struct dnp3_message_request));
    // based on 0x05 0x64 0x0b 0xc4 0x46 0x00 0x40 0x00 0xa3 0xfe 0xd0 0xcd 0x01 0x3c 0x02 0x06 0xc2 0x62
    dnp3_message.ll.magic = htons(0x0564);
    dnp3_message.ll.len = 0x0b; // 11 bytes
    dnp3_message.ll.ctrl = 0xc4; // Control: DIR, PRM, Unconfirmed User Data
    dnp3_message.ll.dst = htons(0x4600); // destination
    dnp3_message.ll.src = htons(0x4000); // source
    dnp3_message.ll.crc = htons(0xefef); // wrong CRC just for testing now
    dnp3_message.tl = (DNP3_TL_FIN_BIT | DNP3_TL_FIR_BIT | (DNP3_TL_SEQ_BITS & 0)); // set FIN / FIR and SEQ BITS
    // TODO append application layer, calculate CRC

    send(sock , &dnp3_message , sizeof(struct dnp3_message_request) , 0 );
    printf("DNP3 request sent\n");
    valread = read( sock , buffer, 1024);
    printf("%s\n",buffer );
    return 0;
}