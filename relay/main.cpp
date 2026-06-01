#include <netinet/in.h>
#include <unistd.h>

#include "hproto.h"
#include "hproto_router.h"

static void send_redirect(int sock, const sockaddr_in &to, const sockaddr_in &peer) {
    RouterRedirectAnswer answer {
        ntohl(peer.sin_addr.s_addr),
        ntohs(peer.sin_port)
    };
    std::variant<RouterRedirectAnswer> v = answer;

    char buf[sizeof(hproto_id_t) + sizeof(RouterRedirectAnswer)];
    hproto_write(v, buf);
    sendto(sock, buf, sizeof(buf), 0,
           reinterpret_cast<const sockaddr *>(&to), sizeof(to));
}

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        hLog() << "Cannot create socket";
        return 1;
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port   = htons(17171),
        .sin_addr = { .s_addr = INADDR_ANY }
    };
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        hLog() << "Cannot bind socket to port";
        close(sockfd);
        return 1;
    }

    std::optional<sockaddr_in> waiting;

    while (true) {
        char buffer[1500];
        struct sockaddr_in from;
        socklen_t len = sizeof(from);

        std::cout << "Wating packet" << std::endl;

        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                         (struct sockaddr *) &from, &len);

        std::cout << "Got some packet" << std::endl;

        std::variant var = hproto_read<RouterCreateWaitroomRequest>(buffer, n);
        if (RouterCreateWaitroomRequest* req = std::get_if<RouterCreateWaitroomRequest>(&var)) {
            bool sameAddr = waiting && waiting->sin_addr.s_addr == from.sin_addr.s_addr
                                    && waiting->sin_port == from.sin_port;

            if (waiting.has_value() && !sameAddr) {
                send_redirect(sockfd, from, *waiting);
                send_redirect(sockfd, *waiting, from);
                waiting.reset();
                std::cout << "Redirected peers to each other" << std::endl;
            } else {
                waiting = from;
                std::cout << "Stored waiting peer" << std::endl;
            }
        }
    }

    hLog() << "All good";
}
