/*
 * protocol.h
 *
 * Shared header file for UDP client and server
 * Contains protocol definitions, data structures, constants and function prototypes
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdint.h>

/*
 * ============================================================================
 * PROTOCOL CONSTANTS
 * ============================================================================
 */

#define SERVER_PORT 56700
#define BUFFER_SIZE 512
#define DEFAULT_IP "127.0.0.1"

/*
 * ============================================================================
 * PROTOCOL DATA STRUCTURES
 * ============================================================================
 */

typedef struct {
    char type;        // Weather data type: 't', 'h', 'w', 'p'
    char city[64];    // City name (null-terminated string)
} weather_request_t;

typedef struct {
    unsigned int status;  // Response status code
    char type;            // Echo of request type
    float value;          // Weather data value
} weather_response_t;

/*
 * ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================
 */

// Add here the signatures of the functions you implement



#endif /* PROTOCOL_H_ */
