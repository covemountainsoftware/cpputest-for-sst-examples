include_directories(include)
add_subdirectory(test)
add_library(hwLockCtrlService include/hwLockCtrlService.hpp src/hwLockCtrlService.cpp)
target_link_libraries(hwLockCtrlService hwLockCtrl)
target_include_directories(hwLockCtrlService PUBLIC include)

# TODO: setting up QP should probably be a higher level build option
include_directories(${SST_INCLUDE_DIR})
include_directories(${CMS_SST_SUPPORT_INCLUDE_DIR})
include(${CMS_CMAKE_DIR}/sstCMakeSupport.cmake)
