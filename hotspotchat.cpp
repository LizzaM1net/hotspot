#include "hotspotchat.h"
#include "hotspot.capnp.h"
#include <capnp/message.h>
#include <capnp/serialize-packed.h>

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
}

bool HotspotChat::connected() {
    return state() == QAbstractSocket::ConnectedState;
}

QVariantList HotspotChat::messages() const {
    return m_messages;
}

void HotspotChat::send(QString text) {
    ::capnp::MallocMessageBuilder message;
    Header::Builder header = message.initRoot<Header>();

    qWarning() << QString::fromUcs4(&HOTSPOT, 1);

    header.setEmoji(HOTSPOT);

    kj::VectorOutputStream kjBuffer;
    capnp::writeMessage(kjBuffer, message);
    kj::ArrayPtr<unsigned char> arr = kjBuffer.getArray();

    QByteArray array(reinterpret_cast<const char*>(arr.begin()), arr.size() * sizeof(unsigned char));

    // std::string rawText = text.toStdString();
    writeDatagram(array, QHostAddress(m_url.host()), m_url.port());

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

        kj::ArrayPtr arr(reinterpret_cast<const unsigned char*>(data.data()), data.size());
        kj::ArrayInputStream stream(arr);

        ::capnp::InputStreamMessageReader message(stream);

        Header::Reader header = message.getRoot<Header>();

        char32_t emoji = header.getEmoji();
        QChar emojiChar(emoji);
        qWarning() << emojiChar;

        // QString text(data);
        QString text = emojiChar;
        m_messages.append(QVariantMap({{"from", "remote"}, {"text", text}}));
        // qDebug() << data;
        emit messagesChanged();

        if (data.startsWith(c_bannerRedirect)) {
            QUrl url(data.mid(c_bannerRedirect.size()));
            if (url.isValid()) {
                qDebug() << "got redirect url" << url;
                emit redirected(url);
            }
        }
    }
}
