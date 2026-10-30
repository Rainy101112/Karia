cmake_minimum_required(VERSION 3.16)

project(Karia VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt Widgets + Network
find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Network)

# KDE Frameworks 6
find_package(KF6XmlGui REQUIRED)
find_package(KF6CoreAddons REQUIRED)
find_package(KF6WidgetsAddons REQUIRED)
find_package(KF6I18n REQUIRED)
find_package(KF6Config REQUIRED)

qt_standard_project_setup()

add_subdirectory(src)
