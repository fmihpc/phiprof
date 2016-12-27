module phiprof
  Use, intrinsic :: ISO_C_BINDING
  use :: mpi
  implicit none

  interface  
     function phiprof_initialize() bind(C,name='phiprof_initialize') result(stat)
       ! the C interface is int phiprof_initialize()
       use, intrinsic :: ISO_C_BINDING
       implicit none
       integer(kind=C_INT) :: stat
     end function phiprof_initialize


     function phiprof_initializeTimer(label) bind(C,name='phiprof_initializeTimer') result(stat)
       ! the C interface is int phiprof_initializeTimer(char *label)
       use, intrinsic :: ISO_C_BINDING
       implicit none
       character(kind=C_CHAR), intent(inout) :: label(*)   
       integer(kind=C_INT) :: stat
     end function phiprof_initializeTimer
     
     function phiprof_getId(label) bind(C,name='phiprof_getId') result(stat)
       ! the C interface is int phiprof_getId(char *label);
       use, intrinsic :: ISO_C_BINDING
       implicit none
       character(kind=C_CHAR), intent(inout) :: label(*)   
       integer(kind=C_INT) :: stat
     end function phiprof_getId

     function phiprof_start(label) bind(C,name='phiprof_start') result(stat)
       ! the C interface is int phiprof_start(char *label);
       use, intrinsic :: ISO_C_BINDING
       implicit none
       character(kind=C_CHAR), intent(in) :: label(*)   
       integer(kind=C_INT) :: stat
     end function phiprof_start

     function phiprof_stop(label) bind(C,name='phiprof_stop') result(stat)
       ! the C interface is int phiprof_stop(char *label);
       use, intrinsic :: ISO_C_BINDING
       implicit none
       character(kind=C_CHAR), intent(in) :: label(*)   
       integer(kind=C_INT) :: stat
     end function phiprof_stop

     function phiprof_stopUnits(name, units, unitName) bind(C,name='phiprof_stopUnits') result(stat)
       ! the C interface is int phiprof_stopUnits(char *name,double units,char *unitName);
       use, intrinsic :: ISO_C_BINDING
       implicit none
       character(kind=C_CHAR), intent(in) :: name(*)   
       real(kind=C_DOUBLE), value, intent(in) :: units
       character(kind=C_CHAR), intent(in) :: unitName(*) 
       integer(kind=C_INT) :: stat
     end function phiprof_stopUnits

     function phiprof_startId(id) bind(C,name='phiprof_startId') result(stat)
       ! the C interface is int phiprof_startId(int id);
       use, intrinsic :: ISO_C_BINDING
       implicit none
       integer(kind=C_INT), value, intent(in) :: id
       integer(kind=C_INT) :: stat
     end function phiprof_startId

     function phiprof_stopId(id) bind(C,name='phiprof_stopId') result(stat)
       ! the C interface is int phiprof_stopId(int id);
       use, intrinsic :: ISO_C_BINDING
       implicit none
       integer(kind=C_INT), value,  intent(in) :: id   
       integer(kind=C_INT) :: stat
     end function phiprof_stopId

     function phiprof_stopIdUnits(id, units, unitName) bind(C,name='phiprof_stopIdUnits') result(stat)
       ! the C interface is int phiprof_stopIdUnits(int id,double units,char *unitName);
       use, intrinsic :: ISO_C_BINDING
       implicit none
       integer(kind=C_INT), value, intent(in) :: id  
       real(kind=C_DOUBLE), value, intent(in) :: units
       character(kind=C_CHAR), intent(in) :: unitName(*) 
       integer(kind=C_INT) :: stat
     end function phiprof_stopIdUnits


     function phiprof_print(comm, fileNamePrefix) bind(C,name='phiprof_print_from_fortran') result(stat)
       ! the C interface is int phiprof_print(MPI_Comm comm, char *fileNamePrefix);
       use, intrinsic :: ISO_C_BINDING
       implicit none
       integer(c_int), value, intent(in) :: comm 
       character(kind=C_CHAR), intent(in) :: fileNamePrefix(*)
       integer(kind=C_INT) :: stat
     end function phiprof_print



  end interface

end module phiprof




