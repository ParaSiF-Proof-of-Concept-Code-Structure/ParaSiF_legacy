set(_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
list(INSERT CMAKE_MODULE_PATH 0 "/home/omar/WORK/OpenFOAM/OpenFOAM-v2206/ThirdParty-v2206/sources/adios/ADIOS2-2.7.1/cmake")

if(TRUE)
  set(EVPath_DIR /home/omar/WORK/OpenFOAM/OpenFOAM-v2206/ThirdParty-v2206/build/linux64Gcc/ADIOS2-2.7.1/thirdparty/EVPath/EVPath)
  if(NOT ON)
    set(atl_DIR /home/omar/WORK/OpenFOAM/OpenFOAM-v2206/ThirdParty-v2206/build/linux64Gcc/ADIOS2-2.7.1/thirdparty/atl/atl)
    set(dill_DIR /home/omar/WORK/OpenFOAM/OpenFOAM-v2206/ThirdParty-v2206/build/linux64Gcc/ADIOS2-2.7.1/thirdparty/dill/dill)
    set(ffs_DIR /home/omar/WORK/OpenFOAM/OpenFOAM-v2206/ThirdParty-v2206/build/linux64Gcc/ADIOS2-2.7.1/thirdparty/ffs/ffs)
    set(enet_DIR /home/omar/WORK/OpenFOAM/OpenFOAM-v2206/ThirdParty-v2206/build/linux64Gcc/ADIOS2-2.7.1/thirdparty/enet/enet)
  endif()
endif()

set(${CMAKE_FIND_PACKAGE_NAME}_CONFIG "${CMAKE_CURRENT_LIST_FILE}")
include("${CMAKE_CURRENT_LIST_DIR}/adios2-config-common.cmake")

set(CMAKE_MODULE_PATH ${_CMAKE_MODULE_PATH})
unset(_CMAKE_MODULE_PATH)
