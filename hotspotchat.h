#ifndef HOTSPOTCHAT_H
#define HOTSPOTCHAT_H

#include <QQmlEngine>
#include <QUdpSocket>

class HotspotChat : public QUdpSocket
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(quint16 port READ port NOTIFY portChanged FINAL)
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged FINAL)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged FINAL)
    Q_PROPERTY(QVariantList messages READ messages NOTIFY messagesChanged FINAL)

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

    Q_INVOKABLE void greetAddress(QUrl url);

private:
    quint16 m_port = 0;
    QUrl m_url;
    QVariantList m_messages;

    ConnectionState m_connectionState = Disconnected;

signals:
    void portChanged();
    void urlChanged();
    void connectedChanged();
    void messagesChanged();

    void redirected(QUrl url);

private slots:
    void readyRead();
};

#endif // HOTSPOTCHAT_H
