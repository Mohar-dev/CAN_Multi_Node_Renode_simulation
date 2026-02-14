/*
 * pat_trigger.c
 *
 * Sends PAT start command to ECU via SocketCAN
 * ID: 0x700
 * DATA[0]: 0xAA
 *
 * Compile:
 * gcc pat_trigger.c -o pat_trigger
 *
 * Run:
 * sudo ./pat_trigger vcan0
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#define CAN_HOST_START_PAT  0x700

void die(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;

    if(argc != 2)
    {
        printf("Usage: %s <can_interface>\n", argv[0]);
        printf("Example: %s vcan0\n", argv[0]);
        return 1;
    }

    const char *ifname = argv[1];

    /* 1️⃣ Create CAN RAW socket */
    sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if(sock < 0)
        die("Socket");

    /* 2️⃣ Specify CAN interface */
    strcpy(ifr.ifr_name, ifname);
    if(ioctl(sock, SIOCGIFINDEX, &ifr) < 0)
        die("ioctl");

    addr.can_family  = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    /* 3️⃣ Bind socket */
    if(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        die("Bind");

    printf("SocketCAN bound to %s\n", ifname);

    /* 4️⃣ Prepare CAN frame */
    frame.can_id  = CAN_HOST_START_PAT;
    frame.can_dlc = 1;
    frame.data[0] = 0xAA;

    /* 5️⃣ Send frame */
    if(write(sock, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
        die("Write");

    printf("Sent PAT START command (ID=0x700, DATA=0xAA)\n");

    /* 6️⃣ Optional: Listen for responses */
    printf("Listening for responses (Ctrl+C to exit)...\n");

    while(1)
    {
        struct can_frame rx_frame;
        int nbytes = read(sock, &rx_frame, sizeof(struct can_frame));

        if(nbytes > 0)
        {
            printf("RX: ID=0x%03X DLC=%d Data=",
                   rx_frame.can_id & CAN_EFF_MASK,
                   rx_frame.can_dlc);

            for(int i = 0; i < rx_frame.can_dlc; i++)
                printf("%02X ", rx_frame.data[i]);

            printf("\n");
        }
    }

    close(sock);
    return 0;
}

