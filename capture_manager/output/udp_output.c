

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "../common.h"
#include "output.h"


#ifdef __cplusplus
extern "C" {
#endif

int g_socket = -1;
struct sockaddr_in g_server_addr;
socklen_t g_addr_size;

unsigned int g_seqnum = 0;
const unsigned char* FRAME_INDICATOR = "DeepCode_0123456";

static int udp_open(output_param_t* param)
{
    const int port = 7891;
    const char* address = "192.168.2.200";

    /*Create UDP socket*/
    g_socket = socket(PF_INET, SOCK_DGRAM, 0);

    /*Configure settings in address struct*/
    g_server_addr.sin_family = AF_INET;
    g_server_addr.sin_port = htons(port);
    g_server_addr.sin_addr.s_addr = inet_addr(address);
    memset(g_server_addr.sin_zero, '\0', sizeof g_server_addr.sin_zero);

    /*Initialize size variable to be used later on*/
    g_addr_size = sizeof(g_server_addr);

    log_info("Create UDP client socket: %s:%d\n", address, port);

    return 0;
}

static int udp_start()
{
    log_debug("%s is called\n", __func__);
    return 0;
}

static void udp_close()
{
    close(g_socket);
    g_socket = -1;
}

static int udp_send(unsigned char* buf, int size)
{
    int ret = sendto(g_socket, buf, size, 0, (struct sockaddr *)&g_server_addr, g_addr_size);
    if (ret < 0) {
        return -1;
    }
    return 0;
}

#define UDP_SEND(data, size) \
    if (udp_send((unsigned char*)(data), (size)) < 0) \
        return -1;

static int udp_write_frame(unsigned char * frame_buf, int frame_size)
{
    //char seqstr[8];

    //uint32_t crc = 0;
    //++g_seqnum;
    //sprintf(seqstr, "%04d", g_seqnum);

    //UDP_SEND(FRAME_INDICATOR, strlen(FRAME_INDICATOR))
    //UDP_SEND(seqstr, 4);
    //UDP_SEND(&(g_seqnum), sizeof(g_seqnum))
    //UDP_SEND(&frame_size, sizeof(frame_size))
    //UDP_SEND(&crc, sizeof(crc))
    UDP_SEND(frame_buf, frame_size);
    return 0;
}

output_device_t udp_output_device = {
    .name = "udp output",
    .open = udp_open,
    .start = udp_start,
    .close = udp_close,
    .write_frame = udp_write_frame,
    .get_next_buf = NULL
};

#ifdef __cplusplus
}
#endif

