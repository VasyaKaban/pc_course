cmake_minimum_required(VERSION 3.5)

project(pc_course_client VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

set(PROJECT_SOURCES
        main.cpp
		Utils/ServerConnection.cpp
		Utils/ServerConnection.h
		Utils/NoReplace.hpp
		Utils/User.hpp
		Utils/Settings.hpp
		Utils/LuaConf.hpp
		Utils/MessageBox.hpp
		Utils/Requests.hpp
		Utils/Model.hpp
		Login/Ui/LoginWindow.cpp
		Login/Ui/LoginWindow.h
		Login/Ui/LoginWindow.ui
		Admin/Ui/AdminPanel.h
		Admin/Ui/AdminPanel.cpp
		Admin/Ui/AdminPanel.ui
		Employee/Ui/EmployeePanel.h
		Employee/Ui/EmployeePanel.cpp
		Employee/Ui/EmployeePanel.ui
		Employee/Ui/NewItemDialog.h
		Employee/Ui/NewItemDialog.cpp
		Employee/Ui/NewItemDialog.ui
		Accountant/Ui/AccountantPanel.h
		Accountant/Ui/AccountantPanel.cpp
		Accountant/Ui/AccountantPanel.ui
		Admin/Models/UsersModel.h
		Admin/Models/UsersModel.cpp
		Employee/Models/CustomersModel.h
		Employee/Models/CustomersModel.cpp
		Employee/Models/ItemsModel.h
		Employee/Models/ItemsModel.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(pc_course_client
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET pc_course_client APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(pc_course_client SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(pc_course_client
            ${PROJECT_SOURCES}
        )
    endif()
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "../client_out/")
target_include_directories(${PROJECT_NAME} PRIVATE "../sdk/")
target_include_directories(${PROJECT_NAME} PRIVATE "../../../../../lua_way/src/")
target_include_directories(${PROJECT_NAME} PRIVATE "Utils/")
find_package(Lua REQUIRED)

target_link_libraries(pc_course_client PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network ${LUA_LIBRARIES})

set_target_properties(pc_course_client PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS pc_course_client
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(pc_course_client)
endif()
