#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8765
#define MAX_MESSAGE_LENGTH 100
#define MAX_CLIENTS 10

struct socketInfo
{
    int sock;
    struct sockaddr_in addr;
};

pthread_mutex_t mutex;
char messageBoard[MAX_MESSAGE_LENGTH];
int numMessages = 0;

int server_socket(void);
void *clientHandler(void *arg);
void postMessage(int clientSocket);
void getMessages(int clientSocket);

int server_socket(void)
{
    int soc, opt = 1;
    struct sockaddr_in addr;

    if ((soc = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("[-]socket()");
        exit(1);
    }

    if (setsockopt(soc, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("[-]setsockopt()");
        close(soc);
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(soc, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("[-]bind()");
        close(soc);
        exit(1);
    }

    if (listen(soc, MAX_CLIENTS) == -1)
    {
        perror("[-]listen()");
        close(soc);
        exit(1);
    }

    return soc;
}

void *clientHandler(void *arg)
{
    int clientSocket = ((struct socketInfo *)arg)->sock;
    struct sockaddr_in clientAddr = ((struct socketInfo *)arg)->addr;
    char choice[2];

    // メニューを表示
    char menu[] = "1. Post a message\n2. Get messages\nEnter your choice: ";
    send(clientSocket, menu, strlen(menu), 0);
    recv(clientSocket, choice, sizeof(choice), 0);

    // 選択に応じて処理を実行
    if (choice[0] == '1')
    {
        postMessage(clientSocket);
    }
    else if (choice[0] == '2')
    {
        getMessages(clientSocket);
    }
    else
    {
        char invalidChoice[] = "Invalid choice";
        send(clientSocket, invalidChoice, strlen(invalidChoice), 0);
    }

    close(clientSocket);
    printf("[+]Closed connection from %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    pthread_exit(NULL);
}

void postMessage(int clientSocket)
{
    char enterMessage[] = "Enter your message: ";
    send(clientSocket, enterMessage, strlen(enterMessage), 0);

    pthread_mutex_lock(&mutex);
    memset(messageBoard, 0, sizeof(messageBoard));
    recv(clientSocket, messageBoard, sizeof(messageBoard)-1, 0);
    pthread_mutex_unlock(&mutex);

    char successMessage[] = "Message posted successfully";
    send(clientSocket, successMessage, strlen(successMessage), 0);
}

void getMessages(int clientSocket)
{

    pthread_mutex_lock(&mutex);
    send(clientSocket, messageBoard, strlen(messageBoard), 0);
    pthread_mutex_unlock(&mutex);
}

int main(void)
{
    int soc, clientSocket;
    struct sockaddr_in clientAddr;
    struct socketInfo client;
    socklen_t clientLen;
    pthread_t thread_id;

    soc = server_socket();
    pthread_mutex_init(&mutex, NULL);

    while (1)
    {
        clientLen = sizeof(clientAddr);
        if ((clientSocket = accept(soc, (struct sockaddr *)&clientAddr, &clientLen)) == -1)
        {
            perror("[-]accept()");
            continue;
        }

        printf("[+]Accept: %s (%d)\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        client.sock = clientSocket;
        client.addr = clientAddr;

        if (pthread_create(&thread_id, NULL, clientHandler, (void *)&client) != 0)
        {
            perror("[-]pthread_create()");
        }
        else
        {
            printf("[+]pthread_created: thread_id = %d\n", (int)thread_id);
        }
    }

    close(soc);
    pthread_mutex_destroy(&mutex);
    return 0;
}