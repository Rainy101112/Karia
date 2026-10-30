qt_add_executable(karia
    main.cpp
)

qt_add_qml_module(karia
    URI Karia
    VERSION 1.0
    QML_FILES
        qml/Main.qml
)

# Use the QtQuick module from Qt 6.
target_link_libraries(karia PRIVATE Qt6::Quick)

# Install the executable
install(TARGETS karia DESTINATION bin)
