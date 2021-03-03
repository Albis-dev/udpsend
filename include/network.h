#include "mbed.h"
#include "platform/CircularBuffer.h"
#include "EthernetInterface.h"
#include "LWIPStack.h"

/* Environment dependent settings */
#define HOME

#ifndef HOME
#define PC_IP "192.168.1.30"
#endif

#ifdef HOME
#define PC_IP "192.168.1.234"
#endif

/* General network settings */
#define MY_IP "192.168.1.135"
#define MY_MASK "255.255.255.0"
#define MY_GATEWAY "192.168.1.1"
#define PC_RX_PORT 8000
#define PC_TX_PORT 8001
#define UDP_BUFFER_SIZE 128 // * sizeof(char)

using std::string;

int eth_init();
int udpsock_init();
void udp_recvhandler();
void parse_and_store(string);
void report(string);