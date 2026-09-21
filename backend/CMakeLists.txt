cmake_minimum_required(VERSION 3.20)

project(VinothMartBackend)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Drogon CONFIG REQUIRED)

find_package(unofficial-sqlite3 CONFIG QUIET)

if(unofficial-sqlite3_FOUND)
    set(SQLITE_TARGET unofficial::sqlite3::sqlite3)
else()
    find_package(SQLite3 REQUIRED)
    set(SQLITE_TARGET SQLite::SQLite3)
endif()

add_executable(VinothMartBackend
    main.cpp
)

target_link_libraries(VinothMartBackend
    PRIVATE
    Drogon::Drogon
    ${SQLITE_TARGET}
)