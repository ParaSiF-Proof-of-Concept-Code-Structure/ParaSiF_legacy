Instructions for compiling FEniCS 2019.1.0 for ARCHER2
======================================================

These instructions are for compiling FEniCS 2019.1.0 on 
[ARCHER2](https://www.archer2.ac.uk).
FEniCS 2019.1.0 is installed with the following software:
```bash
  -GNU: 10.2.0
  -python: 3.8.5
  -cmake: 3.21.3

  -PYBIND11: v2.6.1
  -boost: v1.72.0
  -EIGEN: v3.3.9
  -pkgconfig: v1.5.5
  -hdf5: v1.10.7
  -petsc: v3.11.4
    -glm: v0.9.9.6
    -hypre: v2.18.0
    -matio: v1.5.18
    -superlu: v5.2.2
    -superlu-dist: v6.4.0
    -metis: v5.1.0
    -MUMPS: v5.3.5
    -parmetis: v4.0.3
    -scotch: v6.1.0

  -petsc4py: v3.11.0
```

Create and set the installation and build folders
---------------------------------------------
```bash
  export INSTALL_FOLDER=`pwd`
  mkdir ${INSTALL_FOLDER}/FEniCS
  mkdir ${INSTALL_FOLDER}/FEniCS/V2019.1.0

  export BUILD_DIR=${INSTALL_FOLDER}/FEniCS/V2019.1.0
```

Load modules and set python paths/build paths
---------------------------------------------

```bash
  module load PrgEnv-gnu
  module load cray-python
  module load cmake

  cd $BUILD_DIR

  export PYTHONUSERBASE=${INSTALL_FOLDER}/.local
  export PATH=$PYTHONUSERBASE/bin:$PATH

  pip install --user virtualenv
  virtualenv --version
  virtualenv --system-site-packages fenics2019_FSI

  export PATH=$PATH:$BUILD_DIR/bin
  export PATH=$PATH:$BUILD_DIR/shared/bin
  export PYTHONPATH=$PYTHONPATH:$BUILD_DIR/lib/python3.8/site-packages
  export LD_LIBRARY_PATH=$BUILD_DIR/lib:$LD_LIBRARY_PATH
  export CC=cc
  export CXX=CC
```

Download, configure and build pybind
-------------------------------------

```bash
  cd $BUILD_DIR
  export PYBIND11_VERSION=2.6.1
  wget https://github.com/pybind/pybind11/archive/v${PYBIND11_VERSION}.tar.gz
  tar zxvf v${PYBIND11_VERSION}.tar.gz
  mkdir pybind11-${PYBIND11_VERSION}/build
  cd pybind11-${PYBIND11_VERSION}/build
  cmake -DPYBIND11_TEST=off .. -DCMAKE_INSTALL_PREFIX=$(pwd) -DPYTHON_EXECUTABLE:FILEPATH=$BUILD_DIR/fenics2019_FSI/bin/python3
  make install
```

Download, configure and prepare BOOST
--------------------------------------

While BOOST is available centrally on ARCHER2, the shared libraries are not 
(and that is what we need for FEniCS/DOLFIN). For this, we will use the build 
script provided by the ARCHER2 CSE team.

```bash
  cd $BUILD_DIR
  git clone https://github.com/ARCHER2-HPC/pe-scripts.git
  mv pe-scripts boost
  cd  boost
  git checkout cse-develop
  sed -i 's/make_shared=0/make_shared=1/g' sh/.preamble.sh
  ./sh/boost.sh --prefix=$(pwd)/boost
  #if there is a problem here, download boost_1_72_0.tar.bz2 directly, and retype the previous command
```

Download, configure and install EIGEN
--------------------------------------

```bash
  cd $BUILD_DIR
  wget https://gitlab.com/libeigen/eigen/-/archive/3.3.9/eigen-3.3.9.tar.gz
  tar zxvf eigen-3.3.9.tar.gz
  mkdir eigen-3.3.9/build
  cd eigen-3.3.9/build
  cmake ../ -DCMAKE_INSTALL_PREFIX=build -DPYTHON_EXECUTABLE:FILEPATH=$BUILD_DIR/fenics2019_FSI/bin/python3
  make -j 8 install
```

Download, configure and install mpi4py
---------------------------------------------------------

```bash
  cd $BUILD_DIR
  . fenics2019_FSI/bin/activate
  pip install pkgconfig
  pip install h5py==3.0.0rc1
```

Download, configure and install hdf5
-------------------------------------

```bash
  cd $BUILD_DIR
  cd boost
  wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.10/hdf5-1.10.7/src/hdf5-1.10.7.tar.gz
  tar zxvf hdf5-1.10.7.tar.gz
  cd hdf5-1.10.7

  ######### Note that this is one command split in several lines
  ./configure \
  --prefix=$BUILD_DIR/boost/hdf5-1.10.7_install \
  CC=cc \
  CFLAGS=-O3 \
  CXX=CC \
  CXXFLAGS=-O3 \
  --enable-cxx \
  --enable-parallel \
  --enable-unsupported
  #########

  make -j 8
  make install
  export LD_LIBRARY_PATH=${BUILD_DIR}/boost/hdf5-1.10.7_install/lib:$LD_LIBRARY_PATH
  export LD_RUN_PATH=${BUILD_DIR}/boost/hdf5-1.10.7_install/lib:$LD_RUN_PATH
```


Download, configure and install FEniCS python components
---------------------------------------------------------

```bash
  cd $BUILD_DIR
  wget https://bitbucket.org/fenics-project/ffc/downloads/ffc-2019.1.0.post0.tar.gz
  tar zxvf ffc-2019.1.0.post0.tar.gz
  cd ffc-2019.1.0.post0/
  python3 setup.py install #(if does not work try "pip3 install .")
```


Download, configure and install prerequisites for PETSc
---------------------------------------------------------

The "sundials" library is not required by the coupling, and is therefore not installed. As it is accounted for by default, Line 21 (L21) of tpsl.sh, to be found as `${BUILD_DIR}/boost/sh/tpsl.sh` has to be changed from

printf "%s\n" glm hypre matio metis scotch parmetis mumps sundials superlu superlu-dist \
to
printf "%s\n" glm hypre matio metis scotch parmetis mumps superlu superlu-dist \

```bash
  cd $BUILD_DIR
  cd  boost
  ./sh/tpsl.sh --prefix=$(pwd)/boost
  export PATH=$PATH:${BUILD_DIR}/boost/metis-5.1.0/include
  export PATH=$PATH:${BUILD_DIR}/boost/parmetis-4.0.3/include
  export PATH=$PATH:${BUILD_DIR}/boost/superlu-5.2.2/SRC
  export PATH=$PATH:${BUILD_DIR}/boost/superlu_dist-6.4.0/SRC
  export PATH=$PATH:${BUILD_DIR}/boost/scotch_6.1.0/include
  export PATH=$PATH:${BUILD_DIR}/boost/MUMPS_5.3.5/include
  export PATH=$PATH:${BUILD_DIR}/boost/hdf5-1.10.7_install/include
  export PATH=$PATH:${BUILD_DIR}/boost/boost/include
  export LD_LIBRARY_PATH=${BUILD_DIR}/boost/hdf5-1.10.7_install/lib:$LD_LIBRARY_PATH
  export LD_RUN_PATH=${BUILD_DIR}/boost/hdf5-1.10.7_install/lib:$LD_RUN_PATH
  export HDF5_INCLUDE_DIR=${BUILD_DIR}/boost/hdf5-1.10.7_install/include
```

Download, configure and install PETSc
---------------------------------------

```bash
  cd $BUILD_DIR
  cd  boost
  wget https://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-3.11.4.tar.gz
  tar zxvf petsc-3.11.4.tar.gz
  cd petsc-3.11.4

  export ROOT_SHARED_DIR=${BUILD_DIR}/boost
  ######### Note that this is one command split in several lines
  ./configure \
  --prefix=$ROOT_SHARED_DIR/petsc-3.11.4/install \
  --with-mpi=1 \
  --CC=cc \
  --CFLAGS=-O3 \
  --CXX=CC \
  --CXXFLAGS=-O3 \
  --with-cxx-dialect=C++11 \
  --FC=ftn \
  --FFLAGS=-O3 \
  --enable-debug=0 \
  --enable-shared=1 \
  --with-precision=double \
  --with-hdf5=0 \
  --with-hdf5-dir=$ROOT_SHARED_DIR/hdf5-1.10.7_install \
  --download-superlu=yes \
  --download-superlu_dist=yes \
  --download-metis=yes \
  --download-parmetis=yes \
  --download-ptscotch=yes \
  --with-scalapack=1 \
  --with-mumps=1 \
  --with-mumps-include="${BUILD_DIR}/boost/MUMPS_5.3.5/include" \
  --with-mumps-lib="-L${BUILD_DIR}/boost/MUMPS_5.3.5/lib -lcmumps -ldmumps -lesmumps -lsmumps -lzmumps -lmumps_common -lptesmumps -lesmumps -lpord"
  ######### 

  make PETSC_DIR=`pwd` all
  make PETSC_DIR=`pwd` install
  export PETSC_DIR=${BUILD_DIR}/boost/petsc-3.11.4
  export PETSC_ARCH=arch-linux-c-opt
```

Download, configure and install PETSc4py
---------------------------------------
```bash
  cd $BUILD_DIR
  cd boost
  wget https://bitbucket.org/petsc/petsc4py/downloads/petsc4py-3.11.0.tar.gz
  tar zxvf petsc4py-3.11.0.tar.gz
  cd petsc4py-3.11.0
```

In order to continue the installation, some lines have to be erased from the file `src/PETSc/cyclicgc.pxi`. So, after editing it, Lines 27 and 28 (L27 and L28) should be deleted first, and then Lines 17, 18 and 19 (L17, L18 and L19):

```bash
L27:    if arg == NULL and _Py_AS_GC(d).gc_refs == 0:
L28:        _Py_AS_GC(d).gc_refs = 1  

L17:    ctypedef struct PyGC_Head:
L18:       Py_ssize_t gc_refs"gc.gc_refs"
L19:    PyGC_Head *_Py_AS_GC(PyObject*)
```
	
```bash
  rm -f src/petsc4py.PETSc.c
  python3 setup.py install
```
Download, configure and install DOLFIN
---------------------------------------

Download DOLFIN, and make sure that all the dependencies are correct:

```bash
  cd $BUILD_DIR
  export FENICS_VERSION=2019.1.0.post0
  git clone --branch=$FENICS_VERSION https://bitbucket.org/fenics-project/dolfin
  mkdir dolfin/build
  cd dolfin/build
  export BOOST_ROOT=$BUILD_DIR/boost
  export EIGEN3_INCLUDE_DIR=$BUILD_DIR/eigen-3.3.9/build/build/include/eigen3
  export SCOTCH_DIR=$BUILD_DIR/boost/boost
  
  export LD_LIBRARY_PATH=${BUILD_DIR}/boost/hdf5-1.10.7_install/lib:$LD_LIBRARY_PATH
  export LD_RUN_PATH=${BUILD_DIR}/boost/hdf5-1.10.7_install/lib:$LD_RUN_PATH
  export HDF5_INCLUDE_DIR=${BUILD_DIR}/boost/hdf5-1.10.7_install/include
  export PETSC_DIR=${BUILD_DIR}/boost/petsc-3.11.4
  export PETSC_ARCH=arch-linux-c-opt
```

The file `$BUILD_DIR/dolfin/CMakeLists.txt` needs some editing to 
add the following text to the top of the file (at Line 5 (L5)):

```bash
  SET( EIGEN3_INCLUDE_DIR "$ENV{EIGEN3_INCLUDE_DIR}" )
  IF( NOT EIGEN3_INCLUDE_DIR )
      MESSAGE( FATAL_ERROR "Please point the environment variable EIGEN3_INCLUDE_DIR to the include directory of your Eigen3 installation.")
  ENDIF()
  INCLUDE_DIRECTORIES ( "${EIGEN3_INCLUDE_DIR}" )
```

The file `$BUILD_DIR/dolfin/cmake/modules/FindPETSc.cmake` needs some editing and the following line should be changed 

```bash
  pkg_search_module(PETSC craypetsc_real PETSc)
  into
  pkg_search_module(PETSC petsc PETSc)
```

Finally, CMake is ran as follows, before the code is installed using make:

```bash
  cmake -DCMAKE_INSTALL_PREFIX=$(pwd)   \
  -DPYTHON_EXECUTABLE:FILEPATH=$BUILD_DIR/fenics2019_FSI/bin/python3   \
  -DDOLFIN_ENABLE_PYTHON=true \
  -DDOLFIN_USE_PYTHON3=true \
  -DDOLFIN_ENABLE_PETSC=true \
  -DPETSC_DIR="${BUILD_DIR}/boost/petsc-3.11.4" \
  -DPETSC_LIBRARY="${BUILD_DIR}/boost/petsc-3.11.4/arch-linux-c-opt/lib/libpetsc.so" \
  -DDOLFIN_SKIP_BUILD_TESTS=true \
  -DCMAKE_REQUIRED_LIBRARIES="-lmpifort" \
  -DCMAKE_CXX_FLAGS_RELEASE="-Wno-literal-suffix -O3 -DNDEBUG" \
  -DHDF5_ROOT="${BUILD_DIR}/boost/hdf5-1.10.7_install" \
  -DHDF5_INCLUDE_DIRS="${BUILD_DIR}/boost/hdf5-1.10.7_install/include" \
  -DPTESMUMPS_LIBRARY="${BUILD_DIR}/boost/petsc-3.11.4/install/lib/libptesmumps.a" \
  ..

  make -j 8 install
  source ${BUILD_DIR}/dolfin/build/share/dolfin/dolfin.conf
```

Build python build

```bash
  cd ../python
  export pybind11_DIR=$BUILD_DIR/pybind11-2.6.1/build//share/cmake/pybind11/
  export DOLFIN_DIR=$BUILD_DIR/dolfin/build/share/dolfin/cmake
  python3 setup.py install
```

Test the installation
---------------------------------------

Because the suite of software is installed for the compute nodes, some tests are carried out using an interactive session on ARCHER2. An example of how to set up such a session is provided here, but more information is available in ARCHER2 documentation pages. The following command is typed from a terminal session:

```bash
salloc --nodes=1 --tasks-per-node=128 --cpus-per-task=1 --time=00:20:00 --partition=standard --qos=short --reservation=shortqos --account=budget_code
```
where budget_code should be set by the user.

Before running the tests, some environment variables should be set, and the file FEniCS_2019.1.0_ARCHER2.conf should be copied to ARCHER2 and adapted for the current installation, by changing your_own_installation_path in Line 3 (L3) to the actual installation path. It is then sourced as:

```bash
. ./FEniCS_2019.1.0_ARCHER2.conf
```

It might convenient to add it to the .bashrc file.

The first test consists of checking if all the software are properly installed, by using a small piece of code called FEniCS_Test.py, which should be copied to ARCHER2, before running:

```bash
srun --distribution=block:block --hint=nomultithread python3 FEniCS_Test.py

python3 -c "from dolfin import *"
python3 -c "from dolfin import VectorFunctionSpace"
python3 -c "from dolfin import BoxMesh"
```
