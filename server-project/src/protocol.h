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
#define DEFAULT_IP "127.0.0.1"
#define BUFFER_SIZE 512


/*
 * ============================================================================
 * PROTOCOL DATA STRUCTURES
 * ============================================================================
 */

	typedef struct {
		char type;        // 't', 'h', 'w', 'p'
		char city[64];    // Nome città
	} weather_request_t;

	typedef struct {
		unsigned int status;
		char type;
		float value;
	} weather_response_t;

/*
 * ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================
 */

// Add here the signatures of the functions you implement
	void valida(weather_request_t *req, weather_response_t *resp);
	float random_float(float min, float max);

	float get_temperature(void);
	float get_humidity(void);
	float get_wind(void);
	float get_pressure(void);


#endif /* PROTOCOL_H_ */
