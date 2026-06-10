#include <netinet/in.h>
#include <unistd.h>

#include "hproto.h"
#include "hlog.h"
#include "hproto_router.h"
#include "hudpchannel.h"

static void send_redirect(HUdpChannel &chan, const sockaddr_in &to, const sockaddr_in &peer) {
    RouterRedirectAnswer answer {
        ntohl(peer.sin_addr.s_addr),
        ntohs(peer.sin_port)
    };
    std::variant<RouterRedirectAnswer> v = answer;

    char buf[sizeof(hproto_id_t) + sizeof(RouterRedirectAnswer)];
    hproto_write(v, buf);
    chan.write(buf, sizeof(buf), to);
}

int main() {
    HUdpChannel channel(INADDR_ANY, 17171);
    if (!channel.isValid())
        return 1;

    std::optional<sockaddr_in> waiting;

    while (true) {
        char buffer[1500];
        struct sockaddr_in from;

        hLog() << "Wating packet";

        int n = channel.read(buffer, sizeof(buffer), &from);

        hLog() << "Got some packet";

        std::variant var = hproto_read<RouterCreateWaitroomRequest>(buffer, n);
        if (RouterCreateWaitroomRequest* req = std::get_if<RouterCreateWaitroomRequest>(&var)) {
            bool sameAddr = waiting && waiting->sin_addr.s_addr == from.sin_addr.s_addr
                                    && waiting->sin_port == from.sin_port;

            if (waiting.has_value() && !sameAddr) {
                send_redirect(channel, from, *waiting);
                send_redirect(channel, *waiting, from);
                waiting.reset();
                hLog() << "Redirected peers to each other";
            } else {
                waiting = from;
                hLog() << "Stored waiting peer";
            }
        }
    }

    hLog() << "All good";
}
