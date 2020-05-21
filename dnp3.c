#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "dnp3.h"

#ifndef __DEBUG__
#define __DEBUG__

#ifdef DEBUG
#define debug(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define debug(fmt, ...) ((void)0)
#endif /* DEBUG */

#endif /* __DEBUG__ */

/************************************************************************
This function updates the contents of *crcAccum using right shifts for
each bit. (Copied from IEEE Std 1815-2012 - Annex E CRC16 calculation)
************************************************************************/
void computeCRC(unsigned char dataOctet, unsigned short *crcAccum) {
    unsigned char idx;
    unsigned short temp;

    // Perform right shifts and update crc accumulator
    for (idx = 0; idx < 8; idx++)
    {
        temp = (*crcAccum ^ dataOctet) & 1;
        *crcAccum >>= 1;
        dataOctet >>= 1;
        if (temp) {
            *crcAccum ^= REVPOLY;
        }   
    }
} /* end computeCRC() */

int encode_dnp3_message(struct dnp3_message_read_request* dnp3_msg) {
    return 0;
}


int validate_rx_link_layer(unsigned char* buffer, struct dnp3_message_read_request* dnp3_msg) {
    // copy buffer onto dnp3_msg and validate CRC
    memcpy(dnp3_msg, buffer, sizeof(struct dnp3_message_read_request));
    // validate link layer CRC
    unsigned short expected_crc = 0;
    unsigned short block_size = sizeof(struct dnp3_link_layer) - 2;
    unsigned char* data_block = (unsigned char*)dnp3_msg;

    debug("Computing CRC for block_size %hu\n", 
        block_size);
    for (int idx = 0; idx < block_size; idx++) {
        debug("%02x ", data_block[idx]);
        computeCRC(data_block[idx],&expected_crc);
    }
    debug("\n");
    expected_crc = ~expected_crc;

    if (expected_crc != dnp3_msg->ll.crc) {
        debug("Got wrong ll crc 0x%04hx. Expected 0x%04hx\n",
               dnp3_msg->ll.crc, expected_crc);
        return -EINVAL;
    }

    dnp3_msg->ll.magic = ntohs(dnp3_msg->ll.magic);
    if (dnp3_msg->ll.magic != DNP3_MAGIC_SYNC_WORD) {
        debug("Got wrong magic number 0x%04hx. Expected 0x%04hx\n",
             dnp3_msg->ll.magic, DNP3_MAGIC_SYNC_WORD);
        return -EINVAL;
    }

    // convert network to host order for elements greater than 1-byte:
    dnp3_msg->ll.dst = ntohs(dnp3_msg->ll.dst);
    dnp3_msg->ll.src = ntohs(dnp3_msg->ll.src);
    dnp3_msg->ll.crc = ntohs(dnp3_msg->ll.crc);

    return 0;
}
