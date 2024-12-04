#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server;
    char message[BUFFER_SIZE], server_reply[BUFFER_SIZE];

    // Winsock ba�lat�l�yor
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Winsock ba�lat�lamad�. Hata kodu: %d\n", WSAGetLastError());
        return 1;
    }

    // Sunucuya ba�lanmak i�in soket olu�turuluyor
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Socket olu�turulamad�. Hata kodu: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Sunucu yap�land�rmas�
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Sunucuya ba�lan�l�yor
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Ba�lant� ba�ar�s�z. Hata kodu: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Sunucuya ba�land�.\n");

    // Kullan�c�dan komut al�yoruz ve g�nderiyoruz
    while (1) {
        printf("> ");
        fgets(message, BUFFER_SIZE, stdin);
        message[strcspn(message, "\n")] = 0; // Yeni sat�r karakterini kald�r

        // Sunucuya komut g�nderme
        if (send(sock, message, strlen(message), 0) == SOCKET_ERROR) {
            printf("Komut g�nderilemedi. Hata kodu: %d\n", WSAGetLastError());
            break;
        }

        // Sunucudan yan�t al�n�yor
        memset(server_reply, 0, sizeof(server_reply));  // Yan�t� temizliyoruz
        int read_size = recv(sock, server_reply, sizeof(server_reply) - 1, 0);
        if (read_size == SOCKET_ERROR) {
            printf("Sunucudan yan�t al�namad�. Hata kodu: %d\n", WSAGetLastError());
            break;
        } else if (read_size == 0) {
            printf("Sunucu ba�lant�y� kapatt�.\n");
            break;
        }

        server_reply[read_size] = '\0';  // Yan�t� sonland�r
        printf("Sunucudan yan�t: %s\n", server_reply);
    }

    // Soketi kapat
    closesocket(sock);

    // Winsock sonland�rma
    WSACleanup();
    return 0;
}

