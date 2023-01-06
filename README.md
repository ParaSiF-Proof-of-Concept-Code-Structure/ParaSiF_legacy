# ParaSiF - Parallel Partitioned Multi-physics Simulation Framework

Notes:
1) The coupling between the CFD and CSM solvers are achieved by the MUI library https://github.com/MxUI, which is under constant development and improvements. This version of ParaSIF is using MUI 1.2.4, and a copy of the library is also located in ParaSiF/src/CFD/OpenFOAM/ThirdParty-v2206/sources/. 
1) couplingFSILab is moved inside /ThirdPart/sources/MUI/MUI_utelites

Parallel Partitioned Multi-physics Simulation Framework is developed based on the MUI library. It offers a platform where users can carry out multi-physics (mainly fluid-structure interaction) studies using supercomputers.

The framework uses a partitioned approach to couple two or more physics domains together for multi-physics simulations. It takes several advantages of the MUI library:

• Flexibility to select different solvers for each physics domain;

• Flexibility to extend the number of physics domains;

• Good scalability on communications among physics domains for large simulations;

• Keeping the development of the coupled solvers decoupled. It allows for easier independent testing of each solver and avoids potential incompatibilities between two solvers (i.e if they both use a certain library X each one depends on a different version of it which are not compatible);

• Coupling of multiple solvers which have different programming language interfaces (e.g C, C++, FORTRAN and Python);

• "Plug and play" strategy. One solver can be replaced by another incrementally and without the need of recompiling if the MUI interface is used as a common adaptor;

• Use of multiple solvers which have two incompatible licenses exploiting the dual licensing of the MUI library (both solvers are never mixed source-wise or binary-wise).

**This framework is a beta software at the moment and under active development** to involve more solvers as well as more physics domains. Such infrastructure will make it possible to simulate large multi-physics problems and simulate complicated multi-physics cases by using supercomputing facilities.

## Licensing

Copyright (C) 2021 Engineering and Environment Group, Scientific Computing Department, Science and Technology Facilities Council, UK Research and Innovation. All rights reserved.

This code is licensed under the GNU General Public License version 3

The Parallel Partitioned Multi-physics Simulation Framework provides FEniCS v2019.1.0 <-> MUI v1.2.4 <-> OpenFOAM v2206 two-way coupling.

