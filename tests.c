#include "dnp3.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

int test_link_layer_validation() {
    struct dnp3_message_read_request dnp3_msg;
    unsigned short local_address=0x4600;
    unsigned short new_crc;
    int error_code = 0;
    int test_counter = 1;
    int test_error;

    /**********************************************************************************************/
    printf("\nTEST CASE %d - Verify link layer validation with correct sample message...\n", test_counter++);
    unsigned char buffer[] = {0x05, 0x64, 0x0b, 0xc4, 0x46, 0x00, 0x40, 0x00,
                              0xa3, 0xfe, 0xd0, 0xcd, 0x01, 0x3c, 0x02, 0x06,
                              0xc2, 0x62};
    memset(&dnp3_msg, 0, sizeof(dnp3_msg));

    if (validate_rx_link_layer(buffer, &dnp3_msg, &local_address) == 0) {
        printf("PASS\n");
    }
    else {
        test_error = (1 << (test_counter - 1));
        error_code |= test_error;
        printf("FAIL 0x%08x\n", test_error);
    }

    /**********************************************************************************************/
    printf("\nTEST CASE %d - Verify link layer validation with wrong CRC...\n", test_counter++);
    // change a bit so that CRC gets wrong
    buffer[0] = 0x04;
    memset(&dnp3_msg, 0, sizeof(dnp3_msg));

    if (validate_rx_link_layer (buffer, &dnp3_msg, &local_address) == -EINVAL) {
        printf("error detected - PASS\n");
    }
    else {
        test_error = (1 << (test_counter - 1));
        error_code |= test_error;
        printf("error not detected - FAIL 0x%08x\n", test_error);
    }
    buffer[0] = 0x05;

    /**********************************************************************************************/
    printf("\nTEST CASE %d - Verify link layer validation with wrong magic number...\n", test_counter++);
    // keep wrong magic number but with correct CRC
    buffer[0] = 0x04;
    generate_crc(buffer, (unsigned short)8, &new_crc);
    buffer[8] = 0xff & new_crc;
    buffer[9] = new_crc >> 8;

    memset(&dnp3_msg, 0, sizeof(dnp3_msg));
    if (validate_rx_link_layer (buffer, &dnp3_msg, &local_address) == -EINVAL) {
        printf("wrong address detected - PASS\n");
    }
    else {
        test_error = (1 << (test_counter - 1));
        error_code |= test_error;
        printf("wrong address not detected - FAIL 0x%08x\n", test_error);
    }
    buffer[0] = 0x05;

    /**********************************************************************************************/
    printf("\nTEST CASE %d - Verify link layer validation with wrong destination...\n", test_counter++);
    // change destination address and recalculate CRC
    buffer[5] = 0x01;
    generate_crc(buffer, (unsigned short)8, &new_crc);
    buffer[8] = 0xff & new_crc;
    buffer[9] = new_crc >> 8;

    memset(&dnp3_msg, 0, sizeof(dnp3_msg));
    if (validate_rx_link_layer (buffer, &dnp3_msg, &local_address) == -ENXIO) {
        printf("PASS\n");
    }
    else {
        error_code |= (1 << (test_counter - 1));
        printf("FAIL\n");
    }
    buffer[0] = 0x05;



    return error_code;
}

int main() {
    int ret_code = 0;
    ret_code |= test_link_layer_validation();

    if (ret_code != 0) {
        printf("\nERRORS detected. One or more tests failed.\n");
        printf("\nInternal error code = 0x%08x\n", ret_code);
        return -1;
    }
    printf("\nNo errors detected. All tests passed.\n");
    return 0;
}