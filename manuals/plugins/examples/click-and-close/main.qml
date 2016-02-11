import QtQuick 2.0

Rectangle {
    id: root
    width: 640
    height: 480

    property int counter: 1

    function reactToClick() {
        documents.create_new();
        documents.get_current().set_selected_text ("hello world");
        Qt.quit();
    }

    Text {
        id: text
        text: qsTr("Click me to create new document and close this window")
        anchors.centerIn: parent
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: {
            reactToClick()
        }
    }
    
     Component.onCompleted: {
     
            console.debug("LOADED");
        }
}
