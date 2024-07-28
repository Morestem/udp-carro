#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int total_packets = 0;
int timeout_reached = 0;
fd_set read_fds;

void* timeout_handler(void* arg) {
    sleep(20);
    timeout_reached = 1;
    return NULL;
}

void receive_total_packets(int sockfd, struct sockaddr_in server_addr) {
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(server_addr);
    recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&server_addr, &addr_len);
    buffer[strcspn(buffer, "\n")] = '\0';
    total_packets = atoi(buffer);
    printf("Total de pacotes a serem recebidos: %d\n", total_packets);
}

void receive_packets(int sockfd, struct sockaddr_in server_addr, int* received_packets) {
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(server_addr);
    timeout_reached = 0;
    pthread_t timer_thread;
    pthread_create(&timer_thread, NULL, timeout_handler, NULL);

    while (!timeout_reached) {
        struct timeval tv = {1, 0};
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        select(sockfd + 1, &read_fds, NULL, NULL, &tv);

        if (FD_ISSET(sockfd, &read_fds)) {
            int bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&server_addr, &addr_len);
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                printf("Recebido: %s\n", buffer);
                int packet_num = atoi(buffer + 7);
                received_packets[packet_num - 1] = 1;
            }
        }
    }
    pthread_cancel(timer_thread);
}

void get_missing_packets(int* received_packets, int* missing_packets, int* missing_count) {
    *missing_count = 0;
    for (int i = 0; i < total_packets; i++) {
        if (!received_packets[i]) {
            missing_packets[(*missing_count)++] = i + 1;
        }
    }
    printf("Pacotes faltando: ");
    for (int i = 0; i < *missing_count; i++) {
        printf("%d ", missing_packets[i]);
    }
    printf("\n");
}

void request_missing_packets(int sockfd, struct sockaddr_in server_addr, int* missing_packets, int missing_count) {
    char buffer[BUFFER_SIZE];
    for (int i = 0; i < missing_count; i++) {
        if (i == 0) {
            sprintf(buffer, "%d", missing_packets[i]);
        } else {
            sprintf(buffer + strlen(buffer), ",%d", missing_packets[i]);
        }
    }
    sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int received_packets[200] = {0};

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Erro ao criar socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    while (1) {
        printf("Digite 'iniciar' para solicitar pacotes ao servidor: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "sair") == 0) {
            printf("Encerrando o cliente.\n");
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
            break;
        }

        sendto(sockfd, "Iniciar", 7, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        receive_total_packets(sockfd, server_addr);
        receive_packets(sockfd, server_addr, received_packets);

        int missing_packets[200];
        int missing_count;
        get_missing_packets(received_packets, missing_packets, &missing_count);

        while (missing_count > 0) {
            request_missing_packets(sockfd, server_addr, missing_packets, missing_count);
            receive_packets(sockfd, server_addr, received_packets);
            get_missing_packets(received_packets, missing_packets, &missing_count);
        }

        printf("Todos os pacotes foram recebidos.\n");
    }

    close(sockfd);
    return 0;
}