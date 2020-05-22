
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "dnp3.h"

int main(int argc, char const *argv[])
{
    int sock = 0, bytesread;
    struct sockaddr_in serv_addr;
    struct dnp3_message_read_request dnp3_message;
    char rcvbuffer[1024] = {0};
    unsigned short ll_crc;
    unsigned short dnp3_master_address=0x40;
    unsigned short dnp3_outstation_address=0x46;
    if( argc == 3 ) {
      dnp3_master_address = atoi(argv[1]);
      dnp3_outstation_address = atoi(argv[2]);
    }
    printf("Local master address is %hu\n", dnp3_master_address);
    printf("Outstation address is %hu\n", dnp3_outstation_address);

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

    memset(&dnp3_message, 0, sizeof(struct dnp3_message_read_request));
    // based on 0x05 0x64 0x0b 0xc4 0x46 0x00 0x40 0x00 0xa3 0xfe 0xd0 0xcd 0x01 0x3c 0x02 0x06 0xc2 0x62
    dnp3_message.ll.magic = htons(0x0564);
    dnp3_message.ll.len = 0x0b; // 11 bytes
    dnp3_message.ll.ctrl = 0xc4; // Control: DIR, PRM, Unconfirmed User Data
    dnp3_message.ll.dst = htons(dnp3_outstation_address); // destination
    dnp3_message.ll.src = htons(dnp3_master_address); // source
    generate_crc((unsigned char*)&dnp3_message, 8, &ll_crc);
    dnp3_message.ll.crc = ll_crc; // just copy CRC from sample packet
    dnp3_message.tl = (DNP3_TL_FIN_BIT | DNP3_TL_FIR_BIT | (DNP3_TL_SEQ_BITS & 0)); // set FIN / FIR and SEQ BITS
    dnp3_message.al_header.ctrl = (DNP3_AL_FIN_BIT | DNP3_AL_FIR_BIT | 0xd);
    dnp3_message.al_header.fc = DNP3_AL_FC_READ;
    dnp3_message.al_obj.type_group = 0x3c;
    dnp3_message.al_obj.type_variation = 0x01;
    dnp3_message.al_obj.qualifier = 0x06;
    dnp3_message.app_crc = htons(0xa8aa); // just copy CRC from sample packet

    send(sock , &dnp3_message , sizeof(struct dnp3_message_read_request) , 0 );
    printf("DNP3 request sent\n");
    bytesread = read( sock , rcvbuffer, 1024);
    printf("Received %d bytes\n", bytesread);

    close(sock);

    return 0;
}