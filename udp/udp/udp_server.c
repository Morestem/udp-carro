#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
#define TOTAL_PACKETS 200

void send_total_packets(int sockfd, struct sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "%d", TOTAL_PACKETS);
    sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
}

void send_packets(int sockfd, struct sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE];
    for (int i = 0; i < TOTAL_PACKETS; i++) {
        sprintf(buffer, "Pacote %d", i + 1);
        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
        usleep(100000);
    }
    printf("%d pacotes enviados.\n", TOTAL_PACKETS);
}

void send_missing_packets(int sockfd, struct sockaddr_in client_addr, char* missing_packets) {
    char* token = strtok(missing_packets, ",");
    while (token != NULL) {
        char buffer[BUFFER_SIZE];
        sprintf(buffer, "Pacote %s", token);
        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
        token = strtok(NULL, ",");
    }
    printf("Pacotes solicitados enviados: %s\n", missing_packets);
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(client_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Erro ao criar socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro ao fazer bind");
        close(sockfd);
        return 1;
    }

    printf("Servidor UDP ouvindo na porta %d\n", SERVER_PORT);

    while (1) {
        recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &addr_len);
        printf("Mensagem recebida de %s:%d: %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);
        send_total_packets(sockfd, client_addr);
        send_packets(sockfd, client_addr);

        while (1) {
            recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &addr_len);
            buffer[strcspn(buffer, "\n")] = '\0';
            if (strcmp(buffer, "sair") == 0) {
                printf("Conexão com %s:%d encerrada.\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                break;
            }
            send_missing_packets(sockfd, client_addr, buffer);
        }
    }

    close(sockfd);
    return 0;
}