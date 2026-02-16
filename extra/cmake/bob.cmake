set(BUILD_WITH_BOB_DEFAULT TRUE)

option(BUILD_WITH_BOB "bob Enabled" ${BUILD_WITH_BOB_DEFAULT})
message("BUILD_WITH_BOB: ${BUILD_WITH_BOB}")

if(BUILD_WITH_BOB)
  set(BOB_DIR ${THIRDPARTY_DIR}/bob)
  set(BOB_FILES
    ${BOB_DIR}/framebuffer.cpp
    ${BOB_DIR}/framebuffer.h)

  add_library(bob STATIC ${BOB_FILES})
  target_include_directories(bob INTERFACE ${BOB_DIR})
endif()