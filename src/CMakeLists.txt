add_executable(karia
    main.cpp
    mainwindow.cpp
    aria2client.cpp
    settingsdialog.cpp
    downloadmodel.cpp
    downloadfilterproxymodel.cpp
    progressbardelegate.cpp
)

# 将 XMLGUI 布局文件内嵌到可执行文件中，
# 这样就不依赖安装路径，也不需要 ECM 的 kxmlgui_add_ui。
qt_add_resources(karia "kariaui"
    PREFIX "/"
    FILES kariaui.rc
)

target_link_libraries(karia PRIVATE
    Qt6::Widgets
    Qt6::Network
    KF6::XmlGui
    KF6::CoreAddons
    KF6::WidgetsAddons
    KF6::I18n
    KF6::ConfigCore
)

install(TARGETS karia DESTINATION bin)
