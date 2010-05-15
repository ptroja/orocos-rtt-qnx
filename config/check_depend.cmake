# Modules path (for searching FindXXX.cmake files)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/config")

SET(CMAKE_VERSION "${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")
MESSAGE(STATUS "CMAKE_VERSION: ${CMAKE_VERSION}")


###########################################################
#                                                         #
# Some global options we need to set here                 #
#                                                         #
###########################################################
#
# STATIC or SHARED
#
OPTION( BUILD_STATIC "Build Orocos RTT as a static library." ${FORCE_BUILD_STATIC})
#
# CORBA
#
OPTION( ENABLE_CORBA "Enable CORBA" OFF)
IF (NOT CORBA_IMPLEMENTATION)
  SET( CORBA_IMPLEMENTATION "TAO" CACHE STRING "The implementation of CORBA to use (allowed values: TAO or OMNIORB )" )
ENDIF (NOT CORBA_IMPLEMENTATION)
#
# CORBA Remote Methods in C++
#
OPTION( ORO_REMOTING "Enable transparant Remote Methods and Commands in C++" ${ENABLE_CORBA} )
# Force remoting when CORBA is enabled.
IF ( ENABLE_CORBA AND NOT ORO_REMOTING )
  MESSAGE( "Forcing ORO_REMOTING to ON")
  SET( ORO_REMOTING ON CACHE BOOL "Enable transparant Remote Methods and Commands in C++" FORCE)
ENDIF( ENABLE_CORBA AND NOT ORO_REMOTING )

# Is modified by target selection below
OPTION(OS_NO_ASM "Do not use any assembler instruction, but stick to ISO C++ as much as possible. This will exclude lock-free and atomic algorithms." OFF )
if (OS_NO_ASM AND Boost_VERSION LESS 103600)
  message(SEND_ERROR "OS_NO_ASM was turned on, but this requires Boost v1.36.0 or newer.")
endif()

###########################################################
#                                                         #
# Look for dependencies required by individual components #
#                                                         #
###########################################################

