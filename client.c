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

struct Args {
    int port;
    char *ip;
};

struct Args parseArgs(int argnum, char *argv[]) {
    struct Args ans = {.port = -1, .ip = "127.0.0.1"};
    for (int i = 0; i < argnum; ++i) {
        if (argv[i][0] == '-') {
            if (i + 1 >= argnum) {
                fprintf(stderr, "no argument for %s\n", argv[i]);
                exit(1);
            }
            if (strcmp(argv[i], "-p") == 0) {
                ans.port = atoi(argv[i + 1]);
            } else if (strcmp(argv[i], "-i") == 0) {
                ans.ip = argv[i + 1];
                if (strcmp(ans.ip, "localhost") == 0) {
                    ans.ip = "127.0.0.1";
                }
            } else {
                fprintf(stderr, "incorrect argument %s\n", argv[i]);
                exit(1);
            }
        }
    }

    if (ans.port < 0) {
        fprintf(stderr, "port is required\n");
        exit(1);
    }

    return ans;
}


int main(int argc, char *argv[]) {

    setSignalsProperties();
    struct Args args = parseArgs(argc - 1, argv + 1);

    in_addr_t ip = inet_addr(args.ip);
    uint16_t port = htons(args.port);

    int soc = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in ipv4_addr = {
            .sin_family = AF_INET, .sin_port = port, .sin_addr = ip};

    int connect_res = connect(soc, (struct sockaddr *) &ipv4_addr, sizeof ipv4_addr);
    if (connect_res == -1) {
        fprintf(stderr, "can connect server");
        return 1;
    }

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