#Tu nastavit verziu Qt6
set(Qt6_Version 6.9.3)
cmake_minimum_required(VERSION 3.16.0)
PROJECT(TSS_App VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
	add_definitions( "/D_CRT_SECURE_NO_WARNINGS /MP /openmp" )
endif(MSVC)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(Qt6_DIR "C:/Qt/${Qt6_Version}/msvc2022_64/lib/cmake/Qt6")
set(Qt6WidgetsTools_DIR "C:/Qt/${Qt6_Version}/msvc2022_64/lib/cmake/Qt6WidgetsTools")
set(Qt6CoreTools_DIR "C:/Qt/${Qt6_Version}/msvc2022_64/lib/cmake/Qt6CoreTools")
set(Qt6GuiTools_DIR "C:/Qt/${Qt6_Version}/msvc2022_64/lib/cmake/Qt6GuiTools")

# Pridaj Test komponent pre testy
find_package(Qt6 COMPONENTS Widgets Core Gui Test REQUIRED)

file(GLOB UI_FILES src/*.ui)
file(GLOB H_FILES src/*.h)
file(GLOB CPP_FILES src/*.cpp)
file(GLOB QRC_FILES src/*.qrc)
file(GLOB RC_FILES src/*.rc)
file(GLOB TEST_SOURCES tests/*.cpp)

set(SOURCE_LIST ${CPP_FILES} ${UI_FILES} ${H_FILES} ${QRC_FILES} ${RC_FILES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${SOURCE_LIST}
    ${TEST_SOURCES}
)

# Deploy Qt function
function(deploy_qt_for_target target_name)
    add_custom_command(TARGET ${target_name}
        POST_BUILD
        COMMAND "C:/Qt/${Qt6_Version}/msvc2022_64/bin/windeployqt.exe" "$<TARGET_FILE:${target_name}>"
        COMMENT "Running windeployqt for ${target_name}")
endfunction()

# =====================================================
# 1) Hlavná aplikácia
# =====================================================
add_executable(${PROJECT_NAME} ${SOURCE_LIST})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
deploy_qt_for_target(${PROJECT_NAME})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/resources"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
    COMMENT "Copying resources folder to build directory"
)
# =====================================================
# 2) Testy
# =====================================================
set(PROJECT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 2.1) GUI Test
add_executable(tst_TSS_AppGUI
    tests/TestTSSAppGUI.cpp
    src/TSS_App.cpp
    src/TSS_App.h
    src/PhotoTableModel.cpp
    src/PhotoTableModel.h
    src/PhotoMetadata.cpp
    src/PhotoMetadata.h
    src/PhotoDetailDialog.cpp
    src/PhotoDetailDialog.h
    src/PhotoEditDialog.cpp
    src/PhotoEditDialog.h
    src/PhotoExportDialog.cpp
    src/PhotoExportDialog.h
    src/ThemeUtils.h
    src/Photo.cpp              
    src/Photo.h
    src/ThemeUtils.cpp         
    src/CropDialog.cpp
    src/CropDialog.h   
)

target_link_libraries(tst_TSS_AppGUI
    PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui Qt6::Test
)

target_include_directories(tst_TSS_AppGUI
    PRIVATE ${PROJECT_SRC_DIR}
)

deploy_qt_for_target(tst_TSS_AppGUI)

# 2.2) Integration Test
add_executable(tst_TSS_AppIntegration
    tests/TestTSSAppIntegration.cpp
    src/TSS_App.cpp
    src/TSS_App.h
    src/PhotoTableModel.cpp
    src/PhotoTableModel.h
    src/PhotoMetadata.cpp
    src/PhotoMetadata.h
    src/PhotoDetailDialog.cpp
    src/PhotoDetailDialog.h
    src/PhotoEditDialog.cpp
    src/PhotoEditDialog.h
    src/PhotoExportDialog.cpp
    src/PhotoExportDialog.h
    src/ThemeUtils.h
    src/Photo.cpp              
    src/Photo.h
    src/ThemeUtils.cpp 
    src/CropDialog.cpp
    src/CropDialog.h   
        
)

target_link_libraries(tst_TSS_AppIntegration
    PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui Qt6::Test
)

target_include_directories(tst_TSS_AppIntegration
    PRIVATE ${PROJECT_SRC_DIR}
)

deploy_qt_for_target(tst_TSS_AppIntegration)


# 2.3) Unit Test
add_executable(tst_TSS_AppUnit
    tests/TestTSSAppUnit.cpp
    src/PhotoTableModel.cpp
    src/PhotoTableModel.h
    src/PhotoMetadata.cpp
    src/PhotoMetadata.h
    src/Photo.cpp              
    src/Photo.h
)

target_link_libraries(tst_TSS_AppUnit
    PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui Qt6::Test
)

target_include_directories(tst_TSS_AppUnit
    PRIVATE ${PROJECT_SRC_DIR}
)

deploy_qt_for_target(tst_TSS_AppUnit)

# =====================================================
# 3) Registrácia testov pre ctest
# =====================================================
enable_testing()

add_test(NAME gui_TSS_App
    COMMAND tst_TSS_AppGUI
)

add_test(NAME integration_TSS_App
    COMMAND tst_TSS_AppIntegration
)

add_test(NAME unit_TSS_App
    COMMAND tst_TSS_AppUnit
)