## Acknowledgements
The Parallel Partitioned Multi-physics Simulation Framework is developed at the [Scientific Computing Department](https://www.scd.stfc.ac.uk/) of the [Science and Technology Facilities Council](https://stfc.ukri.org/). If you use this framework, please cite us:

*Liu, W., Longshaw, S., Skillen, A., Emerson, D.R., Valente, C. and Gambioli, F. (in press). A High-performance Open-source Solution for Multiphase Fluid-Structure Interaction. International Journal of Offshore and Polar Engineering.

*Liu, W., Wang, W., Skillen, A., Longshaw, S.M., Moulinec, C. and Emerson, D.R. (2021). A Parallel Partitioned Approach on Fluid-Structure Interaction Simulation Using the Multiscale Universal Interface Coupling Library. In: 14th World Congress In Computational Mechanics (WCCM) And ECCOMAS Congress 2020.*

## Publication

Liu, W., Longshaw, S., Skillen, A., Emerson, D.R., Valente, C. and Gambioli, F. (in press). A High-performance Open-source Solution for Multiphase Fluid-Structure Interaction. International Journal of Offshore and Polar Engineering.

Liu, W., Longshaw, S.M., Skillen, A., Emerson, D.R., Valente, C. and Gambioli, F. (2021). A High-Performance Open-Source Solution for Multiphase Fluid-Structure Interaction. In: 31st International Ocean and Polar Engineering Conference (ISOPE).

Liu, W., Wang, W., Skillen, A., Longshaw, S.M., Moulinec, C. and Emerson, D.R. (2021). A Parallel Partitioned Approach on Fluid-Structure Interaction Simulation Using the Multiscale Universal Interface Coupling Library. In: 14th World Congress In Computational Mechanics (WCCM) And ECCOMAS Congress 2020.

Liu, W., Wang, W., Skillen, A., Fernandez, E.R., Longshaw, S. and Sawko, R. (2020). Code Development on Parallel Partitioned Fluid-Structure Interaction Simulations. Project Report. [online] STFC e-Pub. Available at: https://epubs.stfc.ac.uk/work/46400815.

## Contact

Should you have any question please do not hesitate to contact the developers

## Installation

**Step One: Install FEniCS v2019.0.1**

It is recommended to build  FEniCS with petse. To install petsc and petsc4py, follow the instructions in https://www.mcs.anl.gov/petsc/petsc4py-current/docs/usrman/install.html

It is important to add the following lines to .bashrc and source it. 
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/petsc/installation/directory/petsc/lib
export PETSC_DIR=/path/to/petsc/installation/directory/petsc
```

Please follow the instruction from FEniCS Prjoect Manual to install the FEniCS version 2019.1.0, which was released on April 19th 2019: https://fenicsproject.org/download/

If building FEniCS from source, please refer to the **Stable version** section to install the latest stable release of FEniCS (version 2019.1.0) from [this link](https://fenics.readthedocs.io/en/latest/installation.html#from-source)

The instructions for compiling FEniCS 2019.1.0 on a number of different operating environments are provided in the doc/ subfolder.

**Step Two: Obtain MUI and install FSI coupling lab and wrappers**
Note: inside the files Makefile used below, the path for ***eigen*** library has to be defined.

```bash
cd ParaSiF/src/MUI_Utility
wget https://github.com/MxUI/MUI/archive/1.2.tar.gz
tar -xzf 1.2.tar.gz
cd MUI-1.2/
cp -r ../couplingFSILab ./
cd ..
rm 1.2.tar.gz
mv MUI-1.2/ ../../../MUI

cd ../../../MUI/MUI-1.2/wrappers/C
make

cd ../Python

make USE_RBF=1 INC_EIGEN=/path/to/eigen package
make pip-install

cd ../../couplingFSILab/wrappers/C
make -f Makefile_CAPI

cd ../Python
make COMPILER=GNU package
make pip-install
cd ../../../../../ParaSiF/src
```

For more information on MUI, please refer to the MxUI GitHub organisation pages https://github.com/MxUI

**Step Three: Install OpenFOAM v6 with MUI patches**

```bash
cd ../../
mkdir -p OpenFOAM/v6
cd OpenFOAM/v6

wget -O - http://dl.openfoam.org/source/6 | tar xvz
wget -O - http://dl.openfoam.org/third-party/6 | tar xvz

mv OpenFOAM-6-version-6 OpenFOAM-6
mv ThirdParty-6-version-6 ThirdParty-6

cp ../../ParaSiF/src/MUI_Utility/OpenFOAM_patch/* ./
chmod +x ./patch_OF6-MUI
./patch_OF6-MUI
```

Please place a symlink with absolute path to the MUI library into the new folder "ThirdParty-6/MUI" before move forward.

Please note: 1. make sure the symlink to the MUI library is not relative path as a script takes a copy of these files; 2. Whenever MUI has been updated, the old platform files of OpenFOAM need to be cleaned and re-run Allwmake to update the copy that gets used by the OpenFOAM applications.

For more information, please refere to ParaSiF/src/MUI_Utility/OpenFOAM_patch/README, OpenFOAM-6/README-MUI and ThirdParty-6/README-MUI

```bash
source OpenFOAM-6/etc/bashrc
cd OpenFOAM-6


./Allwmake

cd ../../../ParaSiF/src/
```
For more information on OpenFOAM v6, please refer to the OpenFOAM Foundation: https://openfoam.org/download/6-source/

**Step Four: Install ParaSiF OpenFOAM solvers**

```bash
cd CFD/OpenFOAM/V6/applications/solvers/

# Source OpenFOAM run functions
. $WM_PROJECT_DIR/bin/tools/RunFunctions

# Compile ParaSiF solvers
wmake pimpleFSIFoam
wmake interFSIFoam
```

please note: if there are errors on eigen or mui.h during 'wmake pimpleFSIFoam' or 'wmake interFSIFoam', please open 'ParaSiF/src/CFD/OpenFOAM/V6/applications/solvers/pimpleFSIFoam/Make/options', change Line 18 - Line 19 according to your MUI and eigen3 folder path. Do the same for 'ParaSiF/src/CFD/OpenFOAM/V6/applications/solvers/interFSIFoam/Make/options'. Repeat the wmake after these modifications.

## Source and export before run ParaSiF cases

```bash
source /path/to/dolfin/dolfin.conf
source /path/to/OpenFOAM-6/etc/bashrc
export PYTHONPATH=/path/to/ParaSiF/src/CSM/FEniCS/V2019.1.0:$PYTHONPATH
```

## Demos

Demos of both the single phase FSI case (3-D flow past a flexible beam) and the multiphase FSI case (3-D dam break with a flexible beam) are in ParaSiF/demo/FSIBeam_pimpleFSIFoam and ParaSiF/demo/FSIBeam_interFSIFoam, respectively.

In the demo folde, the subfolder caseSetup contains the input files for both fluid and structure domain, the subfolder runCtrl contains scripts for the case execution.

To run the demo cases, execute the Allrun script.

A new subfolder runData will be generated during runtime, which contains output files (such as checkpoint data, probs, forces, etc) generated from both the fluid and structure domain.

In runData/structureDomain/structureResults: 

• checkpointData_XXX.h5 is the checkpoint data for structure domain, which the XXX is the time name of the checkpoint data;

• displacementXXX.vtu, stressXXX.vtu and surface_traction_structureXXX.vtu are VTU format files on displacement, stress and surface traction of the structure domain, respectively. The XXX is the time-step name.

• displacement.pvd, stress.pvd and surface_traction_structure.pvd are PVD format files on displacement, stress and surface traction of the structure domain, respectively. These are the files for post-process visualation (such as read by ParaView). 

• tip-displacementX_0.txt is the TXT format file on the displacement of the probed point (defined in structureInputPara.ini, POSTPROCESS section, pointMoniX/pointMoniY/pointMoniZ variables). The 'X' in the 'tip-displacementX_0.txt' indicates that the data is about the x-axis displacement, and the '0' means it is the output from rank0 of the parallel simulation.

To restart: 
1. Follow the OpenFOAM restart procedure to set the runData/fluidDomain subfolder; 
2. Copy the checkpoint data with the time name you want from the runData/structureDomain/structureResults to the subfolder runData/structureDomain/dataInput (for example, if we want to restart at t=0.01, checkpointData_0.01.h5 need to be copied to dataInput subfolder);
3. Rename dataInput/checkpointData_XXX.h5 into dataInput/checkpointData.h5 (i.e. remove the time name of the checkpoint data. for example, change the name of dataInput/checkpointData_0.01.h5 into dataInput/checkpointData.h5);
4. Change the parameter 'iContinueRun' into 'True' in the 'TIME' section of the structure input file runData/structureDomain/structureFSISetup/structureInputPara.ini;
5. (Optional) To avoide overwrite of the previous calculated data, re-name the runData/structureDomain/structureResults;
6. In 'Allrun', comment out './runCtrl/runDataFolderCreation' as the subfolder 'runData' has already created in previous run; comment out './runCtrl/preProcess' as we don't need OpenFOAM pre-processes (mesh generation, setFields, decomposePar, etc) in restart.
7. Execute the updated Allrun script.
