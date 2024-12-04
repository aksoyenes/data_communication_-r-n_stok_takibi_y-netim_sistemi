#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

// Ürün veri yapýsý
typedef struct {
    char name[50];
    int quantity;
    float price;
} Product;

Product products[100];
int product_count = 0;

// Ürün ekleme fonksiyonu
void add_product(const char* name, int quantity, float price) {
    strcpy(products[product_count].name, name);
    products[product_count].quantity = quantity;
    products[product_count].price = price;
    product_count++;
}

// Ürün listeleme fonksiyonu
char* list_products() {
    static char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    int i;
    for (i = 0; i < product_count; i++) {
        char line[100];
        sprintf(line, "%s - %d units - $%.2f\n", products[i].name, products[i].quantity, products[i].price);
        strcat(buffer, line);
    }
    return buffer;
}

// Ürün sorgulama fonksiyonu
char* query_product(const char* name) {
    static char buffer[100];
    int i;
    for (i = 0; i < product_count; i++) {
        if (strcmp(products[i].name, name) == 0) {
            sprintf(buffer, "%s has %d units - $%.2f each", name, products[i].quantity, products[i].price);
            return buffer;
        }
    }
    return "Product not found";
}

// Ürün çýkarma fonksiyonu
void remove_product(const char* name, int quantity) {
    int i;
    for (i = 0; i < product_count; i++) {
        if (strcmp(products[i].name, name) == 0) {
            products[i].quantity -= quantity;
            if (products[i].quantity < 0) products[i].quantity = 0;
            return;
        }
    }
}

// Ýstemciyi iþleyen fonksiyon
DWORD WINAPI handle_client(LPVOID socket_desc) {
    int client_sock = *(int*)socket_desc;
    char buffer[BUFFER_SIZE];
    int read_size;

    while ((read_size = recv(client_sock, buffer, sizeof(buffer), 0)) > 0) {
        buffer[read_size] = '\0'; // Gelen veriyi düzgün sonlandýr
        char response[BUFFER_SIZE] = "Invalid command";

        if (strncmp(buffer, "LIST", 4) == 0) {
            strcpy(response, list_products());
        } else if (strncmp(buffer, "ADD", 3) == 0) {
            char name[50];
            int quantity;
            float price;
            sscanf(buffer + 4, "%s %d %f", name, &quantity, &price);
            add_product(name, quantity, price);
            sprintf(response, "Product %s added with %d units at $%.2f each", name, quantity, price);
        } else if (strncmp(buffer, "REMOVE", 6) == 0) {
            char name[50];
            int quantity;
            sscanf(buffer + 7, "%s %d", name, &quantity);
            remove_product(name, quantity);
            sprintf(response, "Removed %d units from %s", quantity, name);
        } else if (strncmp(buffer, "QUERY", 5) == 0) {
            char name[50];
            sscanf(buffer + 6, "%s", name);
            strcpy(response, query_product(name));
        }

        send(client_sock, response, strlen(response), 0);
    }

    closesocket(client_sock);
    free(socket_desc);
    return 0;
}

int main() {
    WSADATA wsa;
    int server_sock, client_sock, *new_sock;
    struct sockaddr_in server, client;
    int client_size = sizeof(client); // socklen_t yerine int kullandýk

    // Windows Sockets API baþlatma
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup hatasý. Hata kodu: %d\n", WSAGetLastError());
        return 1;
    }

    // Socket oluþturma
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) {
        printf("Socket oluþturma hatasý. Hata kodu: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // Socket'i baðlama
    if (bind(server_sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind hatasý. Hata kodu: %d\n", WSAGetLastError());
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    // Dinlemeye baþlama
    if (listen(server_sock, MAX_CLIENTS) == SOCKET_ERROR) {
        printf("Dinleme hatasý. Hata kodu: %d\n", WSAGetLastError());
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    printf("Sunucu dinleniyor...\n");

    // Ýstemcileri kabul etme
    while ((client_sock = accept(server_sock, (struct sockaddr*)&client, &client_size))) {
        printf("Yeni istemci baðlandý.\n");

        // Yeni iþ parçacýðý oluþturma
        new_sock = malloc(sizeof(int));
        *new_sock = client_sock;
        CreateThread(NULL, 0, handle_client, (LPVOID)new_sock, 0, NULL);
    }

    closesocket(server_sock);
    WSACleanup();
    return 0;
}


