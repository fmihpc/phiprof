module phiprof
  use, intrinsic :: ISO_C_BINDING
  use :: mpi
  implicit none

  interface  
     function phiprof_initialize_c() bind(C,name='phiprof_initialize') result(stat)
       ! the C interface is int phiprof_initialize()
       use, intrinsic :: ISO_C_BINDING
       implicit none
       integer(kind=C_INT) :: stat
     end function phiprof_initialize_c


     function phiprof_initializeTimer_c(label) bind(C,name='phiprof_initializeTimer') result(id)
       ! the C interface is int phiprof_initializeTimer(char *label)
       use, intrinsic :: ISO_C_BINDING
       implicit none
       character(kind=C_CHAR), intent(in) :: label(*)   
       integer(kind=C_INT) :: id
     end function phiprof_initializeTimer_C
     
     function phiprof_initializeTimerWithGroups1_c(label, group1) &
          bind(C,name='phiprof_initializeTimerWithGroups1') result(id)
       ! the C interface is int phiprof_initializeTimer(char *label)
       use, intrinsic :: ISO_C_BINDING
       implicit none
       character(kind=C_CHAR), intent(in) :: label(*)   
       character(kind=C_CHAR), intent(in) :: group1(*)   
       integer(kind=C_INT) :: id
     end function phiprof_initializeTimerWithGroups1_c

     function phiprof_initializeTimerWithGroups2_c(label, group1, group2) &
          bind(C,name='phiprof_initializeTimerWithGroups2') result(id)
       ! the C interface is int phiprof_initializeTimer(char *label)
       use, intrinsic :: ISO_C_BINDING
       implicit none
       character(kind=C_CHAR), intent(in) :: label(*)   
       character(kind=C_CHAR), intent(in) :: group1(*)   
       character(kind=C_CHAR), intent(in) :: group2(*)   
       integer(kind=C_INT) :: id
     end function phiprof_initializeTimerWithGroups2_c

     function phiprof_initializeTimerWithGroups3_c(label, group1, group2, group3) &
          bind(C,name='phiprof_initializeTimerWithGroups3') result(id)
       ! the C interface is int phiprof_initializeTimer(char *label)
       use, intrinsic :: ISO_C_BINDING
       implicit none
       character(kind=C_CHAR), intent(in) :: label(*)   
       character(kind=C_CHAR), intent(in) :: group1(*)   
       character(kind=C_CHAR), intent(in) :: group2(*)   
       character(kind=C_CHAR), intent(in) :: group3(*)   
       integer(kind=C_INT) :: id
     end function phiprof_initializeTimerWithGroups3_c


          
     function phiprof_getChildId_c(label) bind(C,name='phiprof_getChildId') result(id)
       ! the C interface is int phiprof_getId(char *label);
       use, intrinsic :: ISO_C_BINDING
       implicit none
       character(kind=C_CHAR), intent(in) :: label(*)   
       integer(kind=C_INT) :: id
     end function phiprof_getChildId_c

     function phiprof_start_c(label) bind(C,name='phiprof_start') result(stat)
       ! the C interface is int phiprof_start(char *label);
       use, intrinsic :: ISO_C_BINDING
       implicit none
       character(kind=C_CHAR), intent(in) :: label(*)   
       integer(kind=C_INT) :: stat
     end function phiprof_start_c

     function phiprof_stop_c(label) bind(C,name='phiprof_stop') result(stat)
       ! the C interface is int phiprof_stop(char *label);
       use, intrinsic :: ISO_C_BINDING
       implicit none
       character(kind=C_CHAR), intent(in) :: label(*)   
       integer(kind=C_INT) :: stat
     end function phiprof_stop_c

     function phiprof_stopUnits_c(name, units, unitName) bind(C,name='phiprof_stopUnits') result(stat)
       ! the C interface is int phiprof_stopUnits(char *name,double units,char *unitName);
       use, intrinsic :: ISO_C_BINDING
       implicit none
       character(kind=C_CHAR), intent(in) :: name(*)   
       real(kind=C_DOUBLE), value, intent(in) :: units
       character(kind=C_CHAR), intent(in) :: unitName(*) 
       integer(kind=C_INT) :: stat
     end function phiprof_stopUnits_c

     function phiprof_startId_c(id) bind(C,name='phiprof_startId') result(stat)
       ! the C interface is int phiprof_startId(int id);
       use, intrinsic :: ISO_C_BINDING
       implicit none
       integer(kind=C_INT), value, intent(in) :: id
       integer(kind=C_INT) :: stat
     end function phiprof_startId_c

     function phiprof_stopId_c(id) bind(C,name='phiprof_stopId') result(stat)
       ! the C interface is int phiprof_stopId(int id);
       use, intrinsic :: ISO_C_BINDING
       implicit none
       integer(kind=C_INT), value,  intent(in) :: id   
       integer(kind=C_INT) :: stat
     end function phiprof_stopId_c

     function phiprof_stopIdUnits_c(id, units, unitName) bind(C,name='phiprof_stopIdUnits') result(stat)
       ! the C interface is int phiprof_stopIdUnits(int id,double units,char *unitName);
       use, intrinsic :: ISO_C_BINDING
       implicit none
       integer(kind=C_INT), value, intent(in) :: id  
       real(kind=C_DOUBLE), value, intent(in) :: units
       character(kind=C_CHAR), intent(in) :: unitName(*) 
       integer(kind=C_INT) :: stat
     end function phiprof_stopIdUnits_c


     function phiprof_print_from_fortran_c(comm, fileNamePrefix) bind(C,name='phiprof_print_from_fortran') result(stat)
       ! the C interface is int phiprof_print(MPI_Comm comm, char *fileNamePrefix);
       use, intrinsic :: ISO_C_BINDING
       implicit none
       integer(c_int), value, intent(in) :: comm 
       character(kind=C_CHAR), intent(in) :: fileNamePrefix(*)
       integer(kind=C_INT) :: stat
     end function phiprof_print_from_fortran_c
  end interface
  
