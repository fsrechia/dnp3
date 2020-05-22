#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "dnp3.h"


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

/**************************************************************************
This function illustrates how to compute the CRC and append it to a data
block for transmission. (based on example from IEEE Std 1815-2012 - Annex E)
**************************************************************************/
int appendCRC(unsigned char* srcbuffer, unsigned short srclen, unsigned char* dstbuffer) {
    // TODO: tidy up this spaghetti code... it can be made a lot clearer
    short i, j; // Indexes
    short blockSize = 16; // Size of data block, not including CRC octets
    unsigned short crc; // 16-bit check code (crc accumulator)
    unsigned short whole_chunks = (unsigned short) (srclen / 16);
    unsigned short remaining_bytes = srclen % 16;
    unsigned short newlength = (whole_chunks * 18);
    debug("appendCRC - whole_chunks: %hu, remaining_bytes: %hu\n",whole_chunks,remaining_bytes);

    for (i = 0; i < whole_chunks; i++) {
        memcpy(dstbuffer + i*(blockSize + 2), srcbuffer + i*blockSize, blockSize);
        // Compute check code
        crc = 0; // Initialize
        for (j = i*blockSize; j < (i + 1)*blockSize; j++) {
            computeCRC(dstbuffer[j],&crc);
        }
        crc = ~crc; // Invert
        // Append CRC to end of block
        dstbuffer[j++] = (unsigned char)crc;
        dstbuffer[j] = (unsigned char)(crc >> 8);
    }
    // process remaining bytes for last blocksize < 16 bytes
    if (remaining_bytes > 0) {
        newlength += remaining_bytes + 2;
        memcpy(dstbuffer + (i*(blockSize + 2)), srcbuffer + (i*blockSize), remaining_bytes);
        // Compute check code
        crc = 0; // Initialize
        for (j = i*(blockSize + 2); j < i*(blockSize + 2) + remaining_bytes; j++) {
            computeCRC(dstbuffer[j],&crc);
        }
        crc = ~crc; // Invert
        // Append CRC to end of block
        dstbuffer[j++] = (unsigned char)crc;
        dstbuffer[j] = (unsigned char)(crc >> 8);
    }

    return newlength;
}

int encode_dnp3_read_req_message(struct dnp3_message_read_request* dnp3_msg) {
    return 0;
}

int encode_dnp3_read_resp_message(
    struct dnp3_message_read_response* dnp3_msg,
    unsigned short src,
    unsigned short dst,
    unsigned char* outbuffer
    )
{
    // dummy user data = transport + app layer object list response without CRCs embedded
    unsigned char user_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x02, 0x28, 0x04, 0x00, 0xe0, 0x00, 0x81, 0x99, 0xd7, 0x74,
    0x10, 0x72, 0x01, 0xdc, 0x00, 0x81, 0x99, 0xd7, 0x74, 0x10, 0x72, 0x01, 0xdf, 0x00, 0x81, 0x99,
    0xd7, 0x74, 0x10, 0x72, 0x01, 0xe1, 0x00, 0x81, 0x99, 0xd7, 0x74, 0x10, 0x72, 0x01};
    unsigned short ll_crc;
    int length;
    unsigned short ll_len = sizeof(user_data) + 5; // USER_DATA + SRC + DST + CTRL

    memset(dnp3_msg, 0, sizeof(struct dnp3_message_read_response));
    dnp3_msg->ll.magic = htons(0x0564);
    dnp3_msg->ll.len = ll_len;
    dnp3_msg->ll.src = htons(src);
    dnp3_msg->ll.dst = htons(dst);
    dnp3_msg->ll.ctrl = DNP3_LL_CTRL_PRM_BIT | (DNP3_LL_CTRL_FC_BITS & 4); // UNCONFIRMED_USER_DATA
    generate_crc((unsigned char*)dnp3_msg, 8, &ll_crc);
    dnp3_msg->ll.crc = htons(ll_crc);
    dnp3_msg->tl = DNP3_TL_FIN_BIT | DNP3_TL_FIR_BIT | 1;
    dnp3_msg->al_header.ctrl = (DNP3_AL_FIN_BIT | DNP3_AL_FIR_BIT);
    dnp3_msg->al_header.fc = DNP3_AL_FC_RESPONSE;
    dnp3_msg->indications = htons(0x0600);

    // copy link layer only
    length = sizeof(struct dnp3_link_layer);
    memcpy(outbuffer, dnp3_msg, sizeof(struct dnp3_link_layer));
    user_data[0] = dnp3_msg->tl;
    user_data[1] = dnp3_msg->al_header.ctrl;
    user_data[2] = dnp3_msg->al_header.fc;
    user_data[3] = dnp3_msg->indications & 0xff;
    user_data[4] = dnp3_msg->indications >> 8;

    length += appendCRC(user_data, (unsigned short) sizeof(user_data), outbuffer + sizeof(struct dnp3_link_layer));

    return length;
}

int generate_crc(
        unsigned char* data_block,
        unsigned short block_size,
        unsigned short* result_crc
        )
{
    *result_crc = 0;
    //debug("Computing CRC for block_size %hu\n", block_size);
    for (int i = 0; i < block_size; i++) {
        //debug("%02x ", data_block[i]);
        computeCRC(data_block[i], result_crc);
    }
    //debug("\n");
    *result_crc = ~*result_crc;

    return 0;
}



int validate_rx_link_layer(
        const unsigned char* buffer,
        struct dnp3_message_read_request* dnp3_msg,
        const unsigned short* exp_dst_address
        )
{
    // copy buffer onto dnp3_msg and validate CRC
    memcpy(dnp3_msg, buffer, sizeof(struct dnp3_message_read_request));

    // validate link layer CRC
    unsigned short expected_crc;
    unsigned short block_size = sizeof(struct dnp3_link_layer) - sizeof(expected_crc);
    unsigned char* data_block = (unsigned char*)dnp3_msg;
    generate_crc(data_block, block_size, &expected_crc);
    if (expected_crc != dnp3_msg->ll.crc) {
        debug("Got crc 0x%04hx. Expected 0x%04hx\n", dnp3_msg->ll.crc, expected_crc);
        errno = EINVAL;
        return -EINVAL;
    }

    // validate magic number
    dnp3_msg->ll.magic = ntohs(dnp3_msg->ll.magic);
    if (dnp3_msg->ll.magic != DNP3_MAGIC_SYNC_WORD) {
        debug("Got wrong magic number 0x%04hx. Expected 0x%04hx\n",
             dnp3_msg->ll.magic, DNP3_MAGIC_SYNC_WORD);
        errno = EINVAL;
        return -EINVAL;
    }

    // convert network to host order for elements greater than 1-byte:
    dnp3_msg->ll.dst = ntohs(dnp3_msg->ll.dst);
    dnp3_msg->ll.src = ntohs(dnp3_msg->ll.src);
    dnp3_msg->ll.crc = ntohs(dnp3_msg->ll.crc);

    // validate destination address
    if ( (dnp3_msg->ll.dst != *exp_dst_address) &&
         (dnp3_msg->ll.dst <= 0xfffc) // special addresses
    ) {
        debug("Dest Address: %hu, Expected Address %hu or special addresses\n",
             dnp3_msg->ll.dst, *exp_dst_address);
        errno = ENXIO;
        return -ENXIO;
    }

    // TODO: more validation must be done according to Section 9.2.9 of
    // DNP3 standard.

    return 0;
}
