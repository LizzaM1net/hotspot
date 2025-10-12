#include "hotspotchat.h"

#include <hproto.h>

struct RouterCreateWaitroomRequest {
    uint32_t local_ip;
    uint16_t local_port;
};
HOTSPOT_SIZED_OBJECT(RouterCreateWaitroomRequest, 0x1002, 8)

struct RouterRedirectAnswer {
    uint32_t ip;
    uint16_t port;
};
HOTSPOT_SIZED_OBJECT(RouterRedirectAnswer, 0x1003, 8)

static const QByteArray c_bannerGreet = QByteArrayLiteral("👋");
static const QByteArray c_bannerRedirect = QByteArrayLiteral("↩");

HotspotChat::HotspotChat(QObject *parent)
    : QUdpSocket{parent}
{
    connect(this, &QAbstractSocket::stateChanged,
            this, &HotspotChat::connectedChanged);
    connect(this, &QIODevice::channelReadyRead,
            this, &HotspotChat::readyRead);

    bind(QHostAddress::Any);
    m_port = localPort();
}

HotspotChat::~HotspotChat() {
    disconnect(this, &QAbstractSocket::stateChanged,
               this, &HotspotChat::connectedChanged);
    disconnect(this, &QIODevice::channelReadyRead,
               this, &HotspotChat::readyRead);
}

quint16 HotspotChat::port() {
    return m_port;
}

QUrl HotspotChat::url() const {
    return m_url;
}

void HotspotChat::setUrl(const QUrl &newUrl) {
    if (m_url == newUrl)
        return;

    m_url = newUrl;
    emit urlChanged();

    quint32 addr = localAddress().toIPv4Address();

    std::variant<RouterCreateWaitroomRequest> var = RouterCreateWaitroomRequest{addr, localPort()};
    QByteArray arr(hproto_size(var), Qt::Uninitialized);
    hproto_write(var, arr.data());

    writeDatagram(arr.data(), arr.size(), QHostAddress(m_url.host()), m_url.port());
}

bool HotspotChat::connected() {
    return m_connectionState == ConnectedToPeer;
}

QVariantList HotspotChat::messages() const {
    return m_messages;
}

void HotspotChat::send(QString text) {
    std::variant<std::string> var = text.toStdString();
    QByteArray arr(hproto_size(var), Qt::Uninitialized);
    hproto_write(var, arr.data());

    writeDatagram(arr, QHostAddress(m_url.host()), m_url.port());

    m_messages.append(QVariantMap({{"from", "local"}, {"text", text}}));
    emit messagesChanged();
}

void HotspotChat::greetAddress(QUrl url) {
    writeDatagram(c_bannerGreet, QHostAddress(url.host()), url.port());
    writeDatagram(c_bannerRedirect, QHostAddress(url.host()), url.port());
}

HotspotChat::ConnectionState HotspotChat::connectionState() const
{
    return m_connectionState;
}

void HotspotChat::setConnectionState(ConnectionState newConnectionState)
{
    if (m_connectionState == newConnectionState)
        return;
    m_connectionState = newConnectionState;
    emit connectionStateChanged();
}

void HotspotChat::readyRead() {
    while (hasPendingDatagrams()) {
        QByteArray data(pendingDatagramSize(), Qt::Uninitialized);
        QHostAddress host;
        quint16 port;
        readDatagram(data.data(), data.size(), &host, &port);

        std::variant var = hproto_read<std::string, RouterRedirectAnswer>(data.data(), data.size());
        if (std::string *string = std::get_if<std::string>(&var)) {
            QString text = QString::fromStdString(*string);
            m_messages.append(QVariantMap({{"from", "remote"}, {"text", text}}));
            emit messagesChanged();
        } else if (RouterRedirectAnswer *answer = std::get_if<RouterRedirectAnswer>(&var)) {
            QHostAddress addr(answer->ip);
            QUrl url;
            url.setScheme("h");
            url.setHost(addr.toString());
            url.setPort(answer->port);

            if (url.isValid()) {
                emit redirected(url);
            }
        } else {
            qWarning() << "got trash";
        }
    }
}
