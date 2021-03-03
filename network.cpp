#include "mbed.h"
#include "EthernetInterface.h"
#include "LWIPStack.h"
#include "include/main.h"
#include "include/network.h"
#include <vector>
using std::string;

UDPSocket udpsock;

EthernetInterface eth;
SocketAddress myIP; // Board info
SocketAddress pcIP; // PC info

int eth_init() {
    nsapi_size_or_error_t response;
    SocketAddress netmask;
    SocketAddress gateway;

    /* NETWORK INIT */
    myIP.set_ip_address(MY_IP);
    netmask.set_ip_address(MY_MASK);
    gateway.set_ip_address(MY_GATEWAY);
    response = eth.set_network(myIP, netmask, gateway);
    if (response == NSAPI_ERROR_UNSUPPORTED) {
        return -1;
    }
    response = eth.connect();
    if (response < 0) {
        return -1;
    }

    pcIP.set_ip_address(PC_IP);
    pcIP.set_port(PC_RX_PORT);

    return 0;
}

int udpsock_init() {
    nsapi_size_or_error_t response;

    // Open up the UDP socket and bind pre-defined port. See config.h
    response = udpsock.open(&eth);
    if (response < 0) {
        return -1;
    }

    response = udpsock.bind(PC_TX_PORT);
    if (response < 0) {
        return -1;
    }

    // Receive data on the shared thread context
    udpsock.set_blocking(false); 
    udpsock.sigio(high_prio_shared_queue->event(udp_recvhandler));

    report("UDPSocket Initialized");

    return 0;
}

void udp_recvhandler() {
    char buffer[UDP_BUFFER_SIZE];
    nsapi_size_or_error_t response = udpsock.recv(buffer, UDP_BUFFER_SIZE);

    if (response == NSAPI_ERROR_WOULD_BLOCK) {
        return;
    } else if (response < 0) {
        return;
    }

    shared_queue->call(parse_and_store, string(buffer, response));
}

void parse_and_store(string message) {
    SocketAddress sender;   
    size_t pos;
    std::vector<string> tokens;
    string token;

    while(true) {
        if ((pos = message.find(" ")) == std::string::npos) {
            break;
        }
        token = message.substr(0, pos);
        message.erase(0, pos+1);
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    tokens.push_back(message);

    if (commandBuffer.size() + tokens.size() > CIRCULAR_BUFFER_SIZE) {
        string warning = "BUFFER IS FULL. DROPPING COMMAND";
        report(warning);
        return;
    }

    for (auto it=tokens.begin(); it!=tokens.end(); it++) {
        commandBuffer.push(*it);
    }
    
    event_flags.set(BUFFER_READY);
}

void report(string msg) {
    nsapi_size_or_error_t response;
    while(true) {
        response = udpsock.sendto(pcIP, msg.c_str(), msg.size());
        if (response == NSAPI_ERROR_NO_SOCKET) {
            return;
        } else if (response == NSAPI_ERROR_WOULD_BLOCK) {
            continue;
        } else {
            break;
        }
    } 
}
