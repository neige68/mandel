# <CMakeLists.txt> -*- coding: utf-8 -*-
#
# Project GoldenMandel
# Copyright (C) 2022 neige68
# https://github.com/neige68/mandel
# This program is released under license GPLv3
#

cmake_minimum_required(VERSION 3.16)

# スタティックリンク
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(GoldenMandel CXX)

#------------------------------------------------------------
#
# boost
#

find_package(Boost 1.75 REQUIRED)
message(DEBUG "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(DEBUG "Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

#------------------------------------------------------------
#
# OWL
#

set(OWL "$ENV{OWLROOT}")
#set(OWL "u:/lib/owl/7.0.2")
include_directories(${OWL}/include)
link_directories(${OWL}/lib)

#------------------------------------------------------------

# C++17 使用
set(CMAKE_CXX_STANDARD "17")

# /MP: マルチプロセス
# /EHsc: 例外に関する推奨オプション、OWL に必須だと思われる
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /EHsc")
# OWL 用のデバッグオプション
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D__TRACE /D__WARN /D__DEBUG=2")

add_compile_definitions(_WIN32_WINNT=0x0600) # Vista 以上
add_compile_definitions(_OWLPCH) # OWL プリコンパイルヘッダ
add_compile_definitions(UNICODE;_UNICODE)
add_link_options(/ENTRY:wWinMainCRTStartup)

add_executable(GoldenMandel WIN32
    main.cpp misc.cpp pointd.cpp drawwndw.cpp colordlg.cpp colormap.cpp fltrngvl.cpp
    mandel.rc resource.rc)

