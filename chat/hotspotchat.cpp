#include "hotspotchat.h"

#include "hproto.h"
#include "hproto_router.h"
#include "hproto_types.h"

#include <QFile>
#include <QTemporaryFile>
#include <QStandardPaths>
#include <QGuiApplication>
#include <QDateTime>
#include <QHostAddress>

HotspotChat::HotspotChat(QObject *parent)
    : QObject{parent}
    , m_server(INADDR_ANY, 0, [this](HUdpChannel *channel) -> Task {
        return processDatagram(channel);
    })
{
    auto *notifier = new QSocketNotifier(m_server.sockfd(), QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated,
        this, [this](){
            m_server.processDatagram();
        });
}

HotspotChat::~HotspotChat() {
}

quint16 HotspotChat::port() {
    return m_server.address().port();
}

QUrl HotspotChat::url() const {
    return m_url;
}

void HotspotChat::setUrl(const QUrl &newUrl) {
    if (m_url == newUrl)
        return;

    m_url = newUrl;
    emit urlChanged();

    std::variant<RouterCreateWaitroomRequest> var = RouterCreateWaitroomRequest{ m_server.address() };
    QByteArray arr(hproto_size(var), Qt::Uninitialized);
    hproto_write(var, arr.data());

    if (m_currentConnection)
        m_currentConnection->close();

    HSocketAddress peer(QHostAddress(m_url.host()).toIPv4Address(), m_url.port());
    m_currentConnection = m_server.channelToAddress(peer);
    m_currentConnection->write(arr.data(), arr.size());
}

bool HotspotChat::connected() {
    return m_connectionState == ConnectedToPeer;
}

QVariantList HotspotChat::messages() const {
    return m_messages;
}

void HotspotChat::send(QString text) {
    if (!m_currentConnection)
        return;

    std::variant<std::string> var = text.toStdString();
    QByteArray arr(hproto_size(var), Qt::Uninitialized);
    hproto_write(var, arr.data());

    m_currentConnection->write(arr.data(), arr.size());

    m_messages.append(QVariantMap({{"from", "local"}, {"type", "text"}, {"path", ""}, {"text", text}}));
    emit messagesChanged();
}

void HotspotChat::sendFile(QUrl url) {
    if (!m_currentConnection)
        return;

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

    m_currentConnection->write(arr.data(), arr.size());

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

Task HotspotChat::processDatagram(HUdpChannel *channel) {
    char buffer[65536];

    while (true) {
        ssize_t n = co_await channel->read(buffer, sizeof(buffer));
        if (n < 0)
            continue;

        std::variant var = hproto_read<std::string, RouterRedirectAnswer, HotspotFile, RouterGreet>(buffer, n);
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
