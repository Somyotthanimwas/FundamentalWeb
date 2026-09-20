cmake_minimum_required(VERSION 3.20)
project(FundamentalWebUploader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

add_executable(FundamentalWebUploader
    FundamentalWebUploader.cpp
)

target_link_libraries(FundamentalWebUploader PRIVATE CURL::libcurl)

if (WIN32)
    target_compile_definitions(FundamentalWebUploader PRIVATE
        CURL_STATICLIB
    )
endif()
