#include <arpa/inet.h>
#include <cstdio>
#include <cstdint>
#include <netinet/in.h>
#include <optional>
#include <sys/socket.h>
#include <unistd.h>
#include <variant>

#include "hproto.h"
#include "hproto_types.h"

static constexpr uint16_t SERVER_PORT    = 17171;
static constexpr size_t   MAX_PACKET     = 1500;

static void send_redirect(int sock, const sockaddr_in &to, const sockaddr_in &peer) {
    // Addresses in the struct are host byte order to match QHostAddress expectations
    RouterRedirectAnswer answer{ntohl(peer.sin_addr.s_addr), ntohs(peer.sin_port), 0};
    std::variant<RouterRedirectAnswer> v = answer;

    uint8_t buf[sizeof(hproto_id_t) + sizeof(RouterRedirectAnswer)];
    hproto_write(v, buf);
    sendto(sock, buf, sizeof(buf), 0,
           reinterpret_cast<const sockaddr *>(&to), sizeof(to));
}

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    sockaddr_in bind_addr{};
    bind_addr.sin_family      = AF_INET;
    bind_addr.sin_port        = htons(SERVER_PORT);
    bind_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, reinterpret_cast<sockaddr *>(&bind_addr), sizeof(bind_addr)) < 0) {
        perror("bind"); close(sock); return 1;
    }

    printf("hServer listening on port %d\n", SERVER_PORT);

    std::optional<sockaddr_in> waiting;
    uint8_t buf[MAX_PACKET];

    for (;;) {
        sockaddr_in from{};
        socklen_t from_len = sizeof(from);
        ssize_t len = recvfrom(sock, buf, sizeof(buf), 0,
                               reinterpret_cast<sockaddr *>(&from), &from_len);
        if (len < 0) { perror("recvfrom"); continue; }

        printf("Datagram from %s:%d\n", inet_ntoa(from.sin_addr), ntohs(from.sin_port));

        auto var = hproto_read<RouterCreateWaitroomRequest>(buf, static_cast<size_t>(len));
        if (!std::get_if<RouterCreateWaitroomRequest>(&var))
            continue;

        printf("RouterCreateWaitroomRequest received\n");

        auto same_addr = [&](const sockaddr_in &a) {
            return a.sin_addr.s_addr == from.sin_addr.s_addr
                && a.sin_port        == from.sin_port;
        };

        if (waiting.has_value() && !same_addr(*waiting)) {
            send_redirect(sock, from, *waiting);
            send_redirect(sock, *waiting, from);
            waiting.reset();
            printf("Redirected peers to each other\n");
        } else {
            waiting = from;
            printf("Stored waiting peer\n");
        }
    }

    close(sock);
}
