cmake_minimum_required(VERSION 3.16)
project(cpputest-for-sst-examples)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

add_compile_options(-Wall -Wextra -Werror)

include_directories(include)

set(EXTERNALS_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals)

# set CMS_QPCPP_TOP_DIR to avoid cpputest-for-qpcpp-lib from fetching its own copy of QP/C++
set(CMS_SST_TOP_DIR ${EXTERNALS_TOP_DIR}/sst)
set(CMS_CPPUTEST_SST_TOP_DIR ${EXTERNALS_TOP_DIR}/cpputest-for-sst)

add_subdirectory(externals/cpputest-for-sst)

set(CMS_TEST_SUPPORT_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test_support)
set(MOCKS_TOP_DIR ${CMS_TEST_SUPPORT_TOP_DIR}/mocks)
set(DRIVERS_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/drivers)

set(SST_INCLUDE_DIR ${CMS_SST_TOP_DIR}/include)

set(CMS_SST_INCLUDE_DIR ${CMS_SST_TOP_DIR}/include)

# handy header file only support classes, such as SstFlatStatemachineTask
set(CMS_SST_SUPPORT_INCLUDE_DIR ${CMS_CPPUTEST_SST_TOP_DIR}/cpputest-for-sst-lib/include)

add_subdirectory(drivers)
add_subdirectory(examples)
