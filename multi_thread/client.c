#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8765
#define BUFFER_SIZE 1024

int main()
{
    int soc;
    struct sockaddr_in serverAddr;

    // ソケットを作成
    if ((soc = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("[-]socket()");
        exit(1);
    }

    // サーバーの情報を設定
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &(serverAddr.sin_addr)) <= 0)
    {
        perror("[-]inet_pton()");
        exit(1);
    }

    // サーバーに接続
    if (connect(soc, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("[-]connect()");
        exit(1);
    }

    char response[BUFFER_SIZE];
    char input[BUFFER_SIZE];
    while (1)
    {
        // サーバーからの応答を受け取り、標準出力に出力
        memset(response, 0, sizeof(response));
        if (recv(soc, response, sizeof(response) - 1, 0) < 0)
        {
            perror("[-]recv()");
            exit(1);
        }
        printf("%s", response);

        // 標準入力からデータを読み取り、サーバーに送信
        fgets(input, sizeof(input), stdin);
        if (send(soc, input, strlen(input), 0) < 0)
        {
            perror("[-]send()");
            exit(1);
        }
    }

    // ソケットを閉じる
    close(soc);

    return 0;
}