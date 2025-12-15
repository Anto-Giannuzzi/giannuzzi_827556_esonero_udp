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

#ifndef NO_ERROR
#define NO_ERROR 0
#endif

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif

weather_request_t request;
weather_response_t response;

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


/*void deserializza(char buffer[BUFFMAX], int rcv_msg_size)
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
}*/

int main(int argc, char *argv[])
{

	// TODO: Implement server logic

	srand(time(NULL));

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

	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_port = htons(port);
	sad.sin_addr.s_addr = htonl(INADDR_ANY);

	/* set server address */
	if ((bind(my_socket, (struct sockaddr *)&sad,
			  sizeof(sad))) < 0)
	{
		errorhandler("bind() failed");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	char buffer_in[BUFFER_SIZE];
	    char buffer_out[BUFFER_SIZE];
	    struct sockaddr_in cad; // Client Address
	    int client_len = sizeof(cad);
	    int rcv_msg_size;

	    puts("\nServer in ascolto..."); // Messaggio iniziale generico

	    while (1)
	    {
	        // Pulizia strutture per ogni iterazione
	        memset(&request, 0, sizeof(request));
	        memset(&response, 0, sizeof(response));
	        memset(buffer_in, 0, BUFFER_SIZE);

	        // A. RICEZIONE DATAGRAMMA
	        client_len = sizeof(cad); // Reset lunghezza
	        if ((rcv_msg_size = recvfrom(my_socket, buffer_in, BUFFER_SIZE, 0,
	                                     (struct sockaddr *)&cad, &client_len)) < 0)
	        {
	            errorhandler("recvfrom() failed");
	            continue; // Non terminare il server, prova prossima richiesta
	        }

	        // B. DESERIALIZZAZIONE MANUALE (Richiesta)
	        int offset = 0;

	        // 1. Leggi Type (1 byte)
	        memcpy(&request.type, buffer_in + offset, sizeof(char));
	        offset += sizeof(char);

	        // 2. Leggi City (copia il resto come stringa)
	        // Calcoliamo quanto resta nel buffer ricevuto
	        int city_len = rcv_msg_size - offset;
	        if (city_len > 63) city_len = 63; // Protezione buffer overflow
	        if (city_len < 0) city_len = 0;

	        memcpy(request.city, buffer_in + offset, city_len);
	        request.city[city_len] = '\0'; // Assicura null-termination

	        // C. LOGGING (Reverse DNS Lookup)
	        char host_name[256];
	        char client_ip[INET_ADDRSTRLEN];

	        // IP to String
	        char *temp_ip = inet_ntoa(cad.sin_addr);
	        strncpy(client_ip, temp_ip, INET_ADDRSTRLEN);

	        // Reverse Lookup
	        struct hostent *he;
	        he = gethostbyaddr((char *)&cad.sin_addr, sizeof(cad.sin_addr), AF_INET);

	        if (he != NULL) {
	            strcpy(host_name, he->h_name);
	        } else {
	            strcpy(host_name, client_ip);
	        }

	        // Stampa Log Richiesto: "Richiesta ricevuta da localhost (ip 127.0.0.1): type='t', city='Roma'"
	        printf("Richiesta ricevuta da %s (ip %s): type='%c', city='%s'\n",
	               host_name, client_ip, request.type, request.city);

	        // D. ELABORAZIONE (Business Logic)
	        valida(&request, &response);

	        // Se la validazione passa (status 0), genera i dati
	        if(response.status == 0)
	        {
	            response.type = request.type; // Echo del tipo
	            switch (request.type)
	            {
	                case 't': response.value = get_temperature(); break;
	                case 'h': response.value = get_humidity(); break;
	                case 'w': response.value = get_wind(); break;
	                case 'p': response.value = get_pressure(); break;
	            }
	        }
	        else
	        {
	            // In caso di errore, type e value sono irrilevanti ma puliti
	            response.type = request.type;
	            response.value = 0.0f;
	        }

	        // E. SERIALIZZAZIONE MANUALE (Risposta)
	        offset = 0;

	        // 1. Status (uint32 -> htonl)
	        uint32_t net_status = htonl(response.status);
	        memcpy(buffer_out + offset, &net_status, sizeof(uint32_t));
	        offset += sizeof(uint32_t);

	        // 2. Type (char)
	        memcpy(buffer_out + offset, &response.type, sizeof(char));
	        offset += sizeof(char);

	        // 3. Value (float -> uint32 -> htonl)
	        uint32_t net_val;
	        memcpy(&net_val, &response.value, sizeof(float));
	        net_val = htonl(net_val);
	        memcpy(buffer_out + offset, &net_val, sizeof(uint32_t));
	        offset += sizeof(uint32_t);


	        // F. INVIO RISPOSTA
	        if (sendto(my_socket, buffer_out, offset, 0,
	                   (struct sockaddr *)&cad, sizeof(cad)) != offset)
	        {
	            errorhandler("sendto() failed");
	        }
	    }



	printf("Server terminated.\n");

	closesocket(my_socket);
	clearwinsock();
	return 0;
} // main end
