import QtQuick
import QtCore
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Dialogs
import QtQuick.Layouts
import Qt.labs.platform as Platform
import Qt.labs.qmlmodels 1.0

import EditorComponents 1.0
import LoggerComponents 1.0
import ModelComponents 1.0




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
            title: qsTr("&视图")
        }
        Menu {
            id: setMenu
            title: qsTr("&设置")
        }
        Menu {
            id: toolMenu
            title: qsTr("&工具")
            MenuItem {
                text: qsTr("插件管理")
                onTriggered: {
                    pluginInfoTable.UpModelData()
                    pluginManage.open()
                }
            }
             MenuItem {
                id: other
                text: qsTr("其他工具")
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

    TableModel{
        id:pluginInfoTable
    }


    Dialog {
        id: pluginManage
        modal: true       
        contentWidth: view.width
        contentHeight: view.height
        anchors.centerIn: Overlay.overlay
        title: "插件管理器"

        Rectangle {
            width: 400
            height: 100
            color: "white"

            TableView {
                id: view
                anchors.fill: parent
                clip: true
                model: pluginInfoTable
                delegate: Rectangle {
                    implicitWidth: 100
                    implicitHeight: 20
                    Text {
                        text: display
                        anchors.centerIn: parent
                    }
                }
            }
        }
        footer:DialogButtonBox {
            Button {
                text: qsTr("加载")
                onClicked: model.submit()
            }
            Button {
                text: qsTr("卸载")
                onClicked: model.submit()
            }
            Button {
                text: qsTr("退出")
                onClicked: model.submit()
            }
            Button {
                text: qsTr("刷新")
                onClicked: pluginInfoTable.UpModelData()
            }
        }
    }

}
