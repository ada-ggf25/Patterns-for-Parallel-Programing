add_executable(pi_mpi pi_mpi.cpp)
target_link_libraries(pi_mpi PRIVATE MPI::MPI_CXX)
