import QtQuick 2.0

Rectangle {

    id: root
    width: 640
    height: 480

    property bool close_on_complete: true

    Component.onCompleted: //run it on plugin load
       {
        console.debug ("Rectangle Component.onCompleted");
        documents.create_new(); //create new document
        documents.get_current().set_selected_text ("hello world"); //insert text at the cursor position
       }
  
}
