//Infrared Remote Control
#include <IRremoteESP8266.h> // Include IRremoteESP8266 library
#include <IRrecv.h>  // Include IRrecv class
#include <IRutils.h> // Include IRutils utility class

// Define the pin connected to the infrared receiver module
const int ir_recv_pin = 2;
IRrecv irrecv(ir_recv_pin); // Create IRrecv object
decode_results results;     // Create decode_results object
