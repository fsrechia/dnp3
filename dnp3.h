#ifndef HEADER_CUSTOM_DNP3_H
#define HEADER_CUSTOM_DNP3_H

#define DNP3_SERVER_PORT 20000
#define REVPOLY 0xA6BC // Reverse polynomial from IEEE DNP3 standard

/************************************************************************
Layer structures and control bit field definitions
************************************************************************/

struct dnp3_link_layer {
    unsigned short magic;
    unsigned char len;
    unsigned char ctrl; // see definitions DNP3_LL_CTRL_* to understand how to use those bits
    unsigned short dst;
    unsigned short src;
    unsigned short crc;
} __attribute__((packed));

#define DNP3_MAGIC_SYNC_WORD 0x0564

/* Link layer control fields below are defined in 9.2.4.1.3 CONTROL field of the DNP3 standard */
#define DNP3_LL_CTRL_DIR_BIT 0x80        // 1 -> from master, 0 from outstation
#define DNP3_LL_CTRL_PRM_BIT 0x40        // 1 -> from primary to secondary, 0 from secondary to primary
#define DNP3_LL_CTRL_FCB_BIT 0x20        // 1 -> frame count bit (alternates 0 and 1)
#define DNP3_LL_CTRL_FCV_OR_DFC_BIT 0x10
                                          /* FCV - Frame count valid (1 -> examine FCB, 0 -> ignore FCB)
                                          * DFC - Data flow control (1 -> buffer full, 0 -> buffer available)
                                          */
#define DNP3_LL_CTRL_FC_BITS 0x0f
                                    /* LINK LAYER Function CODE bits (see section 9.2.4.1.3.6):
                                    *
                                    * if PRM=1
                                    *      (Primary function code, Function code name, Service function, FCV bit, Response function codes permitted from Secondary Station)
                                    *      0, RESET_LINK_STATES, Reset of remote link, FCV 0, responses 0 or 1
                                    *      2, TEST_LINK_STATES, Test function for link, FCV 1, responses 0 or 1 (no response is acceptable if the link states are UnReset)
                                    *      3, CONFIRMED_USER_DATA, Deliver application data confirmation requested, FCV 1, responses 0 or 1
                                    *      4, UNCONFIRMED_USER_DATA, Deliver application data confirmation not requested, FCV 0, No Secondary Station Data Link response
                                    *      9, REQUEST_LINK_STATUS, Request status of link, FCV 0, responses 11
                                    * if PRM=0
                                    *      Secondary function code, Function code name, Service function
                                    *      0, ACK, Positive acknowledgment
                                    *      1, NACK, Negative acknowledgment
                                    *      11, LINK_STATUS, Status of the link
                                    *      15, NOT_SUPPORTED, Link service not supported
                                    */

/* Transport layer control fields (this layer is a single byte) */
#define DNP3_TL_FIN_BIT 0x80   // 1-> Final fragment of the message
#define DNP3_TL_FIR_BIT 0x40   // 1-> First fragment of the message
#define DNP3_TL_SEQ_BITS 0x3f   // 6 bits used for sequentially numbering fragments

// TODO: I need to understand this better before specifying structures
// app layer requests do not carry an internal indications field (responses do carry this field)
struct dnp3_app_layer_header {
    unsigned char ctrl; // application control
    unsigned char fc;   // function code
} __attribute__((packed));

/* Application layer control fields */
#define DNP3_AL_FIN_BIT 0x80   // 1-> Final fragment of the message
#define DNP3_AL_FIR_BIT 0x40   // 1-> First fragment of the message
#define DNP3_AL_CON_BIT 0x20   // Confirm bit
#define DNP3_AL_UNS_BIT 0x10   // Unsolicited bit
#define DNP3_AL_SEQ_BIT 0x0f   // 4 bits used for numbering app-level fragments

/* Application layer function codes fields (this layer is a single byte)
 * see DNP3 document section 4.2.2.5 for more details on other Function code octets
 */
#define DNP3_AL_FC_CONFIRM  0x00   // Confirmation - Confirm Function Code
#define DNP3_AL_FC_READ     0x01   // Request - Read Function Code
#define DNP3_AL_FC_WRITE    0x02   // Request - Write Function Code
#define DNP3_AL_FC_RESPONSE 0x81   // Response - Solicited Response

struct dnp3_class_object_header {
    unsigned char type_group;
    unsigned char type_variation;
    unsigned char qualifier;
} __attribute__((packed));

struct dnp3_message_read_request {
    struct dnp3_link_layer ll;              //link layer
    unsigned char tl;                       //transport layer
    struct dnp3_app_layer_header al_header; // app layer
    struct dnp3_class_object_header al_obj; // app layer object
    unsigned short app_crc;
} __attribute__((packed));

struct dnp3_message_read_response {
    struct dnp3_app_layer_header al_header;   // application control + function code struct
    unsigned short indications;               // internal indications
    // TODO: add optional (zero or more) Object ranges?
} __attribute__((packed));

/************************************************************************
DNP3 function prototypes
************************************************************************/

void computeCRC(unsigned char, unsigned short*);
int encode_dnp3_message(struct dnp3_message_read_request*);
int generate_crc(unsigned char*, unsigned short, unsigned short*);
int validate_rx_link_layer(const unsigned char*, struct dnp3_message_read_request*,
                           const unsigned short*);

#endif // HEADER_CUSTOM_DNP3_H
