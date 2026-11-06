cmake_minimum_required(VERSION 3.16)
project(VideoPlayerScaffold
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "High performance video player & microservice scaffold"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

option(VPS_ENABLE_ALL        "Enable all components"                      OFF)
option(VPS_ENABLE_LOGGER     "Enable Logger utility (spdlog, fmt)"        ON)
option(VPS_ENABLE_JSON       "Enable JsonUtil (jsoncpp)"                  ON)
option(VPS_ENABLE_MAIL       "Enable MailCodeUtil (libcurl)"              ON)
option(VPS_ENABLE_ES         "Enable ESUtil (jsoncpp)"                    ON)
option(VPS_ENABLE_REDIS      "Enable RedisUtil (redis++, hiredis)"        OFF)
option(VPS_ENABLE_FFMPEG     "Enable FFmpegUtil (libavformat, libavcodec)" OFF)
option(VPS_ENABLE_AMQP       "Enable AmqpUtil (AMQP-CPP, libev)"          OFF)
option(VPS_ENABLE_FASTDFS    "Enable FastDFSUtil (fdfsclient)"            OFF)
option(VPS_ENABLE_ETCD       "Enable EtcdUtil (etcd-cpp-api)"             OFF)
option(VPS_ENABLE_RPC        "Enable RpcUtil (brpc, protobuf)"            OFF)
option(VPS_BUILD_EXAMPLES    "Build example utilities"                    OFF)

if(VPS_ENABLE_ALL)
    set(VPS_ENABLE_LOGGER  ON)
    set(VPS_ENABLE_JSON    ON)
    set(VPS_ENABLE_MAIL    ON)
    set(VPS_ENABLE_ES      ON)
    set(VPS_ENABLE_REDIS   ON)
    set(VPS_ENABLE_FFMPEG  ON)
    set(VPS_ENABLE_AMQP    ON)
    set(VPS_ENABLE_FASTDFS ON)
    set(VPS_ENABLE_ETCD    ON)
    set(VPS_ENABLE_RPC     ON)
endif()

set(VPS_SRCS "")
set(VPS_PRIVATE_LIBS "")
set(VPS_PUBLIC_LIBS "")
find_package(Threads REQUIRED)
list(APPEND VPS_PUBLIC_LIBS Threads::Threads)

# --- 模块: Logger (基础组件) ---
if(VPS_ENABLE_LOGGER)
    find_package(spdlog REQUIRED)
    find_package(fmt REQUIRED)
    list(APPEND VPS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/Logger.cpp")
    list(APPEND VPS_PUBLIC_LIBS spdlog::spdlog fmt::fmt)
    # gflags 按需链接（在部分 Linux 发行版中日志与 bRPC 会用到）
    find_library(GFLAGS_LIB gflags)
    if(GFLAGS_LIB)
        list(APPEND VPS_PRIVATE_LIBS ${GFLAGS_LIB})
    endif()
endif()
# --- 模块: JsonUtil ---
if(VPS_ENABLE_JSON)
    find_package(PkgConfig QUIET)
    find_library(JSONCPP_LIB jsoncpp REQUIRED)
    list(APPEND VPS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/JsonUtil.cpp")
    list(APPEND VPS_PUBLIC_LIBS ${JSONCPP_LIB})
endif()
# --- 模块: ESUtil (DSL 构建器) ---
if(VPS_ENABLE_ES)
    find_library(JSONCPP_LIB jsoncpp REQUIRED)
    list(APPEND VPS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/ESUtil.cpp")
    list(APPEND VPS_PUBLIC_LIBS ${JSONCPP_LIB})
endif()
# --- 模块: MailCodeUtil ---
if(VPS_ENABLE_MAIL)
    find_package(CURL REQUIRED)
    list(APPEND VPS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/MailCodeUtil.cpp")
    list(APPEND VPS_PUBLIC_LIBS CURL::libcurl)
endif()
# --- 模块: RedisUtil ---
if(VPS_ENABLE_REDIS)
    find_library(REDIS_PLUS_PLUS_LIB redis++ REQUIRED)
    find_library(HIREDIS_LIB hiredis REQUIRED)
    list(APPEND VPS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/RedisUtil.cpp")
    list(APPEND VPS_PUBLIC_LIBS ${REDIS_PLUS_PLUS_LIB} ${HIREDIS_LIB})
endif()
# --- 模块: FFmpegUtil ---
if(VPS_ENABLE_FFMPEG)
    find_library(AVFORMAT_LIB avformat REQUIRED)
    find_library(AVCODEC_LIB avcodec REQUIRED)
    find_library(AVUTIL_LIB avutil REQUIRED)
    list(APPEND VPS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/FFmpegUtil.cpp")
    list(APPEND VPS_PUBLIC_LIBS ${AVFORMAT_LIB} ${AVCODEC_LIB} ${AVUTIL_LIB})
endif()
# --- 模块: FastDFSUtil ---
if(VPS_ENABLE_FASTDFS)
    find_library(FDFS_CLIENT_LIB fdfsclient REQUIRED)
    find_library(FAST_COMMON_LIB fastcommon REQUIRED)
    list(APPEND VPS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/FastDFSUtil.cpp")
    list(APPEND VPS_PUBLIC_LIBS ${FDFS_CLIENT_LIB} ${FAST_COMMON_LIB})
endif()
# --- 模块: AmqpUtil ---
if(VPS_ENABLE_AMQP)
    find_library(AMQP_CPP_LIB amqpcpp REQUIRED)
    find_library(EV_LIB ev REQUIRED)
    list(APPEND VPS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/AmqpUtil.cpp")
    list(APPEND VPS_PUBLIC_LIBS ${AMQP_CPP_LIB} ${EV_LIB})
endif()
# --- 模块: EtcdUtil ---
if(VPS_ENABLE_ETCD)
    find_library(ETCD_CPP_LIB etcd-cpp-api REQUIRED)
    find_library(CPPREST_LIB cpprest REQUIRED)
    list(APPEND VPS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/EtcdUtil.cpp")
    list(APPEND VPS_PUBLIC_LIBS ${ETCD_CPP_LIB} ${CPPREST_LIB})
endif()
# --- 模块: RpcUtil (bRPC) ---
if(VPS_ENABLE_RPC)
    find_library(BRPC_LIB brpc REQUIRED)
    find_package(Protobuf REQUIRED)
    find_package(OpenSSL REQUIRED)
    find_library(LEVELDB_LIB leveldb REQUIRED)
    list(APPEND VPS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/RpcUtil.cpp")
    list(APPEND VPS_PUBLIC_LIBS
        ${BRPC_LIB}
        protobuf::libprotobuf
        OpenSSL::SSL
        OpenSSL::Crypto
        ${LEVELDB_LIB}
        ${CMAKE_DL_LIBS}
    )
endif()

add_library(VideoPlayerScaffold STATIC ${VPS_SRCS})
add_library(VideoPlayerScaffold::VideoPlayerScaffold ALIAS VideoPlayerScaffold)

target_include_directories(VideoPlayerScaffold PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/VideoPlayerScaffold>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(VideoPlayerScaffold
    PUBLIC  ${VPS_PUBLIC_LIBS}
    PRIVATE ${VPS_PRIVATE_LIBS}
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
install(TARGETS VideoPlayerScaffold
    EXPORT VideoPlayerScaffoldTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT VideoPlayerScaffoldTargets
    FILE VideoPlayerScaffoldTargets.cmake
    NAMESPACE VideoPlayerScaffold::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VideoPlayerScaffold
)
