# - Find LibSVM
# LibSVM is a Library for Support Vector Machines
# available at http://www.csie.ntu.edu.tw/~cjlin/libsvm/
#
# The module defines the following variables:
#  LIBSVM_FOUND - the system has LibSVM
#  LIBSVM_INCLUDE_DIR - where to find svm.h
#  LIBSVM_INCLUDE_DIRS - libsvm includes
#  LIBSVM_LIBRARY - where to find the LibSVM library
#  LIBSVM_LIBRARIES - aditional libraries
#  LIBSVM_MAJOR_VERSION - major version
#  LIBSVM_MINOR_VERSION - minor version
#  LIBSVM_PATCH_VERSION - patch version
#  LIBSVM_VERSION_STRING - version (ex. 2.9.0)
#  LIBSVM_ROOT_DIR - root dir (ex. /usr/local)

#=============================================================================
# Copyright 2005-2012 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)


# set LIBSVM_INCLUDE_DIR
find_path ( LIBSVM_INCLUDE_DIR
  NAMES
    svm.h
  PATHS
    ${LIBSVM_ROOT_DIR}/include
  PATH_SUFFIXES
    libsvm
    libsvm-2.0/libsvm
  DOC
    "LibSVM include directory"
)


# set LIBSVM_INCLUDE_DIRS
if ( LIBSVM_INCLUDE_DIR )
  set ( LIBSVM_INCLUDE_DIRS ${LIBSVM_INCLUDE_DIR} )
endif ()


# version
set ( _VERSION_FILE ${LIBSVM_INCLUDE_DIR}/svm.h )
if ( EXISTS ${_VERSION_FILE} )
  # LIBSVM_VERSION_STRING macro defined in svm.h since version 2.8.9
  file ( STRINGS ${_VERSION_FILE} _VERSION_STRING REGEX ".*define[ ]+LIBSVM_VERSION[ ]+[0-9]+.*" )
  if ( _VERSION_STRING )
    string ( REGEX REPLACE ".*LIBSVM_VERSION[ ]+([0-9]+)" "\\1" _VERSION_NUMBER "${_VERSION_STRING}" )
    math ( EXPR LIBSVM_MAJOR_VERSION "${_VERSION_NUMBER} / 100" )
    math ( EXPR LIBSVM_MINOR_VERSION "(${_VERSION_NUMBER} % 100 ) / 10" )
    math ( EXPR LIBSVM_PATCH_VERSION "${_VERSION_NUMBER} % 10" )
    set ( LIBSVM_VERSION_STRING "${LIBSVM_MAJOR_VERSION}.${LIBSVM_MINOR_VERSION}.${LIBSVM_PATCH_VERSION}" )
  endif ()
endif ()


# check version
set ( _LIBSVM_VERSION_MATCH TRUE )
if ( LibSVM_FIND_VERSION AND LIBSVM_VERSION_STRING )
  if ( LibSVM_FIND_VERSION_EXACT )
    if ( NOT ${LibSVM_FIND_VERSION} VERSION_EQUAL ${LIBSVM_VERSION_STRING} )
      set ( _LIBSVM_VERSION_MATCH FALSE )
    endif ()
  else ()
    if ( ${LibSVM_FIND_VERSION} VERSION_GREATER ${LIBSVM_VERSION_STRING} )
      set ( _LIBSVM_VERSION_MATCH FALSE )
    endif ()
  endif ()
endif ()


# set LIBSVM_LIBRARY
find_library ( LIBSVM_LIBRARY
  NAMES
    svm
  PATHS
    ${LIBSVM_ROOT_DIR}/lib
  DOC
    "LibSVM library location"
)


# set LIBSVM_LIBRARIES
set ( LIBSVM_LIBRARIES ${LIBSVM_LIBRARY} )

# link with math library on unix
if ( UNIX )
  list ( APPEND LIBSVM_LIBRARIES "-lm" )
endif ()


# try to guess root dir from include dir
if ( LIBSVM_INCLUDE_DIR )
  string ( REGEX REPLACE "(.*)/include.*" "\\1" LIBSVM_ROOT_DIR ${LIBSVM_INCLUDE_DIR} )
# try to guess root dir from library dir
elseif ( LIBSVM_LIBRARY )
  string ( REGEX REPLACE "(.*)/lib[/|32|64].*" "\\1" LIBSVM_ROOT_DIR ${LIBSVM_LIBRARY} )
endif ()


# handle REQUIRED and QUIET options
include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args ( LibSVM DEFAULT_MSG LIBSVM_LIBRARY
  _LIBSVM_VERSION_MATCH
  LIBSVM_INCLUDE_DIR
  LIBSVM_INCLUDE_DIRS
  LIBSVM_LIBRARIES
  LIBSVM_ROOT_DIR
)


mark_as_advanced (
  LIBSVM_LIBRARY
  LIBSVM_LIBRARIES
  LIBSVM_INCLUDE_DIR
  LIBSVM_INCLUDE_DIRS
  LIBSVM_ROOT_DIR
  LIBSVM_VERSION_STRING
  LIBSVM_MAJOR_VERSION
  LIBSVM_MINOR_VERSION
  LIBSVM_PATCH_VERSION
)
