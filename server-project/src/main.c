/*
 * main.c
 *
 * UDP Server - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a UDP server
 * portable across Windows, Linux, and macOS.
 */

#if defined WIN32
#include <winsock.h>

typedef int socklen_t;
#define strcasecmp _stricmp
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "protocol.h"

#define NO_ERROR 0
#define BUFFMAX 255
weather_request_t request;

void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

void errorhandler(const char *error_message)
{
	printf("%s\n", error_message);
}

static const char *SUPPORTED_CITIES[] =
{
	"Bari",
	"Roma",
	"Milano",
	"Napoli",
	"Torino",
	"Palermo",
	"Genova",
	"Bologna",
	"Firenze",
	"Venezia"
};


float random_float(float min, float max) {
	float scale = rand() / (float) RAND_MAX;
	return min + scale * (max - min);
}


void valida(weather_request_t *req, weather_response_t *resp)
{

	if(req->type != 't' && req->type != 'h' && req->type != 'w' && req->type != 'p')
	{
		resp->status = 2;
		return;
	}


	int flag = 1;
	for (int i = 0; i < 10; i++)
	{
		if(strcasecmp(req->city, SUPPORTED_CITIES[i]) == 0)
		{
			flag = 0;
			break;
		}
	}

	if(flag == 1)
	{
		resp->status = 1;
	} else {
		resp->status = 0;
	}
}


float get_temperature(void)
{
	return random_float(-10.0, 40.0);
}
float get_humidity(void)
{
	return random_float(20.0, 100.0);
}
float get_wind(void)
{
	return random_float(0.0, 100.0);
}
float get_pressure(void)
{
	return random_float(950.0, 1050.0);
}


void deserializza(char buffer[BUFFMAX], int rcv_msg_size)
{
	if (rcv_msg_size > 0)
	{
	        int offset = 0;



	        memcpy(&request.type, buffer + offset, sizeof(char));
	        offset += sizeof(char);



	        memcpy(request.city, buffer + offset, 20);
	        // Sicurezza: assicurati che la stringa sia terminata se la usi con printf
	        request.city[19] = '\0';
	        offset += 20;

	        printf("Ricevuto: Type=%c, City=%s\n",
	               request.type, request.city);
	    }
}

int main(int argc, char *argv[])
{

	// TODO: Implement server logic

#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR)
	{
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif


	int port = SERVER_PORT;
	if (argc > 2 && strcmp(argv[1], "-p") == 0)
	{
		port = atoi(argv[2]);
	}


	// TODO: Create UDP socket

	// TODO: Configure server address
	
	// TODO: Bind socket

	// TODO: Implement UDP datagram reception loop 


	int my_socket;

		/* create a UDP socket */
		if ((my_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		{
			errorhandler("Error creating socket");
			clearwinsock();
			return -1;
		}

		/* set the server address */
		struct sockaddr_in sad;
		struct sockaddr_in cad;
		unsigned int client_address_length = sizeof(cad);
		memset(&sad, 0, sizeof(sad));
		sad.sin_family = AF_INET;
		sad.sin_port = htons(port);
		sad.sin_addr.s_addr = inet_addr(INADDR_ANY);

		/* set server address */
		if ((bind(my_socket, (struct sockaddr *)&sad,
				  sizeof(sad))) < 0)
		{
			errorhandler("bind() failed");
			closesocket(my_socket);
			clearwinsock();
			return -1;
		}

		char buffer[BUFFMAX];
		int rcv_msg_size;
		while (1)
		{
			puts("\nServer listening...");
			/* clean buffer */
			memset(buffer, 0, BUFFMAX);
			/* receive message from client */
			/*if ((rcv_msg_size = recvfrom(my_socket, buffer, BUFFMAX, 0,
										 (struct sockaddr *)&cad, &client_address_length)) < 0)
			{
				errorhandler("recvfrom() Helo failed");
				closesocket(my_socket);
				clearwinsock();
				return -1;
			}
			struct hostent *client_host = gethostbyaddr((char *)&cad.sin_addr.s_addr, sizeof(cad.sin_addr.s_addr), AF_INET);
			printf("Received '%s' from client %s (IP %s, port %d)\n", buffer, client_host->h_name, inet_ntoa(cad.sin_addr), ntohs(cad.sin_port));
*/
			/* receive data from the client */
			memset(buffer, 0, BUFFMAX);
			if ((rcv_msg_size = recvfrom(my_socket, buffer, BUFFMAX, 0,
										 (struct sockaddr *)&cad, &client_address_length)) < 0)
			{
				errorhandler("recvfrom() data failed");
				closesocket(my_socket);
				clearwinsock();
				return -1;
			}

			printf("Richiesta ricevuta da localhost (%s): type='%c', city='%s'");

			weather_response_t response;
			valida(&request, &response);

			if(response.status == 0)
			{
				switch (request.type)
				{
					case 't':
						response.value = get_temperature();
					break;
					case 'h':
						response.value = get_humidity();
					break;
					case 'w':
						response.value = get_wind();
					break;
					case 'p':
						response.value = get_pressure();
					break;
				}
				response.type = request.type;
			}else
			{
				response.type = '\0';
				response.value = 0.0;


				/* remove vowels from buffer and send it back to the client */


				printf("Sending '%s' back to client\n", buffer);
				if (sendto(my_socket, buffer, strlen(buffer), 0,
						   (struct sockaddr *)&cad, sizeof(cad)) != strlen(buffer))
				{
					errorhandler("sendto() sent a different number of bytes than expected");
					closesocket(my_socket);
					clearwinsock();
					return -1;
				}
			}
		}



	printf("Server terminated.\n");

	closesocket(my_socket);
	clearwinsock();
	return 0;
} // main end
