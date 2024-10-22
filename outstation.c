#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include "dnp3.h"

#define BUFF_SIZE 1024

int main(int argc, char const *argv[]) {
    int server_fd, dnp3_socket, bytesread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    unsigned char buffer[BUFF_SIZE] = {0};
    unsigned short dnp3_local_address=0x46;
    if( argc == 2 ) {
      dnp3_local_address = atoi(argv[1]);
    }
    printf("Local address is %hu\n", dnp3_local_address);
    if (dnp3_local_address >= 65520)
    {
        errno = EINVAL;
        perror("Invalid local address configured");
        exit(EXIT_FAILURE);
    }
    struct dnp3_message_read_request rx_msg;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // TODO: review socket options later
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt))) {
        perror("set socket options failed");
        exit(EXIT_FAILURE);
    }

    // fill out address data structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(DNP3_SERVER_PORT);

    // bind, listen and accept socket connection
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind socket failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    if ((dnp3_socket = accept(server_fd, (struct sockaddr *)&address,
                        (socklen_t*)&addrlen)) < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }


    bytesread = read(dnp3_socket, buffer, BUFF_SIZE);
    printf("Message Received (%d bytes)...\n", bytesread);
    if (validate_rx_link_layer(buffer, &rx_msg, &dnp3_local_address) < 0) {
        perror("DNP3 link layer validation failed");
    } else {
        printf("DNP3 link layer OK!\n");
        printf("Crafting response packet!\n");
        memset(buffer, 0, BUFF_SIZE);
        struct dnp3_message_read_response tx_msg;
        int length;
        length = encode_dnp3_read_resp_message(&tx_msg, dnp3_local_address,
                                rx_msg.ll.src, buffer);
        if (length < 0) {
            perror("Error in the output message");
        } else {
            send(dnp3_socket, buffer, length, 0);
            printf("Response message sent\n");
        }
    }

    close(server_fd);

    return 0;

}