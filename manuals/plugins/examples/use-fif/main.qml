import QtQuick 2.0

Rectangle {
    id: root
    width: 640
    height: 480

    property int counter: 1

    function reactToClick() {

     text.text = tea.fif_get_text();
     
    }

    Text {
        id: text
        text: qsTr("Click me to show the FIF value")
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
