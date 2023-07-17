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

    structureSubDomain.py is the sub-domain class of the structure code 
    located in the caseSetup/structureDomain/structureFSISetup sub-folder
"""

#_________________________________________________________________________________________
#
#%% Import packages
#_________________________________________________________________________________________

from dolfin import *
import configparser

#_________________________________________________________________________________________
#
#%% Import configure file
#_________________________________________________________________________________________

config = configparser.ConfigParser()
config.read('./structureFSISetup/structureInputPara.ini')

OBeamXtemp=float(config['GEOMETRY']['OBeamX'])
OBeamYtemp=float(config['GEOMETRY']['OBeamY'])
OBeamZtemp=float(config['GEOMETRY']['OBeamZ'])
XBeamtemp=float(config['GEOMETRY']['XBeam'])
YBeamtemp=float(config['GEOMETRY']['YBeam'])
ZBeamtemp=float(config['GEOMETRY']['ZBeam'])

#_________________________________________________________________________________________
#
#%% Define SubDomains classes
#%% for defining parts of the boundaries and the interior of the domain
#_________________________________________________________________________________________

class Fixed( SubDomain ):
    def inside (self , x, on_boundary ):
        tol = DOLFIN_EPS
        return near(x[1], (OBeamYtemp + tol))
        
class Flex( SubDomain ):
    def inside (self , x, on_boundary ):
        tol = DOLFIN_EPS
        return near(x[1], (OBeamYtemp + YBeamtemp - tol)) or near(x[0], (OBeamXtemp + tol)) or near(x[0], (OBeamXtemp + XBeamtemp - tol) or near(x[2], (OBeamZtemp + tol)))

class Symmetry( SubDomain ):
    def inside (self , x, on_boundary ):
        tol = DOLFIN_EPS
        return near(x[2], (OBeamZtemp + ZBeamtemp - tol))
        
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%  FILE END  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#