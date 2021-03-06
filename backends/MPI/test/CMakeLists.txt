project(zoltanintegration_test)
cmake_minimum_required(VERSION 2.8)

find_package(Boost REQUIRED COMPONENTS unit_test_framework)
add_definitions(-DBOOST_TEST_DYN_LINK)

file(GLOB test_src "src/*.cpp")
file(GLOB test_inc "include/*.hpp")

find_package( Zoltan REQUIRED )
find_package( MPI REQUIRED )

include_directories( "include"
    ${Zoltan_INCLUDE_DIRS}
    ${MPI_INCLUDE_PATH}
    "../../serial/include" "/usr/include/eigen3" "../include" )

add_executable(RuntimeMPI_test "src/SubGridBuilderTest.cpp" "src/zoltanIntegration.cpp"
                                      "src/generatedCodeExamples.cpp" "src/RuntimeMPITest.cpp"
                                      ${test_inc} )

add_executable( subgridvalidator "src/subgridvalidator.cpp" )

target_link_libraries(RuntimeMPI_test equelle_mpi equelle_rt
    ${MPI_C_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    ${Boost_LIBRARIES}
    ${Zoltan_LIBRARIES}
    opmautodiff opmcore dunecommon )

target_link_libraries(subgridvalidator equelle_mpi equelle_rt
    ${MPI_C_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    ${Boost_LIBRARIES}
    ${Zoltan_LIBRARIES}
    opmautodiff opmcore dunecommon )