#Hack: remove our own FindBoost.cmake if cmake < 2.6.2
if( ${CMAKE_MINOR_VERSION} EQUAL 6 AND ${CMAKE_PATCH_VERSION} LESS 2)
  execute_process( COMMAND cmake -E copy FindBoost.cmake FindBoost.cmake.bak WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/config" OUTPUT_QUIET ERROR_QUIET)
  execute_process( COMMAND cmake -E remove -f FindBoost.cmake WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/config" OUTPUT_QUIET ERROR_QUIET)
endif()

# Look for boost
find_package(Boost 1.36 REQUIRED)

if(Boost_FOUND)
  message("Boost found in ${Boost_INCLUDE_DIR}")
  list(APPEND OROCOS-RTT_INCLUDE_DIRS ${Boost_INCLUDE_DIR} )
  # We don't link with boost here. It depends on the options set by the user.
  #list(APPEND OROCOS-RTT_LIBRARIES ${Boost_LIBRARIES} )
endif()

# Look for Xerces 

# If a nonstandard path is used when crosscompiling, uncomment the following lines
# IF(NOT CMAKE_CROSS_COMPILE) # NOTE: There now exists a standard CMake variable named CMAKE_CROSSCOMPILING
#   set(XERCES_ROOT_DIR /path/to/xerces CACHE INTERNAL "" FORCE) # you can also use set(ENV{XERCES_ROOT_DIR} /path/to/xerces)
# ENDIF(NOT CMAKE_CROSS_COMPILE)

find_package(Xerces)

if(XERCES_FOUND)
  set(OROPKG_SUPPORT_XERCES_C TRUE CACHE INTERNAL "" FORCE)
  list(APPEND OROCOS-RTT_INCLUDE_DIRS ${XERCES_INCLUDE_DIRS} )
  list(APPEND OROCOS-RTT_LIBRARIES ${XERCES_LIBRARIES} ) 
  set(ORODAT_CORELIB_PROPERTIES_MARSHALLING_INCLUDE "\"marsh/CPFMarshaller.hpp\"")
  set(OROCLS_CORELIB_PROPERTIES_MARSHALLING_DRIVER "CPFMarshaller")
  set(ORODAT_CORELIB_PROPERTIES_DEMARSHALLING_INCLUDE "\"marsh/CPFDemarshaller.hpp\"")
  set(OROCLS_CORELIB_PROPERTIES_DEMARSHALLING_DRIVER "CPFDemarshaller")
else(XERCES_FOUND)
  set(OROPKG_SUPPORT_XERCES_C FALSE CACHE INTERNAL "" FORCE)
  set(ORODAT_CORELIB_PROPERTIES_MARSHALLING_INCLUDE "\"marsh/CPFMarshaller.hpp\"")
  set(OROCLS_CORELIB_PROPERTIES_MARSHALLING_DRIVER "CPFMarshaller")
  set(ORODAT_CORELIB_PROPERTIES_DEMARSHALLING_INCLUDE "\"marsh/TinyDemarshaller.hpp\"")
  set(OROCLS_CORELIB_PROPERTIES_DEMARSHALLING_DRIVER "TinyDemarshaller")
endif(XERCES_FOUND)

# Check for OS/Target specific dependencies:
message("Orocos target is ${OROCOS_TARGET}")
string(TOUPPER ${OROCOS_TARGET} OROCOS_TARGET_CAP)

# Setup flags for RTAI/LXRT
if(OROCOS_TARGET STREQUAL "lxrt")
  set(OROPKG_OS_LXRT TRUE CACHE INTERNAL "This variable is exported to the rtt-config.h file to expose our target choice to the code." FORCE)
  set(LINUX_SOURCE_DIR ${LINUX_SOURCE_DIR} CACHE PATH "Path to Linux source dir (required for lxrt target)" FORCE)
  set(OS_HAS_TLSF TRUE)

  find_package(RTAI REQUIRED)
  find_package(Pthread REQUIRED)


  if(RTAI_FOUND)
    list(APPEND OROCOS-RTT_INCLUDE_DIRS ${RTAI_INCLUDE_DIRS} ${PTHREAD_INCLUDE_DIRS})
    list(APPEND OROCOS-RTT_LIBRARIES ${RTAI_LIBRARIES} ${PTHREAD_LIBRARIES}) 
    list(APPEND OROCOS-RTT_DEFINITIONS "OROCOS_TARGET=${OROCOS_TARGET}") 
  endif()
else()
  set(OROPKG_OS_LXRT FALSE CACHE INTERNAL "" FORCE)
endif()

# Setup flags for Xenomai
if(OROCOS_TARGET STREQUAL "xenomai")
  set(OROPKG_OS_XENOMAI TRUE CACHE INTERNAL "This variable is exported to the rtt-config.h file to expose our target choice to the code." FORCE)
  set(OS_HAS_TLSF TRUE)

  find_package(Xenomai REQUIRED)
  find_package(Pthread REQUIRED)
  find_package(XenomaiPosix)

  if(XENOMAI_FOUND)
    list(APPEND OROCOS-RTT_USER_LINK_LIBS ${XENOMAI_LIBRARIES} ) # For libraries used in inline (fosi/template) code.
    list(APPEND OROCOS-RTT_INCLUDE_DIRS ${XENOMAI_INCLUDE_DIRS} ${PTHREAD_INCLUDE_DIRS})
    list(APPEND OROCOS-RTT_LIBRARIES ${XENOMAI_LIBRARIES} ${PTHREAD_LIBRARIES}) 
    list(APPEND OROCOS-RTT_DEFINITIONS "OROCOS_TARGET=${OROCOS_TARGET}") 
    if (XENOMAI_POSIX_FOUND)
      set(MQ_LDFLAGS ${XENOMAI_POSIX_LDFLAGS} )
      set(MQ_CFLAGS ${XENOMAI_POSIX_CFLAGS} )
      set(MQ_INCLUDE_DIRS ${XENOMAI_POSIX_INCLUDE_DIRS})
      set(MQ_LIBRARIES ${XENOMAI_POSIX_LIBRARIES})
    endif()
  endif()
else()
  set(OROPKG_OS_XENOMAI FALSE CACHE INTERNAL "" FORCE)
endif()

# Setup flags for GNU/Linux
if(OROCOS_TARGET STREQUAL "gnulinux")
  set(OROPKG_OS_GNULINUX TRUE CACHE INTERNAL "This variable is exported to the rtt-config.h file to expose our target choice to the code." FORCE)
  set(OS_HAS_TLSF TRUE)

  find_package(Pthread REQUIRED)

  list(APPEND OROCOS-RTT_INCLUDE_DIRS ${PTHREAD_INCLUDE_DIRS})
  list(APPEND OROCOS-RTT_USER_LINK_LIBS ${PTHREAD_LIBRARIES} rt) # For libraries used in inline (fosi/template) code.

  list(APPEND OROCOS-RTT_LIBRARIES ${PTHREAD_LIBRARIES} rt) 
  list(APPEND OROCOS-RTT_DEFINITIONS "OROCOS_TARGET=${OROCOS_TARGET}") 
else()
  set(OROPKG_OS_GNULINUX FALSE CACHE INTERNAL "" FORCE)
endif()

# Setup flags for QNX
if(OROCOS_TARGET STREQUAL "qnxnto")
  set(OROPKG_OS_QNXNTO TRUE CACHE INTERNAL "This variable is exported to the rtt-config.h file to expose our target choice to the code." FORCE)
  set(OS_HAS_TLSF TRUE)
  set(CMAKE_C_FLAGS "-D_QNX_SOURCE -Wc,-std=c99")
  set(CMAKE_SHARED_LINKER_FLAGS "-lm")
  include_directories ("${Boost_INCLUDE_DIRS}/boost/compatibility/cpp_c_headers")

  list(APPEND OROCOS-RTT_DEFINITIONS "OROCOS_TARGET=${OROCOS_TARGET}") 
else()
  set(OROPKG_OS_QNXNTO FALSE CACHE INTERNAL "" FORCE)
endif()

# Setup flags for Mac-OSX
if(OROCOS_TARGET STREQUAL "macosx")
  set(OROPKG_OS_MACOSX TRUE CACHE INTERNAL "This variable is exported to the rtt-config.h file to expose our target choice to the code." FORCE)
  set(OS_HAS_TLSF TRUE)

  find_package(Boost 1.36 REQUIRED thread)
  list(APPEND OROCOS-RTT_INCLUDE_DIRS ${Boost_THREAD_INCLUDE_DIRS} )

  message( "Forcing ORO_OS_USE_BOOST_THREAD to ON")
  set( ORO_OS_USE_BOOST_THREAD ON CACHE BOOL "Forced enable use of Boost.thread on macosx." FORCE)

  list(APPEND OROCOS-RTT_LIBRARIES ${PTHREAD_LIBRARIES}) 
  list(APPEND OROCOS-RTT_DEFINITIONS "OROCOS_TARGET=${OROCOS_TARGET}") 
else()
  set(OROPKG_OS_MACOSX FALSE CACHE INTERNAL "" FORCE)
endif()


# Setup flags for ecos
if(OROCOS_TARGET STREQUAL "ecos")
  set(OROPKG_OS_ECOS TRUE CACHE INTERNAL "This variable is exported to the rtt-config.h file to expose our target choice to the code." FORCE)
  set(OS_HAS_TLSF FALSE)

  # We can't really use 'UseEcos.cmake' because we're building a library
  # and not a final application
  find_package(Ecos REQUIRED)

  if(Ecos_FOUND)

    set(ECOS_SUPPORT TRUE CACHE INTERNAL "" FORCE)

    list(APPEND OROCOS-RTT_INCLUDE_DIRS ${ECOS_INCLUDE_DIRS} )
    list(APPEND OROCOS-RTT_LIBRARIES ${ECOS_LIBRARIES} pthread) 
    list(APPEND OROCOS-RTT_DEFINITIONS "OROCOS_TARGET=${OROCOS_TARGET}") 

    message( "Turning BUILD_STATIC ON for ecos.")
    set( FORCE_BUILD_STATIC ON CACHE INTERNAL "Forces to build Orocos RTT as a static library (forced to ON by Ecos)" FORCE)
    set( BUILD_STATIC ON CACHE BOOL "Build Orocos RTT as a static library (forced to ON by Ecos)" FORCE)
  endif()
else()
  set(OROPKG_OS_ECOS FALSE CACHE INTERNAL "" FORCE)
endif()

if(OROCOS_TARGET STREQUAL "win32")
  set(OROPKG_OS_WIN32 TRUE CACHE INTERNAL "" FORCE)
  message("Forcing OS_HAS_TLSF to OFF for WIN32")
  set(OS_HAS_TLSF FALSE)
  # Force OFF on mqueue transport on WIN32 platform
  message("Forcing ENABLE_MQ to OFF for WIN32")
  set(ENABLE_MQ OFF CACHE BOOL "This option is forced to OFF by the build system on WIN32 platform." FORCE)
  if (MINGW)
    #--enable-all-export and --enable-auto-import are already set by cmake.
    #but we need it here for the unit tests as well.
    set(CMAKE_LD_FLAGS_ADD "-Wl,--enable-auto-import" CACHE INTERNAL "")
  endif()
  if (MSVC)
    if (NOT MSVC80)
    set(NUM_PARALLEL_BUILD 4 CACHE STRING "Number of parallel builds")
    set(PARALLEL_FLAG "/MP${NUM_PARALLEL_BUILD}")
    endif()
    set(CMAKE_CXX_FLAGS_ADD "/wd4355 /wd4251 /wd4180 /wd4996 /bigobj ${PARALLEL_FLAG}")
    list(APPEND OROCOS-RTT_LIBRARIES kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib  ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib Ws2_32.lib winmm.lib)
    # For boost::intrusive !
    find_package(Boost 1.36 REQUIRED)
  endif()
  list(APPEND OROCOS-RTT_DEFINITIONS "OROCOS_TARGET=${OROCOS_TARGET}") 
else(OROCOS_TARGET STREQUAL "win32")
  set(OROPKG_OS_WIN32 FALSE CACHE INTERNAL "" FORCE)
endif(OROCOS_TARGET STREQUAL "win32")

if( NOT OROCOS-RTT_DEFINITIONS )
  message(FATAL_ERROR "No suitable OROCOS_TARGET selected. Use one of 'lxrt,xenomai,gnulinux,macosx,win32'")
endif()

# The machine type is tested using compiler macros in rtt-config.h.in
# Add found include dirs.
INCLUDE_DIRECTORIES( ${OROCOS-RTT_INCLUDE_DIRS} )

#
# Disable line wrapping for gcc/g++ such that eclipse can parse the errors.
#
IF(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCC)
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

#
# If we're using gcc, make sure the version is OK.
#
IF (CMAKE_COMPILER_IS_GNUCXX)
  # this is a workaround distcc:
  IF ( CMAKE_CXX_COMPILER_ARG1 )
    STRING(REPLACE " " "" CMAKE_CXX_COMPILER_ARG1 ${CMAKE_CXX_COMPILER_ARG1} )
    #MESSAGE("Invoking: '${CMAKE_CXX_COMPILER_ARG1} -dumpversion'")
    EXECUTE_PROCESS( COMMAND ${CMAKE_CXX_COMPILER_ARG1} -dumpversion RESULT_VARIABLE CXX_HAS_VERSION OUTPUT_VARIABLE CXX_VERSION)
  ELSE ( CMAKE_CXX_COMPILER_ARG1 )
    #MESSAGE("Invoking: ${CMAKE_CXX_COMPILER} -dumpversion")
    EXECUTE_PROCESS( COMMAND ${CMAKE_CXX_COMPILER} -dumpversion RESULT_VARIABLE CXX_HAS_VERSION OUTPUT_VARIABLE CXX_VERSION)
  ENDIF ( CMAKE_CXX_COMPILER_ARG1 )

  IF ( ${CXX_HAS_VERSION} EQUAL 0 )
    # We are assuming here that -dumpversion is gcc specific.
    IF( CXX_VERSION MATCHES "4\\.[0-9](\\.[0-9])?" )
      MESSAGE(STATUS "Detected gcc4: ${CXX_VERSION}")
      SET(RTT_GCC_HASVISIBILITY TRUE)
    ELSE(CXX_VERSION MATCHES "4\\.[0-9](\\.[0-9])?")
      IF( CXX_VERSION MATCHES "3\\.[0-9](\\.[0-9])?" )
	MESSAGE(STATUS "Detected gcc3: ${CXX_VERSION}")
      ELSE( CXX_VERSION MATCHES "3\\.[0-9](\\.[0-9])?" )
	MESSAGE("ERROR: You seem to be using gcc version:")
	MESSAGE("${CXX_VERSION}")
	MESSAGE( FATAL_ERROR "ERROR: For gcc, Orocos requires version 4.x or 3.x")
      ENDIF( CXX_VERSION MATCHES "3\\.[0-9](\\.[0-9])?" )
    ENDIF(CXX_VERSION MATCHES "4\\.[0-9](\\.[0-9])?")
  ELSE ( ${CXX_HAS_VERSION} EQUAL 0)
    MESSAGE("Could not determine gcc version: ${CXX_HAS_VERSION}")
  ENDIF ( ${CXX_HAS_VERSION} EQUAL 0)
ENDIF()

#
# Check for Doxygen and enable documentation building
#
find_package( Doxygen )
IF ( DOXYGEN_EXECUTABLE )
  MESSAGE( STATUS "Found Doxygen -- API documentation can be built" )
ELSE ( DOXYGEN_EXECUTABLE )
  MESSAGE( STATUS "Doxygen not found -- unable to build documentation" )
ENDIF ( DOXYGEN_EXECUTABLE )

#
# Detect CORBA using user's CORBA_IMPLEMENTATION
#
find_package(Corba REQUIRED)