contains
  
  subroutine phiprof_initialize(error)
    implicit none
    integer, intent(out), optional:: error
    integer error_

    error_ = phiprof_initialize_c()
    if (present(error)) then
       error = error_
    end if
  end subroutine phiprof_initialize

  function phiprof_initializeTimer(label, group1, group2, group3) result(id)
    implicit none
    character(len=*), intent(in) :: label   
    character(len=*), intent(in), optional :: group1  
    character(len=*), intent(in), optional :: group2  
    character(len=*), intent(in), optional :: group3  
    integer:: id
    
    if (present(group1)) then
       if (present(group2)) then
          if (present(group3)) then
             id  = phiprof_initializeTimerWithGroups3_c( &
                  trim(label)//C_NULL_CHAR, &
                  trim(group1)//C_NULL_CHAR, &
                  trim(group2)//C_NULL_CHAR, &
                  trim(group3)//C_NULL_CHAR)
          else ! group3 not present
             id  = phiprof_initializeTimerWithGroups2_c( &
                  trim(label)//C_NULL_CHAR, &
                  trim(group1)//C_NULL_CHAR, &
                  trim(group2)//C_NULL_CHAR)         
          end if
       else ! group2 not present
          id  = phiprof_initializeTimerWithGroups1_c( &
                  trim(label)//C_NULL_CHAR, &
                  trim(group1)//C_NULL_CHAR)          
       end if
    else ! group1 not present
       id  = phiprof_initializeTimer_c(trim(label)//C_NULL_CHAR)
    end if
  end function phiprof_initializeTimer

  function phiprof_getChildId(label) result(id) 
    implicit none
    character(len=*), intent(in) :: label   
    integer:: id
    id = phiprof_getChildId_c(trim(label)//C_NULL_CHAR)
  end function phiprof_getChildId

  
  subroutine phiprof_start(label, error) 
    implicit none
    character(len=*), intent(in) :: label   
    integer, intent(out), optional:: error
    integer error_

    error_ = phiprof_start_c(trim(label)//C_NULL_CHAR)
    if (present(error)) then
       error = error_
    end if
  end subroutine phiprof_start


  subroutine phiprof_stop(label, error) 
    implicit none
    character(len=*), intent(in) :: label   
    integer, intent(out), optional:: error
    
    integer error_

    error_ = phiprof_stop_c(trim(label)//C_NULL_CHAR)
    
    if (present(error)) then
       error = error_
    end if
  end subroutine phiprof_stop

  

  subroutine phiprof_stopUnits(label, units, unitName, error) 
    implicit none
    character(len=*), intent(in) :: label   
    real*8, intent(in) :: units
    character(len=*), intent(in) :: unitName   
    integer, intent(out), optional:: error

    integer error_

    error_ = phiprof_stopUnits_c(trim(label)//C_NULL_CHAR, units, trim(unitName)//C_NULL_CHAR)
    
    if (present(error)) then
       error = error_
    end if
  end subroutine phiprof_stopUnits
  
  
  
  subroutine phiprof_startId(id, error) 
    implicit none
    integer, intent(in) :: id
    integer, intent(out), optional:: error
    integer error_

    error_ = phiprof_startId_c(id)
    if (present(error)) then
       error = error_
    end if
  end subroutine phiprof_startId


  
  subroutine phiprof_stopId(id, error) 
    implicit none
    integer, intent(in) :: id
    integer, intent(out), optional:: error
    integer error_

    error_ = phiprof_stopId_c(id)
    if (present(error)) then
       error = error_
    end if
  end subroutine phiprof_stopId

  
  

  subroutine phiprof_stopIdUnits(id, units, unitName, error) 
    implicit none
    integer, intent(in) :: id
    real*8, intent(in) :: units
    character(len=*), intent(in) :: unitName   
    integer, intent(out), optional:: error

    integer error_
    error_ = phiprof_stopIdUnits_c(id, units, trim(unitName)//C_NULL_CHAR)
    
    if (present(error)) then
       error = error_
    end if
  end subroutine phiprof_stopIdUnits




  subroutine phiprof_print(comm, fileNamePrefix, error) 
    implicit none
    integer, intent(in) :: comm
    character(len=*), intent(in) :: fileNamePrefix   
    integer, intent(out), optional:: error
    integer error_
    
    error_ = phiprof_print_from_fortran_c(comm, trim(fileNamePrefix)//C_NULL_CHAR)    
    if (present(error)) then
       error = error_
    end if
  end subroutine phiprof_print



  

end module phiprof




