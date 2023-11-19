cmake_minimum_required(VERSION 3.5)

project(pc_course_server LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Sources
	    main.cpp
		AsioDefs.hpp
		Server.h
		Server.cpp
		Connection.h
		Connection.cpp
		Request.hpp
		ServerSettings.hpp
		LuaConfSerialize.hpp
	)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "../server_out/")

add_executable(${PROJECT_NAME} ${Sources})

target_include_directories(${PROJECT_NAME} PRIVATE "../sdk/")
target_include_directories(${PROJECT_NAME} PRIVATE "../../../../../lua_way/src/")
target_link_directories(${PROJECT_NAME} PRIVATE "/usr/lib/x86_64-linux-gnu/")
find_package(Lua REQUIRED)
target_link_libraries(${PROJECT_NAME} mysqlpp mysqlclient ${LUA_LIBRARIES})
