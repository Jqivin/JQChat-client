cmake_minimum_required(VERSION 3.16)
project(JQChat VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    Sql
)

set(SOURCES
    src/main.cpp
    src/app/application.cpp
    src/network/http_client.cpp
    src/network/ws_client.cpp
    src/network/api_manager.cpp
    src/models/user_model.cpp
    src/models/message_model.cpp
    src/models/friend_model.cpp
    src/cache/database.cpp
    src/cache/image_cache.cpp
    src/viewmodels/login_viewmodel.cpp
    src/viewmodels/chat_viewmodel.cpp
    src/viewmodels/contact_viewmodel.cpp
    src/viewmodels/setting_viewmodel.cpp
    src/views/login/login_widget.cpp
    src/views/main_window.cpp
    src/views/chat/chat_window.cpp
    src/views/chat/chat_input.cpp
    src/views/chat/message_item.cpp
    src/views/chat/emoji_picker.cpp
    src/views/contact/contact_list.cpp
    src/views/contact/add_friend_dialog.cpp
    src/views/contact/friend_request_list.cpp
    src/views/setting/setting_dialog.cpp
    src/widgets/tray_icon.cpp
    src/widgets/popup_notify.cpp
    src/widgets/rounded_image.cpp
    src/utils/config.cpp
    src/utils/image_utils.cpp
    src/utils/logger.cpp
)

set(HEADERS
    src/app/application.h
    src/network/http_client.h
    src/network/ws_client.h
    src/network/api_manager.h
    src/network/proto.h
    src/models/user_model.h
    src/models/message_model.h
    src/models/friend_model.h
    src/cache/database.h
    src/cache/image_cache.h
    src/viewmodels/login_viewmodel.h
    src/viewmodels/chat_viewmodel.h
    src/viewmodels/contact_viewmodel.h
    src/viewmodels/setting_viewmodel.h
    src/views/login/login_widget.h
    src/views/main_window.h
    src/views/chat/chat_window.h
    src/views/chat/chat_input.h
    src/views/chat/message_item.h
    src/views/chat/emoji_picker.h
    src/views/contact/contact_list.h
    src/views/contact/add_friend_dialog.h
    src/views/contact/friend_request_list.h
    src/views/setting/setting_dialog.h
    src/widgets/tray_icon.h
    src/widgets/popup_notify.h
    src/widgets/rounded_image.h
    src/utils/config.h
    src/utils/image_utils.h
    src/utils/logger.h
)

set(RESOURCES
    resources/resources.qrc
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
)
