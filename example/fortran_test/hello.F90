program hello
  use phiprof
  implicit none
  integer::err
  call mpi_init(err)

  call phiprof_initialize()

  call phiprof_start("print")
  write (*,*) 'Hello world from Fortran!'
  call phiprof_stop("print")

  call phiprof_print(MPI_COMM_WORLD, "profile")
  call mpi_finalize(err)
end program hello
