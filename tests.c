#include "dnp3.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

int test_link_layer_validation() {
    struct dnp3_message_read_request dnp3_msg;
    // Test sample message
    unsigned char buffer[] = {0x05, 0x64, 0x0b, 0xc4, 0x46, 0x00, 0x40, 0x00,
                              0xa3, 0xfe, 0xd0, 0xcd, 0x01, 0x3c, 0x02, 0x06,
                              0xc2, 0x62};
    memset(&dnp3_msg, 0, sizeof(dnp3_msg));

    if (validate_rx_link_layer (buffer, &dnp3_msg) == 0) {
        printf("sample message OK\n");
    }
    else {
        printf("sample message ERROR\n");
        return -1;
    }

    // Test sample message wrong magic number
    buffer[0] = 0x04;
    memset(&dnp3_msg, 0, sizeof(dnp3_msg));

    if (validate_rx_link_layer (buffer, &dnp3_msg) == -EINVAL) {
        printf("wrong magic number detected - OK\n");
    }
    else {
        printf("wrong magic number not detected - ERROR\n");
        return -1;
    }
    return 0;
}

int main() {
    test_link_layer_validation();

    return 0;
}