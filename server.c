#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <assert.h>
#include <limits.h>

#include "DynamicBuffer.h"
#include "AnarchyList.h"


static sig_atomic_t terminate = 0;

void deblock(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFD) | O_NONBLOCK);
}

void handler(int signum) {
    terminate = 1;
}


typedef struct ClientInfoStruct {
    DynamicBuffer buffer;
    AnarchyList *me;
    size_t number;
    int fd;
} ClientInfo;

enum ClientResult {
    OkClient,
    ClosedClient,
    ErrorClient,
};

enum ClientResult clientProcess(ClientInfo *info, int out_fd) {
    ReadResult line_result = readLine(info->fd, &info->buffer);

    switch (line_result) {
        case EndOfFileRead:
            if (info->buffer.size != 0) {
                info->buffer.data[info->buffer.size] = '\0';
                if (dprintf(out_fd, "%zu - %s\n", info->number, info->buffer.data) < 1) {
                    fprintf(stderr, "Error with writing to file");
                }
                bufferCut(&info->buffer, info->buffer.size);
            }
            return ClosedClient;
        case LineRead: {
            char *newline_char = strchr(info->buffer.data, '\n');
            size_t place = newline_char - info->buffer.data + 1;
            char save = info->buffer.data[place];
            info->buffer.data[place] = '\0';
            if (dprintf(out_fd, "%zu - %s", info->number, info->buffer.data) < 1) {
                fprintf(stderr, "Error with writing to file");
            }

            info->buffer.data[place] = save;
            bufferCut(&info->buffer, place);
            return OkClient;
        }
        case WaitingRead:
            return OkClient;
        case ErrorRead:
            return ErrorClient;
    }

}

void setSignalsProperties() {
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = 0;
    sigaction(SIGTERM, &act, 0);
    sigaction(SIGINT, &act, 0);

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGTERM);
    sigdelset(&mask, SIGINT);
    sigprocmask(SIG_SETMASK, &mask, NULL);
}

int startListen(int port) {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in socket_ipv4_address = {.sin_family = AF_INET,
            .sin_port = htons(port),
            .sin_addr =
                    {inet_addr("127.0.0.1")}};


    int bind_result = bind(
            listen_fd,
            (struct sockaddr *) &socket_ipv4_address,
            sizeof(socket_ipv4_address));

    if (bind_result == -1) {
        close(listen_fd);
        fprintf(stderr, "bind error");
        return -1;
    }


    int listen_result = listen(listen_fd, SOMAXCONN);
    if (listen_result == -1) {
        close(listen_fd);
        fprintf(stderr, "listen error");
        return -1;
    }
    return listen_fd;
}

int epoll_acept(int epoll_fd, ClientInfo *listen_info, int free_space) {
    int listen_fd = listen_info->fd;
    static int clients_received = 0;
    int working = 0;
    int client_fd = accept(listen_fd, NULL, NULL);
    while (client_fd != -1) {
        ++working;

        deblock(client_fd);
        ClientInfo *new_client_info = malloc(sizeof(ClientInfo));
        new_client_info->fd = client_fd;
        new_client_info->number = clients_received;
        new_client_info->me = emplaceAnarchyAfter(listen_info->me, new_client_info);
        bufferInit(&new_client_info->buffer);
        ++clients_received;
        struct epoll_event client_event = {.events=EPOLLIN, .data.ptr=new_client_info};
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event);

        if (working >= free_space) {
            return working;
        }

        client_fd = accept(listen_fd, NULL, NULL);
    }
    return working;
}

void epollIteration(int epoll_fd, int event_arr_size, ClientInfo *listen_info, int users_limit) {
    static int working = 0;
    int listen_fd = listen_info->fd;

    struct epoll_event catched[event_arr_size];
    int to_process = epoll_wait(epoll_fd, catched, event_arr_size, -1);
    for (int i = 0; i < to_process; ++i) {
        if (!(catched[i].events & EPOLLIN)) {
            continue;
        }

        ClientInfo *info = (ClientInfo *) catched[i].data.ptr;

        if (info->fd == listen_fd) {
            working += epoll_acept(epoll_fd, info, users_limit - working);
            if (working >= users_limit) {
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, listen_info->fd, NULL);
            }
            continue;
        }

        switch (clientProcess(info, STDOUT_FILENO)) {
            case ErrorClient: {
                fprintf(stderr, "Error with client %zu, it will be shut down\n", info->number);
            }
            case ClosedClient:
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, info->fd, NULL);
                shutdown(info->fd, SHUT_RDWR);
                close(info->fd);
                bufferDestroy(&info->buffer);
                destroyHeapAnarchyList(info->me);
                free(info);
                --working;
                if (working < users_limit) {
                    struct epoll_event socket_event = {.events=EPOLLIN, .data.ptr=listen_info};
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &socket_event);
                }
                break;
            case OkClient:
                break;
        }
    }
}


int main(int argc, char *argv[]) {
    if (argc <= 1) {
        fprintf(stderr, "Error port is required");
        return 1;
    }

    int out_fd = STDOUT_FILENO;
    if (argc > 2) {
        out_fd = open(argv[2], O_WRONLY);
    }

    int limit = 5;
    if (argc > 2) {
        limit = atoi(argv[3]);
        if (limit == -1) {
            limit = INT_MAX;
        }
    }

    setSignalsProperties();

    int listen_fd = startListen(atoi(argv[1]));
    if (listen_fd == -1) {
        return 1;
    }


    int epoll_fd = epoll_create1(0);
    deblock(listen_fd);
    AnarchyList begin = {.value = NULL, .prev = NULL, .next = NULL};
    ClientInfo listen_info = {.fd = listen_fd, .me=&begin};
    begin.value = &listen_info;

    struct epoll_event socket_event = {.events=EPOLLIN, .data.ptr=&listen_info};
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &socket_event);
    const int event_arr_size = 28;

    size_t clients_received = 0;

    while (terminate == 0) {
        epollIteration(epoll_fd, event_arr_size, &listen_info, limit);
    }
    for (AnarchyList *cur = listen_info.me->next; cur != NULL;) {
        AnarchyList *next = cur->next;
        ClientInfo *info = ((ClientInfo *) cur->value);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, info->fd, NULL);
        shutdown(info->fd, SHUT_RDWR);
        close(info->fd);
        bufferDestroy(&info->buffer);
        destroyHeapAnarchyList(cur);
        free(info);
        cur = next;
    }

    close(epoll_fd);
    close(listen_fd);
    return 0;
}