project(scws)

set(LIBSCWS_ROOT ${PROJECT_SOURCE_DIR}
  CACHE STRING "LIBSCWS_ROOT directory")

set(libscws_API_INCLUDES
  ${LIBSCWS_ROOT}/Implementation
  CACHE STRING "libscws_API_INCLUDES path")

include_directories(
  ${libscws_API_INCLUDES})

file(GLOB LIBSCWS_HEADER_FILES ${PROJECT_SOURCE_DIR}/Implementation/*.h)
file(GLOB LIBSCWS_SOURCE_FILES ${PROJECT_SOURCE_DIR}/Implementation/*.c)

list(APPEND LIBSCWS_FILES
  ${LIBSCWS_SOURCE_FILES})

add_library(${PROJECT_NAME} STATIC ${LIBSCWS_FILES} ${LIBSCWS_HEADER_FILES})

