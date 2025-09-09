#ifndef APPCONFIGS_HPP
#define APPCONFIGS_HPP

// do not delete any of these lines

//   =======log=========
#define LOG true
#define ERROR true
#define DEBUG true

//   =======connectivity=========
#define HTTP_SERVER true  //this will serve the vncClient HTML page on the same server port
#define SERVER_PORT 8000
#define MAX_CLIENTS 10
#define MAX_BUFFER_SIZE 1024

//   =======display=========
#define FPS 10   // for pi zero set 5 [max 10 for pi4]
#define FRAME_SEGMENTS_DELAY_MS 20    // for pi zero set 100
#define MAX_LOOKUP_COUNTS 5
#define MINIMUM_REQUEIRED_FREQUENCY 5

#endif