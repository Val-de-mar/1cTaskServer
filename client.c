#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <signal.h>

#define MIN(a, b) ((a) > (b)? (b) : (a))


const static size_t BUF_MAX = 4096;

static sig_atomic_t flag = 1;

void handler(int signum) {
    flag = 0;
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


int main(int argc, char *argv[]) {
    assert(argc > 3);
    in_addr_t ip = inet_addr(argv[1]);
    uint16_t port = htons(atoi(argv[2]));

    int soc = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in ipv4_addr = {
            .sin_family = AF_INET, .sin_port = port, .sin_addr = ip};

    int connect_res = connect(soc, (struct sockaddr *) &ipv4_addr, sizeof ipv4_addr);

    char buf[BUF_MAX];
    while (flag && fgets(buf, BUF_MAX - 1, stdin) != NULL) {
        size_t remain = strlen(buf);
        while (remain > 0) {
            int res = write(soc, buf, MIN(remain, SSIZE_MAX));
            if (res <= 0) {
                flag = 0;
                fprintf(stderr, "can't write to server");
                break;
            }
            remain -= res;
        }
    }

    shutdown(soc, SHUT_RDWR);


    return 0;
}