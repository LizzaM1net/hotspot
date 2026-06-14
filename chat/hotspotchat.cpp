#include "hotspotchat.h"

#include "hproto.h"
#include "hproto_router.h"
#include "hproto_types.h"

#include <QFile>
#include <QTemporaryFile>
#include <QStandardPaths>
#include <QGuiApplication>
#include <QDateTime>

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
    HSocketAddress address(addr, localPort());

    std::variant<RouterCreateWaitroomRequest> var = RouterCreateWaitroomRequest{ address };
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

    m_messages.append(QVariantMap({{"from", "local"}, {"type", "text"}, {"path", ""}, {"text", text}}));
    emit messagesChanged();
}

void HotspotChat::greetAddress(QUrl url) {
    std::variant<RouterGreet> var = RouterGreet();
    QByteArray arr(hproto_size(var), Qt::Uninitialized);
    hproto_write(var, arr.data());

    writeDatagram(arr, QHostAddress(url.host()), url.port());
}

void HotspotChat::sendFile(QUrl url) {
    if (url.scheme() != "file")
        return;

    QFile file(url.toLocalFile());
    if (!file.open(QFile::ReadOnly))
        return;

    QByteArray content = file.readAll();
    file.close();
    std::vector<char> contentVector(content.begin(), content.end());

    QString name = file.fileName();
    name = name.mid(name.lastIndexOf("/")+1);
    std::variant<HotspotFile> var = HotspotFile{name.toStdString(), contentVector};
    QByteArray arr(hproto_size(var), Qt::Uninitialized);
    hproto_write(var, arr.data());

    writeDatagram(arr, QHostAddress(m_url.host()), m_url.port());

    m_messages.append(QVariantMap({{"from", "local"}, {"type", "file"}, {"path", "file://" + file.fileName()}, {"text", name}}));
    emit messagesChanged();
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

        std::variant var = hproto_read<std::string, RouterRedirectAnswer, HotspotFile, RouterGreet>(data.data(), data.size());
        if (std::string *string = std::get_if<std::string>(&var)) {
            QString text = QString::fromStdString(*string);
            m_messages.append(QVariantMap({{"from", "remote"}, {"type", "text"}, {"path", ""}, {"text", text}}));
            emit messagesChanged();
        } else if (RouterRedirectAnswer *answer = std::get_if<RouterRedirectAnswer>(&var)) {
            QHostAddress addr(answer->peerAddress.ip());
            QUrl url;
            url.setScheme("h");
            url.setHost(addr.toString());
            url.setPort(answer->peerAddress.port());

            if (url.isValid()) {
                emit redirected(url);
            }
        } else if (HotspotFile *file = std::get_if<HotspotFile>(&var)) {
            QString name = QString::fromStdString(file->name);
            QFile tmpFile(QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first()+"/"+name.first(name.lastIndexOf("."))+QDateTime::currentDateTime().toString("HH:mm:ss")+name.mid(name.lastIndexOf(".")));
            if (!tmpFile.open(QFile::WriteOnly))
                continue;
            std::vector<char> contents = file->data;
            tmpFile.write(contents.data(), contents.size());
            tmpFile.close();
            m_messages.append(QVariantMap({{"from", "remote"}, {"type", "file"}, {"path", "file://" + tmpFile.fileName()}, {"text", name}}));
            emit messagesChanged();
        } else if (std::get_if<RouterGreet>(&var)) {
            qWarning() << "got greet";
        } else {
            qWarning() << "got trash";
        }
    }
}
