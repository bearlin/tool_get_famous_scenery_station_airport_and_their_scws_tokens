cmake_minimum_required(VERSION 2.8)

project(scws_get_tokens)

set(SCWS_GET_TOKENS_ROOT
  ${PROJECT_SOURCE_DIR}
  CACHE STRING "SCWS_GET_TOKENS_ROOT directory")

add_subdirectory(
  ${CMAKE_CURRENT_SOURCE_DIR}/libscws
  ${CMAKE_CURRENT_BINARY_DIR}/libscws)

set(SCWS_GET_TOKENS_API_INCLUDES
  ${SCWS_GET_TOKENS_ROOT}/
  CACHE STRING "SCWS_GET_TOKENS_API_INCLUDES path")

include_directories(
  ${libscws_API_INCLUDES}
  ${SCWS_GET_TOKENS_API_INCLUDES})

file(GLOB SCWS_GET_TOKENS_HEADER_FILES ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB SCWS_GET_TOKENS_SOURCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME}
  ${SCWS_GET_TOKENS_SOURCE_FILES}
  ${SCWS_GET_TOKENS_HEADER_FILES})

target_link_libraries(${PROJECT_NAME}
  scws)

