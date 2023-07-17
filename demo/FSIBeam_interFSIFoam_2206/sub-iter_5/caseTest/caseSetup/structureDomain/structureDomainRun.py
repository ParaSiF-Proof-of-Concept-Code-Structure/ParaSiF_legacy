""" 
    Parallel Partitioned Multi-physical Simulation Framework (parMupSiF)

    Copyright (C) 2021 Engineering and Environment Group, Scientific 
    Computing Department, Science and Technology Facilities Council, 
    UK Research and Innovation. All rights reserved.

    This code is licensed under the GNU General Public License version 3

    ** GNU General Public License, version 3 **

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
    *********************************************************************
    
    @file structureSubDomain.py
    
    @author W. Liu
    
    @brief This is a part of the Parallel Partitioned Multi-physical Simu-
    lation Framework provides FEniCS v2019.1.0 <-> MUI v1.0 <-> OpenFOAM v6 
    two-way coupling.

    Incompressible Navier-Stokes equations for fluid domain in OpenFOAM
    Structure dynamics equations for structure domain in FEniCS.

    structureFSIRun.py is the main function of the structure code 
    located in the caseSetup/structureDomain sub-folder
"""

#_________________________________________________________________________________________
#
#%% Import packages
#_________________________________________________________________________________________

from dolfin import *
import configparser
import structureFSISetup
import structureFSISolver

#_________________________________________________________________________________________
#
#%% Import configure file
#_________________________________________________________________________________________

config = configparser.ConfigParser()
config.read('./structureFSISetup/structureInputPara.ini')

#_________________________________________________________________________________________
#
#%% Create instances for sub-domains and boundary condition
#_________________________________________________________________________________________

# Create sub-domain instances
fixed = structureFSISetup.structureSubDomain.Fixed()
flex = structureFSISetup.structureSubDomain.Flex()
symmetry = structureFSISetup.structureSubDomain.Symmetry()
# Create boundary condition instances
BCs = structureFSISetup.structureBCS.boundaryConditions()

#_________________________________________________________________________________________
#
#%% Create solver instances
#_________________________________________________________________________________________

solver = structureFSISolver.structureFSISolver.StructureFSISolver(config, fixed, flex, symmetry, BCs)

#_________________________________________________________________________________________
#
#%% Solving
#_________________________________________________________________________________________

solver.Solver()

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%  FILE END  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#