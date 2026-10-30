import QtQuick
import org.kde.kirigami as Kirigami

Kirigami.GlobalDrawer {
    id: drawer

    isMenu: true

    actions: [
        Kirigami.Action {
            text: "hello"
        }
    ]
}
