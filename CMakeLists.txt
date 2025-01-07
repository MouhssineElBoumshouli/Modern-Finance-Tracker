cmake_minimum_required(VERSION 3.5)

project(ModernFinanceTracker VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Sql
    Charts
    PrintSupport
)

# Include directory for headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Windows specific settings
if(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/app.rc")
endif()

# Source files
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    databasemanager.cpp
    include/transaction.h
    include/databasemanager.h
    app.qrc
    ${APP_ICON_RESOURCE_WINDOWS}
)

# Create executable
add_executable(ModernFinanceTracker
    ${PROJECT_SOURCES}
)

# Link libraries
target_link_libraries(ModernFinanceTracker PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Charts
    Qt6::PrintSupport
)
