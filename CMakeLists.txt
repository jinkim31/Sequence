cmake_minimum_required(VERSION 3.0.2)
project(sequence)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

find_package(catkin REQUIRED COMPONENTS
        roscpp
        std_msgs
        )
catkin_package(
        INCLUDE_DIRS inc ros/inc
        LIBRARIES sequence
        CATKIN_DEPENDS roscpp std_msgs
)
include_directories(
        ${catkin_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME} STATIC
        src/sequence_core.cpp
        src/sequence.cpp
        src/sequence_util.cpp
        ros/src/sequence_ros.cpp
        )

add_executable(testcode ros/src/testcode.cpp)
target_link_libraries(testcode ${catkin_LIBRARIES} sequence)

target_include_directories(${PROJECT_NAME} PUBLIC
        inc/sequence_core.h
        inc/sequence.h
        inc/sequence_util.h
        ros/inc/sequence_ros.h)

install(TARGETS ${PROJECT_NAME}
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION})
install(
        DIRECTORY include/${PROJECT_NAME}/
        DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
)
