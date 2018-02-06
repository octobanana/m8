#!/usr/bin/env bash
set -e

# source environment variables
source ./env.sh

DIR_INSTALL=""
BUILD_TYPE="Debug"

# if [[ $# == 2 ]]; then
  # DIR_INSTALL=${2}
if [[ $# == 1 ]]; then
  if [[ $1 == "-d" ]]; then
    BUILD_TYPE="Debug"
  elif [[ $1 == "-r" ]]; then
    BUILD_TYPE="Release"
  fi
else
  # printf "\nusage: install.sh [-d|-r] [install-path]\n"
  printf "\nusage: install.sh [-d|-r]\n"
  exit
fi

if [[ ${BUILD_TYPE} == "Debug" ]]; then
  printf "\nBuilding ${APP}\n"

  printf "\nCompiling ${APP}\n"
  mkdir -p build/debug
  cd build/debug
  cmake ../../ -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
  time make

  printf "\nInstalling ${APP}\n"
  make install
  cd ../../

elif [[ ${BUILD_TYPE} == "Release" ]]; then
  printf "\nBuilding ${APP}\n"

  printf "\nCompiling ${APP}\n"
  mkdir -p build/release
  cd build/release
  cmake ../../ -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
  time make

  printf "\nInstalling ${APP}\n"
  make install
  cd ../../
fi
