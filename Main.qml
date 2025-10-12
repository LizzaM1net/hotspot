import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import hClient

ApplicationWindow {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    HotspotChat {
        id: hotspot

        url: addressField.text

        onRedirected: (url) => {
            addressField.text = url
            // hotspot.send("hei?")
            greetAddress(url)
            addressField.focus = false
        }

        Component.onCompleted: greetAddress("h://94.159.103.63:17172")
    }

    ColumnLayout {
        anchors.fill: parent

        RowLayout {
            Label {
                id: localPort
                text: hotspot.port
            }

            TextField {
                id: addressField
                Layout.fillWidth: true

                text: "h://127.0.0.1:17171"
                // text: "h://94.159.103.63:17171"
            }

            Rectangle {
                height: addressField.height/2
                width: height

                radius: height/2
                color: hotspot.connected ? "green" : "red"
            }
        }

        ListView {
            id: listView

            Layout.fillHeight: true
            Layout.fillWidth: true

            spacing: 20
            clip: true

            model: hotspot.messages
            delegate: Rectangle {
                required property string from
                required property string text

                width: listView.width*0.7
                height: 40
                // anchors.left: index%2==0 && parent ? parent.left : undefined
                anchors.right: from == "local" && parent ? parent.right : undefined

                color: from == "remote" ? "green" : "red"
                radius: 5

                Text {
                    anchors.centerIn: parent
                    text: parent.text
                }
            }
        }

        RowLayout {
            TextField {
                id: messageText
                Layout.fillWidth: true

                onAccepted: {
                    hotspot.send(text)
                    text = ""
                }
            }
            Button {
                icon.name: "done"
                onClicked: messageText.accepted()
            }
        }
    }
}
