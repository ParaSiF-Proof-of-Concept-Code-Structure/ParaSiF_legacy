# CMake generated Testfile for 
# Source directory: /home/omar/WORK/OpenFOAM/OpenFOAM-v2206/ThirdParty-v2206/sources/adios/ADIOS2-2.7.1/testing/adios2/performance/manyvars
# Build directory: /home/omar/WORK/OpenFOAM/OpenFOAM-v2206/ThirdParty-v2206/build/linux64Gcc/ADIOS2-2.7.1/testing/adios2/performance/manyvars
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(Performance.*/TestManyVars.DontRedefineVars/*.MPI "/usr/local/bin/mpiexec" "-n" "8" "/home/omar/WORK/OpenFOAM/OpenFOAM-v2206/ThirdParty-v2206/build/linux64Gcc/ADIOS2-2.7.1/bin/Test.Performance.ManyVars.MPI" "--gtest_filter=*/TestManyVars.DontRedefineVars/*")
set_tests_properties(Performance.*/TestManyVars.DontRedefineVars/*.MPI PROPERTIES  PROCESSORS "8" _BACKTRACE_TRIPLES "/home/omar/WORK/OpenFOAM/OpenFOAM-v2206/ThirdParty-v2206/sources/adios/ADIOS2-2.7.1/cmake/upstream/GoogleTest.cmake;225;add_test;/home/omar/WORK/OpenFOAM/OpenFOAM-v2206/ThirdParty-v2206/sources/adios/ADIOS2-2.7.1/testing/CMakeLists.txt;79;gtest_add_tests;/home/omar/WORK/OpenFOAM/OpenFOAM-v2206/ThirdParty-v2206/sources/adios/ADIOS2-2.7.1/testing/adios2/performance/manyvars/CMakeLists.txt;7;gtest_add_tests_helper;/home/omar/WORK/OpenFOAM/OpenFOAM-v2206/ThirdParty-v2206/sources/adios/ADIOS2-2.7.1/testing/adios2/performance/manyvars/CMakeLists.txt;0;")
