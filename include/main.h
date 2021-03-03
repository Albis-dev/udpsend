#include <string>
#include <chrono>
#include "mbed.h"
#include "mbed_events.h"

#define BUFFER_READY (1UL << 0)

#define CIRCULAR_BUFFER_SIZE 128 // * sizeof(string) 

extern CircularBuffer<string, CIRCULAR_BUFFER_SIZE> commandBuffer;
extern EventQueue *shared_queue;
extern EventQueue *high_prio_shared_queue;
extern EventFlags event_flags;

void f_map_init();
void databomb();
