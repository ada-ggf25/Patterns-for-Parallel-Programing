add_executable(pi_openmp pi_openmp.cpp)
target_link_libraries(pi_openmp PRIVATE OpenMP::OpenMP_CXX)
