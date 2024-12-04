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

    // Winsock baþlatýlýyor
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Winsock baþlatýlamadý. Hata kodu: %d\n", WSAGetLastError());
        return 1;
    }

    // Sunucuya baðlanmak için soket oluþturuluyor
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Socket oluþturulamadý. Hata kodu: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Sunucu yapýlandýrmasý
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Sunucuya baðlanýlýyor
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Baðlantý baþarýsýz. Hata kodu: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Sunucuya baðlandý.\n");

    // Kullanýcýdan komut alýyoruz ve gönderiyoruz
    while (1) {
        printf("> ");
        fgets(message, BUFFER_SIZE, stdin);
        message[strcspn(message, "\n")] = 0; // Yeni satýr karakterini kaldýr

        // Sunucuya komut gönderme
        if (send(sock, message, strlen(message), 0) == SOCKET_ERROR) {
            printf("Komut gönderilemedi. Hata kodu: %d\n", WSAGetLastError());
            break;
        }

        // Sunucudan yanýt alýnýyor
        memset(server_reply, 0, sizeof(server_reply));  // Yanýtý temizliyoruz
        int read_size = recv(sock, server_reply, sizeof(server_reply) - 1, 0);
        if (read_size == SOCKET_ERROR) {
            printf("Sunucudan yanýt alýnamadý. Hata kodu: %d\n", WSAGetLastError());
            break;
        } else if (read_size == 0) {
            printf("Sunucu baðlantýyý kapattý.\n");
            break;
        }

        server_reply[read_size] = '\0';  // Yanýtý sonlandýr
        printf("Sunucudan yanýt: %s\n", server_reply);
    }

    // Soketi kapat
    closesocket(sock);

    // Winsock sonlandýrma
    WSACleanup();
    return 0;
}

