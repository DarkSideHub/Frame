import QtQuick
import QtCore
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Dialogs
import QtQuick.Layouts
import Qt.labs.platform as Platform

import EditorComponents 1.0
import LoggerComponents 1.0



ApplicationWindow {
    id: window
    visible: true
    title: "https Example"
  
    menuBar: MenuBar {
        id: menuBar

        Menu {
            id: fileMenu
            title: qsTr("File")
            MenuItem {
                text: qsTr("open")
                onTriggered: openDialog.open()
            }
        }
        Menu {
            id: editMenu
            title: qsTr("&Edit")
        }  
        Menu {
            id: viewMenu
            title: qsTr("&View")
        }
        Menu {
            id: setMenu
            title: qsTr("&Set")
        }
        Menu {
            id: pluginMenu
            title: qsTr("&插件管理")
            MenuItem {
                text: qsTr("加载")
            }
        }

    }

    FileDialog {
        id: openDialog
        fileMode: FileDialog.OpenFile
        selectedNameFilter.index: 1
        nameFilters: ["Markdown files (*.md *.markdown)"]
        currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        onAccepted: textEditor.load(selectedFile)//on<Signal>是信号处理程序，Accepted是信号
    }

    header: ToolBar{

    }

    footer: ToolBar {
        height: footerRow.implicitHeight + 6

        RowLayout {
            id: footerRow
            anchors.fill: parent

            BusyIndicator {
                id:busyIndicator
                running: false
            }

            Label{
                id:statusbar
                text:"就绪"
            }
        }
    }

    Log{
        objectName:"LogObject"
        id: logger
        qmllog: logArea.textDocument
    }


    TextEditor{
        id: textEditor
        qmldoc: textArea.textDocument

        onLoaded: function (text, format) {
            textArea.textFormat = format
            textArea.text = text
        }
        onSendLog: function (log,LogLevel) {
            logger.ReceiveLog(log,LogLevel)
        }
    }


    RowLayout {
        anchors.fill: parent
        
        ScrollView {
            Layout.preferredWidth: parent.width/3*2
            Layout.fillHeight: true
            background: Rectangle {
                anchors.fill: parent
                border.width:1
                border.color:"black"

                Rectangle{
                    color:parent.color
                    border.width:0
                    anchors.fill: parent
                    anchors.leftMargin:1
                    anchors.topMargin:1
                    anchors.rightMargin:1
                    anchors.bottomMargin:1
                }
            }

            TextArea {
                id: textArea
                textFormat: Qt.RichText
                wrapMode: TextArea.Wrap
                focus: true
                selectByMouse: true
                persistentSelection: true
                Keys.enabled:true
                leftPadding: 6//与内部组件的间距
                rightPadding: 6
                topPadding: 0
                bottomPadding: 0
                background: null
                MouseArea {
                    acceptedButtons: Qt.RightButton//响应的鼠标按键
                    anchors.fill: parent
                    onClicked: cmdMenu.open()
                }
                Keys.onPressed: (event)=> {
                    if (event.key == Qt.Key_F1) {
                    statusbar.text="正在发送和接受数据"
                    busyIndicator.running=true
                        textEditor.Send()
                        event.accepted = true
                        statusbar.text="就绪"
                        busyIndicator.running=false
                    }
                }
                onLinkActivated: function (link) {
                    Qt.openUrlExternally(link)
                }
            }
        }
        
        ScrollView {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width/3-10
            background: Rectangle {
                anchors.fill: parent
                border.width:1
                border.color:"black"

                Rectangle{
                    color:parent.color
                    border.width:0
                    anchors.fill: parent
                    anchors.leftMargin:1
                    anchors.topMargin:1
                    anchors.rightMargin:1
                    anchors.bottomMargin:1
                }
            }
            TextArea {
                id: logArea
                textFormat: Qt.RichText
                wrapMode: TextArea.Wrap
                focus: true
                selectByMouse: true
                persistentSelection: true
                Keys.enabled:false

                leftPadding: 6//与内部组件的间距
                rightPadding: 6
                topPadding: 0
                bottomPadding: 0
                background: null
            }
        }     
    }   
}
