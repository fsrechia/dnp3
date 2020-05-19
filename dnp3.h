struct dnp3_link_layer {
    unsigned short magic = 0x0564;
    unsigned char len;
    unsigned char ctrl; // see definitions DNP3_LL_CTRL_* to understand how to use those bits
    unsigned short dst;
    unsigned short src;
    unsigned short crc;
};

/* Link layer control fields below are defined in 9.2.4.1.3 CONTROL field of the DNP3 standard */
#define DNP3_LL_CTRL_DIR_BIT 0x80;        // 1 -> from master, 0 from outstation
#define DNP3_LL_CTRL_PRM_BIT 0x40;        // 1 -> from primary to secondary, 0 from secondary to primary
#define DNP3_LL_CTRL_FCB_BIT 0x20;        // 1 -> frame count bit (alternates 0 and 1)
#define DNP3_LL_CTRL_FCV_OR_DFC_BIT 0x10; /* FCV - Frame count valid (1 -> examine FCB, 0 -> ignore FCB)
                                          * DFC - Data flow control (1 -> buffer full, 0 -> buffer available) 
                                          */
#define DNP3_LL_CTRL_FC_BITS 0x0f;  /* LINK LAYER Function CODE bits (see section 9.2.4.1.3.6):
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
#define DNP3_TL_FIN_BIT 0x80;   // 1-> Final fragment of the message
#define DNP3_TL_FIR_BIT 0x40;   // 1-> First fragment of the message
#define DNP3_TL_SEQ_BITS 0x3f;   // 6 bits used for sequentially numbering fragments

// TODO: I need to understand this better before specifying structures
// app layer requests do not carry an internal indications field
struct dnp3_app_layer_ctrl_fc {
    unsigned char ctrl; // application control
    unsigned char fc;   // function code
};

struct dnp3_app_layer {
    struct dnp3_app_layer_ctrl_fc ctrlfc;   // application control + function code struct
    unsigned short indications;             // internal indications
    // TODO: add optional (zero or more) Object ranges?
};
