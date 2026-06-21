#include <netinet/in.h>
#include <unistd.h>

#include "hproto.h"
#include "hlog.h"
#include "hproto_router.h"
#include "hproto_types.h"
#include "hudpchannel.h"

static void send_redirect(HUdpChannel *chan, HSocketAddress peer) {
    RouterRedirectAnswer answer { peer };
    std::variant<RouterRedirectAnswer> v = answer;

    char buf[sizeof(hproto_id_t) + sizeof(RouterRedirectAnswer)];
    hproto_write(v, buf);
    chan->write(buf, sizeof(buf));
}

struct SessionData {
    HUdpChannel *channel;
    bool isAwaitingPeer = false;
};

int main() {
    hLog() << "Starting hotspot relay server";

    std::map<HSocketAddress, SessionData> sessions;

    HUdpServer server(INADDR_ANY, 17171, [&sessions](HUdpChannel *channel) -> Task {
        sessions[channel->peer()] = {channel};

        hLog() << "Got new peer";

        char buffer[1500];

        while (true) {
            ssize_t n = co_await channel->read(buffer, sizeof(buffer));
            if (n < 0)
                continue;

            std::variant var = hproto_read<RouterCreateWaitroomRequest>(buffer, n);
            if (RouterCreateWaitroomRequest* req = std::get_if<RouterCreateWaitroomRequest>(&var)) {
                HUdpChannel *other = nullptr;
                for (auto& [sender, candidate] : sessions) {
                    if (sender == channel->peer()
                        || !candidate.isAwaitingPeer)
                        continue;

                    other = candidate.channel;
                    break;
                }

                if (!other) {
                    sessions[channel->peer()].isAwaitingPeer = true;
                    continue;
                }

                send_redirect(other, channel->peer());
                send_redirect(channel, other->peer());
                sessions.erase(other->peer());
                sessions.erase(channel->peer());
                other->close();
                channel->close();
                hLog() << "Redirected peers to each other";
            }
        }
    });

    if (!server.isValid())
        return 1;

    while (true)
        if (!server.processDatagram())
            break;
}
