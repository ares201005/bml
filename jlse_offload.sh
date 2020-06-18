#!/bin/bash

# Make sure all the paths are correct

rm -r build
rm -r install

MY_PATH=$(pwd)

if [ -n "$BML_SDK_VENDOR" ]
then

if [ "$BML_SDK_VENDOR" = "GNU" ]
then

export CC=${CC:=gcc}
export FC=${FC:=gfortran}
export CXX=${CXX:=g++}

elif [ "$BML_SDK_VENDOR" = "INTEL" ]
then

export CC=${CC:=icc}
export FC=${FC:=ifort}
export CXX=${CXX:=icpc}

elif [ "$BML_SDK_VENDOR" = "INTEL_CLANG" ]
then

export CC=${CC:=icx}
export FC=${FC:=./ifx}
export CXX=${CXX:=icpx}

fi
else
export CC=${CC:=gcc}
export FC=${FC:=gfortran}
export CXX=${CXX:=g++}
fi
export BLAS_VENDOR=${BLAS_VENDOR:=MKL}
export BML_OPENMP=${BML_OPENMP:=yes}
export BML_GPU=${BML_GPU:=no}
export INSTALL_DIR=${INSTALL_DIR:="${MY_PATH}/install"}
export BML_TESTING=${BML_TESTING:=yes}
export CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE:=Release}
export EXTRA_CFLAGS=${EXTRA_CFLAGS:=""}
export EXTRA_LINK_FLAGS=${EXTRA_LINK_FLAGS:="-L. -latomic"}
#export CMAKE_Fortran_FLAGS="-fiopenmp -fopenmp-targets=spir64"
export CMAKE_Fortran_FLAGS=""
export CMAKE_C_FLAGS="-fiopenmp -fopenmp-targets=spir64 -D__STRICT_ANSI__"
#export CMAKE_C_FLAGS="-D__STRICT_ANSI__"
#export CUDA_TOOLKIT_ROOT_DIR=${CUDATOOLKIT_ROOT}

./build.sh configure
cd build
make -j8
make test

                                                                                                                                                                                              
                                    
