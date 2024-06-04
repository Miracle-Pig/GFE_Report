cmake_minimum_required(VERSION 3.5)

project(GFE_Report VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_definitions(_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
add_compile_definitions(SPDLOG_WCHAR_FILENAMES)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt5 REQUIRED COMPONENTS AxContainer)

file(GLOB SRC_LIST
    "*.cpp"
    "Prog/*.cpp"
    "Utils/*.cpp"

)

file(GLOB HEADER_LIST
    "*.h"
    "Prog/*.h"
    "Utils/*.h"
)

file(GLOB UI_LIST
    "*.ui"
    "Prog/*.ui"
    "Utils/*.ui"
)

#file(GLOB_RECURSE VTK_LIB_D $ENV{TRD_ROOT}/VTK-8.2.0/build/lib/Debug/*.lib)
#file(GLOB_RECURSE VTK_LIB )

set(PROJECT_SOURCES
    main.cpp
    ${HEADER_LIST}
    ${SRC_LIST}
    ${UI_LIST}
)

add_executable(GFE_Report
    ${PROJECT_SOURCES}
    PrePo.qrc
)

# GFE内部库
set(modules GFE_FileIO GFE_PreAPI GFE_API GFE_Utility GFE_YJKIO)
foreach(m IN LISTS modules)
if(TARGET ${m})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${m})
else()
    target_include_directories(${PROJECT_NAME} PRIVATE $ENV{TRD_ROOT}/${m}/include)
    target_link_libraries(${PROJECT_NAME} PRIVATE $ENV{TRD_ROOT}/${m}/lib/${m}$<$<CONFIG:Debug>:_d>.lib)
endif()
endforeach()

target_include_directories(${PROJECT_NAME}
    PRIVATE
    $ENV{TRD_ROOT}/VTK-8.2.0/include
    $ENV{TRD_ROOT}/python3.8/include
    $ENV{TRD_ROOT}/occt/inc
    $ENV{TRD_ROOT}
)

target_link_directories(${PROJECT_NAME}
    PRIVATE
    $ENV{TRD_ROOT}/python3.8/libs
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt5::AxContainer
    Qt${QT_VERSION_MAJOR}::Widgets
    python38
    $ENV{TRD_ROOT}/VTK-8.2.0/build/lib/Release/*.lib
)

file(GLOB_RECURSE OCC_LIB_D $ENV{TRD_ROOT}/occt/win64/vc14/libd/*.lib)
file(GLOB_RECURSE OCC_LIB_RD $ENV{TRD_ROOT}/occt/win64/vc14/libi/*.lib)
file(GLOB_RECURSE OCC_LIB $ENV{TRD_ROOT}/occt/win64/vc14/lib/*.lib)

if(CMAKE_BUILD_TYPE MATCHES Release)
    foreach(l IN LISTS OCC_LIB)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${l})
    endforeach()
elseif(CMAKE_BUILD_TYPE MATCHES Debug)
    foreach(l IN LISTS OCC_LIB_D)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${l})
    endforeach()
elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    foreach(l IN LISTS OCC_LIB_RD)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${l})
    endforeach()
endif()

