cmake_minimum_required(VERSION 3.12)
project(SensorViz3D LANGUAGES CXX)

# 基础设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(3rdprty_ROOT_DIR "" CACHE PATH "3rdprty")
set(3rdprty_INCLUDE_DIR ${3rdprty_ROOT_DIR}/include)
set(3rdprty_LIBRARY_DIR ${3rdprty_ROOT_DIR}/lib)
set(3rdprty_BIN_DIR ${3rdprty_ROOT_DIR}/bin)

set(Matlab_ROOT_DIR "" CACHE PATH "Matlab must select the folder at the version level")
set(Matlab_INCLUDE_DIR ${Matlab_ROOT_DIR}/extern/include)
set(Matlab_LIBRARY_DIR ${Matlab_ROOT_DIR}/extern/lib/win64/microsoft)
set(Matlab_BIN_DIR ${Matlab_ROOT_DIR}/bin/win64;${Matlab_ROOT_DIR}/extern/bin/win64)

# 查找 Qt 组件
find_package(Qt5 COMPONENTS 
    Core 
    Widgets 
    Gui 
	AxContainer
	Concurrent
    OpenGL  # customplot需要用到
    REQUIRED
)

# 获取 Qt 二进制路径
get_filename_component(QT_BIN_PATH "${Qt5_DIR}/../../../bin" ABSOLUTE)

# 自动处理 Qt 相关文件
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 源文件收集
file(GLOB_RECURSE HEADERS 
    "src/*.h" 
    "src/*.hpp"
)
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)
file(GLOB_RECURSE UISOURCES 
    "src/*.ui"
)
file(GLOB_RECURSE RCSOURCES 
    "src/*.qrc"
)

# 创建可执行文件
add_executable(${PROJECT_NAME} 
    ${SOURCES} 
    ${HEADERS} 
    ${UISOURCES} 
    ${RCSOURCES}
)


# 包含目录设置
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src  # 包含项目源文件目录
    PRIVATE
        ${3rdprty_INCLUDE_DIR}
        ${Qt5Widgets_INCLUDE_DIRS}
        ${Matlab_INCLUDE_DIR}
)

# 链接目录设置
target_link_directories(${PROJECT_NAME} 
    PRIVATE 
        ${3rdprty_LIBRARY_DIR}
        ${Matlab_LIBRARY_DIR}
)

# 链接库设置
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt5::Core
        Qt5::Widgets
        Qt5::Gui
        Qt5::OpenGL  
		Qt5::AxContainer
		Qt5::Concurrent
		
		dwmapi${CMAKE_STATIC_LIBRARY_SUFFIX}
		
        libfftw3-3${CMAKE_STATIC_LIBRARY_SUFFIX}
		qcustomplot2${CMAKE_STATIC_LIBRARY_SUFFIX}
		
		libmat${CMAKE_STATIC_LIBRARY_SUFFIX}
		libmx${CMAKE_STATIC_LIBRARY_SUFFIX}
		mclmcr${CMAKE_STATIC_LIBRARY_SUFFIX}
		mclmcrrt${CMAKE_STATIC_LIBRARY_SUFFIX}
		
)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
    # 设置调试环境变量
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_ENVIRONMENT 
        "PATH=${QT_BIN_PATH};${3rdprty_BIN_DIR};${Matlab_BIN_DIR};%PATH%"
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

# 安装规则
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 打包支持
include(InstallRequiredSystemLibraries)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)