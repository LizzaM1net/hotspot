#include "hotspotchat.h"

#include <hproto.h>

static constexpr uint32_t HOTSPOT = 128732u;
static constexpr uint32_t WAVE = 128075u;

struct Emoji {
    H_SIZED_OBJECT(0x1001)
    char32_t emoji;
};
HOTSPOT_GUARD_SIZE(Emoji, 4)

struct RouterCreateWaitroomRequest {
    H_SIZED_OBJECT(0x1002)
    uint32_t local_ip;
    uint16_t local_port;
};
HOTSPOT_GUARD_SIZE(RouterCreateWaitroomRequest, 8)

struct RouterRedirectAnswer {
    H_SIZED_OBJECT(0x1003)
    uint32_t ip;
    uint16_t port;
};
HOTSPOT_GUARD_SIZE(RouterRedirectAnswer, 8)

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

    // if (state() == QAbstractSocket::ConnectedState
    //     || state() == QAbstractSocket::ConnectingState
    //     || state() == QAbstractSocket::HostLookupState)
    //     disconnectFromHost();

    // if (!m_url.isValid() || m_url.port() == -1)
    //     return;

    // connectToHost(m_url.host(), m_url.port());

    // Emoji emoji{(char32_t)WAVE};
    // HVariant variant;
    // variant = emoji;
    // qWarning() << QString::fromUcs4(&variant.value<Emoji>()->emoji, 1);
    // emoji.emoji = HOTSPOT;
    // qWarning() << QString::fromUcs4(&variant.value<Emoji>()->emoji, 1);
    // variant = emoji;
    // qWarning() << QString::fromUcs4(&variant.value<Emoji>()->emoji, 1);

    quint32 addr = localAddress().toIPv4Address();
    RouterCreateWaitroomRequest request{addr, localPort()};
    HVariant variant;
    variant = request;

    QByteArray arr(variant.serializedSize<RouterCreateWaitroomRequest>(), Qt::Uninitialized);
    variant.serializeToArray<RouterCreateWaitroomRequest>(arr.data());

    writeDatagram(arr.data(), arr.size(), QHostAddress(m_url.host()), m_url.port());
}

bool HotspotChat::connected() {
    return m_connectionState == ConnectedToPeer;
}

QVariantList HotspotChat::messages() const {
    return m_messages;
}

void HotspotChat::send(QString text) {
    Emoji emoji{(char32_t)WAVE};
    HVariant variant;
    variant = emoji;

    QByteArray arr(variant.serializedSize<RouterCreateWaitroomRequest>(), Qt::Uninitialized);
    variant.serializeToArray<RouterCreateWaitroomRequest>(arr.data());

    writeDatagram(arr, QHostAddress(m_url.host()), m_url.port());

    m_messages.append(QVariantMap({{"from", "local"}, {"text", QString::fromUcs4(&HOTSPOT, 1)}}));
    emit messagesChanged();
}

void HotspotChat::greetAddress(QUrl url) {
    writeDatagram(c_bannerGreet, QHostAddress(url.host()), url.port());
}

void HotspotChat::readyRead() {
    while (hasPendingDatagrams()) {
        QByteArray data(pendingDatagramSize(), Qt::Uninitialized);
        QHostAddress host;
        quint16 port;
        readDatagram(data.data(), data.size(), &host, &port);

        HVariant var = HVariant::deserializeFromArray<Emoji, RouterRedirectAnswer>(data.data(), data.size());
        if (Emoji *emoji = var.cast<Emoji*>()) {
            QString text = QStringView(QChar::fromUcs4(emoji->emoji)).toString();

            m_messages.append(QVariantMap({{"from", "remote"}, {"text", text}}));
            emit messagesChanged();
        } else if (RouterRedirectAnswer *answer = var.cast<RouterRedirectAnswer*>()) {
            QHostAddress addr(answer->ip);
            QUrl url;
            url.setScheme("h");
            url.setHost(addr.toString());
            url.setPort(answer->port);

            if (url.isValid()) {
                emit redirected(url);
            }
        } else {
            qWarning() << "got  trash";
        }
    }
}
