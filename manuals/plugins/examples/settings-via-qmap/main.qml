import QtQuick 2.0

//this example shows the access to TEA settings
//using QMap

Rectangle {
    id: root
    width: 640
    height: 480

    property bool close_on_complete: true

    
    
     Component.onCompleted: 
       {
     
        console.debug ("Rectangle Component.onCompleted");

         for (var prop in hs_path)
             //console.log (prop, "=", hs_path[prop])
             log.log (prop + "=" + hs_path[prop]);
        
        }
        
        
   
}
