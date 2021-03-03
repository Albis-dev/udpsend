/* mbed OS */
#include "mbed.h"
#include "mbed_events.h"
/* STL */
#include <string>
#include <unordered_map>

/* General */
#include "include/main.h"
#include "include/network.h"

using std::string;

CircularBuffer<string, CIRCULAR_BUFFER_SIZE> commandBuffer;
EventQueue *shared_queue = mbed_event_queue();
EventQueue *high_prio_shared_queue = mbed_highprio_event_queue();
EventFlags event_flags;

/* map for functions */
typedef void (*command_function)(void); // function pointer type
typedef std::unordered_map<string, command_function> function_map;
function_map f_map;

namespace mbed
{
    FileHandle *mbed_override_console(int)
    {
        static UnbufferedSerial uart(USBTX, USBRX);
        return &uart;
    }
}

int main() {
    string data;

    eth_init(); // network.cpp
    udpsock_init(); // network.cpp
    f_map_init(); // main.cpp
    
    while(true) {
        event_flags.wait_any(BUFFER_READY);

        while (!commandBuffer.empty()) {
            commandBuffer.pop(data);
            auto iter = f_map.find(data);
            if (iter == f_map.end()) {
                data = "[" + data + "]" + " is not a valid command";
                report(data);
            } else {
                (*iter->second)();
            }
        }
    }
}

void f_map_init() {
    f_map.emplace("databomb", &databomb);
}

void databomb() {
    for (int i=0; i<100; i++) {
        report(to_string(i));
    }
}
