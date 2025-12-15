/*
 * main.c
 *
 * UDP Client - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a UDP client
 * portable across Windows, Linux, and macOS.
 */

#if defined WIN32
#include <winsock.h>
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
#include <ctype.h>
#include "protocol.h"

// Correzione Warning NO_ERROR: Lo definiamo solo se non esiste già
#ifndef NO_ERROR
#define NO_ERROR 0
#endif

// Correzione Error INET_ADDRSTRLEN: Lo definiamo se manca (su Windows/Winsock1)
#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif

void errorhandler(char *errorMessage)
{
    printf("%s\n", errorMessage);
}

void clearwinsock()
{
#if defined WIN32
    WSACleanup();
#endif
}

int main(int argc, char *argv[])
{
    weather_request_t request;
    weather_response_t response;

    memset(&request, 0, sizeof(request));
    memset(&response, 0, sizeof(response));

#if defined WIN32
    // Initialize Winsock
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
    if (result != NO_ERROR) {
        printf("Error at WSAStartup()\n");
        return 0;
    }
#endif

    int port = SERVER_PORT;
    char server_address_string[64] = DEFAULT_IP;
    char request_string[128] = "";
    int richiesta = 0;

    // --- PARSING ARGOMENTI ---
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            strncpy(server_address_string, argv[++i], 63);
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            strncpy(request_string, argv[++i], 127);
            richiesta = 1;
        }
    }

    if (!richiesta) {
        printf("Uso: %s [-s server] [-p port] -r \"tipo città\"\n", argv[0]);
        clearwinsock();
        return -1;
    }

    // --- PARSING RICHIESTA ---
    int i = 0;
    while (request_string[i] == ' ' && request_string[i] != '\0') i++;

    request.type = request_string[i];

    if (request_string[i+1] != ' ' && request_string[i+1] != '\0') {
         printf("Errore: Il tipo deve essere un singolo carattere.\n");
         clearwinsock();
         return -1;
    }
    i++;

    while (request_string[i] == ' ' && request_string[i] != '\0') i++;

    if (strlen(&request_string[i]) >= 64) {
        printf("Errore: Nome città troppo lungo (max 63 caratteri).\n");
        clearwinsock();
        return -1;
    }
    strncpy(request.city, &request_string[i], 63);

    // --- CREAZIONE SOCKET UDP ---
    int my_socket;
    my_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (my_socket < 0) {
        errorhandler("Creazione del socket fallita.\n");
        clearwinsock();
        return -1;
    }

    // --- RISOLUZIONE DNS (Forward) ---
    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_port = htons(port);

    struct hostent *host;
    host = gethostbyname(server_address_string);

    if (host == NULL) {
        errorhandler("Risoluzione DNS fallita (host non trovato).\n");
        closesocket(my_socket);
        clearwinsock();
        return -1;
    }

    memcpy(&sad.sin_addr, host->h_addr_list[0], host->h_length);

    // --- SERIALIZZAZIONE MANUALE ---
    char buffer_out[BUFFER_SIZE];
    int offset = 0;

    memcpy(buffer_out + offset, &request.type, sizeof(char));
    offset += sizeof(char);

    int city_len = strlen(request.city) + 1;
    memcpy(buffer_out + offset, request.city, city_len);
    offset += city_len;

    // --- INVIO ---
    if (sendto(my_socket, buffer_out, offset, 0,
               (struct sockaddr *)&sad, sizeof(sad)) != offset)
    {
        errorhandler("Error sending data");
        closesocket(my_socket);
        clearwinsock();
        return -1;
    }

    // --- RICEZIONE ---
    char buffer_in[BUFFER_SIZE];
    struct sockaddr_in from_addr;
    int from_len = sizeof(from_addr);
    int rcv_msg_size;

    if ((rcv_msg_size = recvfrom(my_socket, buffer_in, BUFFER_SIZE, 0,
                                 (struct sockaddr *)&from_addr, &from_len)) < 0)
    {
        errorhandler("Error receiving data");
        closesocket(my_socket);
        clearwinsock();
        return -1;
    }

    // --- DESERIALIZZAZIONE MANUALE ---
    offset = 0;

    uint32_t net_status;
    memcpy(&net_status, buffer_in + offset, sizeof(uint32_t));
    response.status = ntohl(net_status);
    offset += sizeof(uint32_t);

    memcpy(&response.type, buffer_in + offset, sizeof(char));
    offset += sizeof(char);

    uint32_t net_val;
    memcpy(&net_val, buffer_in + offset, sizeof(uint32_t));
    net_val = ntohl(net_val);
    memcpy(&response.value, &net_val, sizeof(float));

    // --- OUTPUT CON REVERSE DNS (Compatibile Winsock 1) ---

    char host_name[256];
    char server_ip_str[INET_ADDRSTRLEN];

    // 1. Convertiamo IP in stringa usando inet_ntoa (Vecchio standard, compatibile)
    char *temp_ip = inet_ntoa(sad.sin_addr);
    strncpy(server_ip_str, temp_ip, INET_ADDRSTRLEN);

    // 2. Reverse DNS usando gethostbyaddr (Vecchio standard, compatibile)
    struct hostent *he;
    he = gethostbyaddr((char *)&sad.sin_addr, sizeof(sad.sin_addr), AF_INET);

    if (he != NULL) {
        strcpy(host_name, he->h_name);
    } else {
        strcpy(host_name, server_ip_str); // Fallback su IP se DNS fallisce
    }

    printf("Ricevuto risultato dal server %s (ip %s). ", host_name, server_ip_str);

    if (response.status == 0) {
        request.city[0] = toupper(request.city[0]);
        printf("%s: ", request.city);

        switch (response.type) {
            case 't': printf("Temperatura = %.1f°C\n", response.value); break;
            case 'h': printf("Umidità = %.1f%%\n", response.value); break;
            case 'w': printf("Vento = %.1f km/h\n", response.value); break;
            case 'p': printf("Pressione = %.1f hPa\n", response.value); break;
            default:  printf("Tipo sconosciuto ricevuto.\n");
        }
    } else if (response.status == 1) {
        printf("Città non disponibile\n");
    } else if (response.status == 2) {
        printf("Richiesta non valida\n");
    } else {
        printf("Errore sconosciuto\n");
    }

	closesocket(my_socket);

	printf("Client terminated.\n");

	clearwinsock();
	return 0;
} // main end
