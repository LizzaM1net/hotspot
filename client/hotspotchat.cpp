#include "hotspotchat.h"

#include "hproto.h"
#include "hproto_types.h"

#include <QFile>
#include <QTemporaryFile>
#include <QStandardPaths>
#include <QGuiApplication>
#include <QDateTime>

struct HotspotFile {
    std::string name;
    QByteArray data;
};

template<>
struct HProtoData<HotspotFile> {
    static constexpr const hproto_id_t hproto_id = 0x2001;
    static constexpr size_t hproto_size(const HotspotFile &f) {
        return HProtoData<std::string>::hproto_size(f.name) + sizeof(string_len_t) + f.data.size();
    }
    static bool hproto_accepts_size(size_t s) {
        // TODO: Maybe move size validation to read
        return s >= 2*sizeof(string_len_t);
    }
    static void hproto_write(const HotspotFile &f, void *data) {
        size_t stringSize = HProtoData<std::string>::hproto_size(f.name);
        HProtoData<std::string>::hproto_write(f.name, data);
        string_len_t dataSize = f.data.size();
        memcpy((char*)data+stringSize, &dataSize, sizeof(string_len_t));
        memcpy((char*)data+stringSize+sizeof(string_len_t), f.data.data(), dataSize);
    }
    static HotspotFile hproto_read(const void* data) {
        HotspotFile file;
        file.name = HProtoData<std::string>::hproto_read(data);
        size_t stringSize = HProtoData<std::string>::hproto_size(file.name);
        string_len_t dataSize;
        memcpy(&dataSize, (char*)data+stringSize, sizeof(string_len_t));
        file.data = QByteArray((char*)data+stringSize+sizeof(string_len_t), dataSize);
        return file;
    }
};

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

    std::variant<RouterCreateWaitroomRequest> var = RouterCreateWaitroomRequest{addr, localPort(), 0};
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

    QString name = file.fileName();
    name = name.mid(name.lastIndexOf("/")+1);
    std::variant<HotspotFile> var = HotspotFile{name.toStdString(), content};
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
            QHostAddress addr(answer->ip);
            QUrl url;
            url.setScheme("h");
            url.setHost(addr.toString());
            url.setPort(answer->port);

            if (url.isValid()) {
                emit redirected(url);
            }
        } else if (HotspotFile *file = std::get_if<HotspotFile>(&var)) {
            QString name = QString::fromStdString(file->name);
            QFile tmpFile(QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first()+"/"+name.first(name.lastIndexOf("."))+QDateTime::currentDateTime().toString("HH:mm:ss")+name.mid(name.lastIndexOf(".")));
            if (!tmpFile.open(QFile::WriteOnly))
                continue;
            tmpFile.write(file->data);
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
