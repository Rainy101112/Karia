#include "mainwindow.h"

#include <QApplication>
#include <QIcon>
#include <QPointer>

#include <KAboutData>
#include <KLocalizedString>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("karia");

    KAboutData aboutData(
        QStringLiteral("karia"),
        i18n("Karia"),
        QStringLiteral("1.0.0"),
        i18n("KDE 桌面上的 aria2 图形化前端"),
        KAboutLicense::GPL_V3,
        QStringLiteral("(C) 2026 Rainy101112"),
        QString(),
        QStringLiteral("https://github.com/Rainy101112/Karia"));
    aboutData.addAuthor(i18n("Rainy101112"), QString(), QStringLiteral("rainy@example.com"));
    KAboutData::setApplicationData(aboutData);

    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("folder-download")));

    // KXmlGuiWindow 默认设置了 Qt::WA_DeleteOnClose，因此窗口必须分配在堆上，
    // 否则关闭窗口时 Qt 会对栈对象执行 delete，导致 free(): invalid size 崩溃。
    // QPointer 会在窗口已被 deleteLater() 删除时自动置空，避免重复释放。
    QPointer<MainWindow> window = new MainWindow;
    window->show();

    const int result = app.exec();
    delete window;
    return result;
}
