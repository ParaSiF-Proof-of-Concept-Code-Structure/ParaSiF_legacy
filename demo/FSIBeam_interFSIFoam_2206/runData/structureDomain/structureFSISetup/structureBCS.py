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
    
    @file structureBCS.py
    
    @author W. Liu
    
    @brief This is a part of the Parallel Partitioned Multi-physical Simu-
    lation Framework provides FEniCS v2019.1.0 <-> MUI v1.0 <-> OpenFOAM v6 
    two-way coupling.

    Incompressible Navier-Stokes equations for fluid domain in OpenFOAM
    Structure dynamics equations for structure domain in FEniCS.

    structureBCS.py is the boundary condition class of the structure code 
    located in the caseSetup/structureDomain/structureFSISetup sub-folder
"""

#_________________________________________________________________________________________
#
#%% Import packages
#_________________________________________________________________________________________

from dolfin import *

#_________________________________________________________________________________________
#
#%% Define boundary conditions
#_________________________________________________________________________________________

class boundaryConditions:
    def DirichletMixedBCs(self, MixedVectorFunctionSpace, boundaries, marks):
        bc1 = DirichletBC(MixedVectorFunctionSpace.sub(0), ((0.0,0.0,0.0)),boundaries, marks)
        bc2 = DirichletBC(MixedVectorFunctionSpace.sub(1), ((0.0,0.0,0.0)),boundaries, marks)
        return bc1, bc2
    def DirichletBCs(self, VectorFunctionSpace, boundaries, marks):
        bc3 = DirichletBC(VectorFunctionSpace, ((0.0,0.0,0.0)),boundaries, marks)
        return bc3

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%  FILE END  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#