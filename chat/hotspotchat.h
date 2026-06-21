#ifndef HOTSPOTCHAT_H
#define HOTSPOTCHAT_H

#include "hudpchannel.h"

#include <QQmlEngine>
#include <QObject>
#include <QSocketNotifier>
#include <memory>

class HotspotChat : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(quint16 port READ port NOTIFY portChanged FINAL)
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged FINAL)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged FINAL)
    Q_PROPERTY(QVariantList messages READ messages NOTIFY messagesChanged FINAL)
    Q_PROPERTY(ConnectionState connectionState READ connectionState NOTIFY connectionStateChanged FINAL)

public:
    enum ConnectionState {
        Disconnected,
        ConnectedToRouter,
        ConnectedToPeer
    }; Q_ENUM(ConnectionState)

public:
    explicit HotspotChat(QObject *parent = nullptr);
    ~HotspotChat();

    quint16 port();

    QUrl url() const;
    void setUrl(const QUrl &url);

    bool connected();

    QVariantList messages() const;

    Q_INVOKABLE void send(QString text);

    Q_INVOKABLE void sendFile(QUrl url);

    ConnectionState connectionState() const;

private:
    void setConnectionState(ConnectionState newConnectionState);
    Task processDatagram(HUdpChannel *channel);

    HUdpServer m_server;
    std::shared_ptr<HUdpChannel> m_currentConnection;
    QUrl m_url;
    QVariantList m_messages;

    ConnectionState m_connectionState = Disconnected;

signals:
    void portChanged();
    void urlChanged();
    void connectedChanged();
    void messagesChanged();

    void redirected(QUrl url);

    void connectionStateChanged();
};

#endif // HOTSPOTCHAT_H
