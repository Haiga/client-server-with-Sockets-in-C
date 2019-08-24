#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <vector>
#include "json.hpp"

#define PORT 4242
#define BUFFER_LENGTH 4096
#define MAX_MESSAGES 10

int slotsUtilizados = 0;
sem_t semaforoProdutor, semaforoConsumidor;
pthread_mutex_t mutexBuffer;
pthread_mutex_t mutexUsuariosConectados;


using json = nlohmann::json;
char **buffer;
std::vector<int> clientfds;

void montarMensagemParaBuffer(char *line_buffer, const json &mensagem) {
    strcpy(line_buffer, (mensagem.dump() + "\n\0").c_str());
}

long enviar(int clientfd, const char *line_buffer) {
    return send(clientfd, line_buffer, strlen(line_buffer), 0);
}

void excluirUsuario(int clientfd) {
    for (auto iter = clientfds.begin(); iter != clientfds.end(); ++iter) {
        if (*iter == clientfd) {
            clientfds.erase(iter);
            break;
        }
    }
}

void *producer(void *identifier_client) {
    char line_buffer[BUFFER_LENGTH];
    int clientfd = *((int *) identifier_client);
    bool encerrado = false;

    std::string nomeCliente;
    std::string textoMensagem;
    std::string mensagemFormatada;
    json boasVindas;
    json dizerTchau;

    boasVindas["comando"] = "mensagemBoasVindas";
    boasVindas["mensagem"] = "Olá! Seja bem-vindo!";

    dizerTchau["comando"] = "desconectar";
    dizerTchau["mensagem"] = "tchau!";

    montarMensagemParaBuffer(line_buffer, boasVindas);
    if (enviar(clientfd, line_buffer)) {
        json mensagemRecebida;
        while (true) {
            memset(line_buffer, 0x0, BUFFER_LENGTH);
            if ((recv(clientfd, line_buffer, BUFFER_LENGTH, 0)) > 0) {
                mensagemRecebida = json::parse(line_buffer);

                if (mensagemRecebida["comando"] == "conectar") {
                    nomeCliente = mensagemRecebida["apelidoUsuario"];
                    strcpy(line_buffer, (nomeCliente + " entrou na sala de conversa!").c_str());
                } else if (mensagemRecebida["comando"] == "enviarMensagem") {
                    textoMensagem = mensagemRecebida["mensagem"];

                    mensagemFormatada = "";
                    mensagemFormatada.append(nomeCliente);
                    mensagemFormatada.append(" enviou: ");
                    mensagemFormatada.append(textoMensagem);

                    stpcpy(line_buffer, (mensagemFormatada).c_str());
                } else {
                    encerrado = true;
                    montarMensagemParaBuffer(line_buffer, dizerTchau);
                    enviar(clientfd, line_buffer);

                    mensagemFormatada = "";
                    mensagemFormatada.append(nomeCliente);
                    mensagemFormatada.append(" saiu da sala de conversa!");
                    stpcpy(line_buffer, (mensagemFormatada).c_str());
                }

                while (slotsUtilizados == MAX_MESSAGES) {
                    sem_wait(&semaforoConsumidor);
                }

                pthread_mutex_lock(&mutexBuffer);
                strcpy(buffer[slotsUtilizados], line_buffer);
                slotsUtilizados += 1;
                pthread_mutex_unlock(&mutexBuffer);
                sem_post(&semaforoProdutor);

                if (encerrado) {
                    break;
                }
            } else {
                close(clientfd);
                pthread_mutex_lock(&mutexUsuariosConectados);
                excluirUsuario(clientfd);
                pthread_mutex_unlock(&mutexUsuariosConectados);

                pthread_exit(NULL);
            }
        }
    }
    close(clientfd);

    pthread_mutex_lock(&mutexUsuariosConectados);
    excluirUsuario(clientfd);
    pthread_mutex_unlock(&mutexUsuariosConectados);
    pthread_exit(NULL);
}

void *consumer(void *arg) {
    char line_buffer[BUFFER_LENGTH];
    while (true) {
        memset(line_buffer, 0x0, BUFFER_LENGTH);
        while (slotsUtilizados == 0) {
            sem_wait(&semaforoProdutor);
        }
        pthread_mutex_lock(&mutexBuffer);
        strcpy(line_buffer, buffer[slotsUtilizados - 1]);
        slotsUtilizados--;
        pthread_mutex_unlock(&mutexBuffer);
        sem_post(&semaforoConsumidor);

        json mensagemParaEnviar;
        mensagemParaEnviar["comando"] = "respostaServidor";
        mensagemParaEnviar["mensagem"] = line_buffer;
        montarMensagemParaBuffer(line_buffer, mensagemParaEnviar);

        pthread_mutex_lock(&mutexUsuariosConectados);
        for (int clientfd : clientfds) {
            enviar(clientfd, line_buffer);
        }
        pthread_mutex_unlock(&mutexUsuariosConectados);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t tid_consumer;
    sem_init(&semaforoProdutor, 0, 0);
    sem_init(&semaforoConsumidor, 0, 0);

    struct sockaddr_in client{}, server{};
    int serverfd, clientfd;
    fprintf(stdout, "Starting server\n");

    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd == -1) {
        perror("Can't create the server socket:");
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Server socket created with fd: %d\n", serverfd);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    memset(server.sin_zero, 0x0, 8);
    int yes = 1;

    if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("Socket options error:");
        return EXIT_FAILURE;
    }

    if (bind(serverfd, (struct sockaddr *) &server, sizeof(server)) == -1) {
        perror("Socket bind error:");
        return EXIT_FAILURE;
    }

    if (listen(serverfd, 1) == -1) {
        perror("Listen error:");
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Listening on port %d\n", PORT);

    socklen_t client_len = sizeof(client);
    buffer = (char **) calloc(MAX_MESSAGES, sizeof(char *));
    for (int i = 0; i < MAX_MESSAGES; i++) {
        char *buffer_temp = (char *) calloc(BUFFER_LENGTH, sizeof(char));
        buffer[i] = buffer_temp;
    }
    pthread_create(&tid_consumer, NULL, consumer, NULL);
//    fprintf(stdout, "Listening on port %d\n", PORT);
    while (true) {
        if ((clientfd = accept(serverfd, (struct sockaddr *) &client, &client_len)) == -1) {
            perror("Accept error:");
            return EXIT_FAILURE;
        }

        pthread_mutex_lock(&mutexUsuariosConectados);
        clientfds.push_back(clientfd);
        pthread_mutex_unlock(&mutexUsuariosConectados);

        pthread_t new_thread = 0;
        fprintf(stdout, "user on : %d\n", clientfd);
        pthread_create(&new_thread, NULL, producer, &clientfd);
    }
    close(serverfd);
    return EXIT_SUCCESS;
}