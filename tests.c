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
        test_error = (1 << (test_counter - 1));
        error_code |= test_error;
        printf("FAIL\n");
    }
    buffer[0] = 0x05;



    return error_code;
}

int compare_buffers(unsigned char* buf1, unsigned char* buf2, unsigned short len) {
    int error = 0;
    for (int i = 0; i < len; i++) {
        debug("buf1[%d] %02x buf2[%d] %02x\n", i, buf1[i], i, buf2[i]);
        if (buf1[i] != buf2[i]) {
            error = -1;
        }
    }

    return error;
}

int test_crc_calculations() {
    int error_code = 0;
    int test_counter = 1;
    int test_error;

    /**********************************************************************************************/
    printf("\nTEST CASE %d - Verify CRC calculation for single block size < 16 bytes...\n", test_counter++);
    {
        unsigned char buffer_in[] = {0x05, 0x64, 0x0b, 0xc4, 0x46, 0x00, 0x40, 0x00};
        unsigned char exp_buffer_out[] = {0x05, 0x64, 0x0b, 0xc4, 0x46, 0x00, 0x40, 0x00, 0xa3, 0xfe};
        unsigned char buffer_out[10] = {0};
        unsigned short output_length;

        output_length = appendCRC(buffer_in, sizeof(buffer_in), buffer_out);

        if (output_length != 10 || compare_buffers(buffer_out, exp_buffer_out, 10)) {
            debug("Output length: %hu\n", output_length);
            printf("FAIL\n");
            test_error = (1 << (test_counter - 1));
            error_code |= test_error;
        }
        else {
            printf("PASS\n");

        }
    }


    /**********************************************************************************************/
    printf("\nTEST CASE %d - Verify CRC calculation for single 16-byte block...\n", test_counter++);
    {
        unsigned char buffer_in[] = {0x94, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00};
        unsigned char exp_buffer_out[] = {0x94, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x66, 0x42};
        unsigned char buffer_out[18] = {0};
        unsigned short output_length;

        output_length = appendCRC(buffer_in, sizeof(buffer_in), buffer_out);


        if (output_length != 18 || compare_buffers(buffer_out, exp_buffer_out, 18)) {
            debug("Output length: %hu\n", output_length);
            printf("FAIL\n");
            test_error = (1 << (test_counter - 1));
            error_code |= test_error;
        }
        else {
            printf("PASS\n");

        }
    }

    /**********************************************************************************************/
    printf("\nTEST CASE %d - Verify CRC calculation for size > 16-byte...\n", test_counter++);
    {
        unsigned char buffer_in[] =         {0x94, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00};
        unsigned char exp_buffer_out[] =    {0x94, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x66, 0x42, 0x02, 0x00, 0x00, 0x00, 0x00, 0xf3, 0xb9};
        unsigned char buffer_out[25] = {0};
        unsigned short output_length;

        output_length = appendCRC(buffer_in, sizeof(buffer_in), buffer_out);

        if (output_length != 25 || compare_buffers(buffer_out, exp_buffer_out, 25)) {
            debug("Output length: %hu\n", output_length);
            printf("FAIL\n");
            test_error = (1 << (test_counter - 1));
            error_code |= test_error;
        }
        else {
            printf("PASS\n");

        }
    }
    return error_code;
}

int main() {
    int ret_code = 0;
    ret_code |= test_link_layer_validation();

    if (ret_code != 0) {
        printf("\nLink Layer validation ERRORS detected. One or more tests failed.\n");
        printf("\nInternal error code = 0x%08x\n", ret_code);
    }

    ret_code |= test_crc_calculations();
    if (ret_code != 0) {
        printf("\nOne or more CRC calculation tests failed.\n");
        printf("\nInternal error code = 0x%08x\n", ret_code);
    }

    return 0;
}