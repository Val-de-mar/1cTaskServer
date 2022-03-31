#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    assert(argc > 1);
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_port = htons(atoi(argv[1])),
            .sin_addr =  inet_addr("127.0.0.1")
    };
    sleep(10);
    int bind_res = bind(listen_fd, (struct sockaddr*) &addr, sizeof(addr));
    assert(bind_res==0);

    int listen_result = listen(listen_fd, SOMAXCONN);

    int client_fd = accept(listen_fd, NULL, NULL);

    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
    close(listen_fd);
}
