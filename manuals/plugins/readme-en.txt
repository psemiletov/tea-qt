TEA PLUGINS DEVELOPER GUIDE

1. What is the TEA plugin?

The TEA plugin is QML-program that has access to inner TEA classes and variables. TEA plugin's files must be placed to its own directory. The main file must have the name "main.qml". It is the entry point. Plugin's directory name is the name of the plugin for TEA "Functions - Plugins" menu. The set of plugin directories can be placed into the top-level directory. For example, TEA source snips with "examples"  plugins directory. When you put it to $HOME/.config/tea/plugins, you'll the the "examples" menu at the "Funtions - Plugins" menu.

2. How to distribute TEA plugin?

Pack it into the ZIP archive. It must contain the plugin's files WITH their directory. To install the plugin, just unpack such zipped directory to $HOME/.config/tea/plugins.

3. What can do TEA plugin?

All things that can do QML programs. There are two types of plugins - the visible (that runs in separated windows) and invisible (that intended to be finished after the exacution). The basic example of the second type see at "run-and-close" example. 

To show TEA that plugin must be closed after execution, you need to set close_on_complete at the root QML component:

property bool close_on_complete: true

Then, put all your code to Component.onCompleted. After the execution will done, TEA closes the plugin with close_on_complete == true.

Without close_on_complete == true, TEA does not close plugin until the user close it manually.

4. TEA API

There are some inner TEA objects are available:

document_holder  documents
CLog log
rvln tea
QSettings settings
QVariantMap hs_path

See the TEA source for details about class members. You can call (from the QML plugin) all public slots and Q_INVOKABLE functions of the classes above.

The basic way to interact with TEA is to get the instance of current object, get the all or selected text, process the text, change the all text or the selection.

For example:

documents.create_new(); //create new document
documents.get_current().set_selected_text ("hello world"); //insert text at the cursor position or replaces the selection

documents.get_current() returns the pointer to CDocument. 

To set selection or insert the text use: 
documents.get_current().set_selected_text ("some string");

To get rhe current selection, use: 

documents.get_current().get_selected_text()

To write some text at the logmemo, use

log.log ("some text");

To get access to TEA settings and paths, use settings and hs_path objects.

To get FIF value, use

tea.fif_get_text();
