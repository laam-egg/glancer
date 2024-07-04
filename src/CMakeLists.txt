cmake_minimum_required (VERSION 3.8)

add_executable (glancer "main.cpp" "MainFrame.cpp" "BaseFrame.cpp" "HomePage.cpp" "Utils.cpp" "ImagePage.cpp")

target_include_directories(glancer PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../include")
target_include_directories(glancer PRIVATE ${wxWidgets_SOURCE_DIR}/include)
target_link_libraries(glancer PRIVATE wx::base wx::core)

if (X11_FOUND)
  target_include_directories(glancer PUBLIC ${X11_INCLUDE_DIR})
  target_link_libraries(glancer PUBLIC ${X11_LIBRARIES})
endif()

set_target_properties(glancer PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

set_target_properties(glancer PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../bin"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET glancer PROPERTY CXX_STANDARD 17)
endif()
