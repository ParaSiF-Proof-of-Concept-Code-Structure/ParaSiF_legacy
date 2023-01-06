/*---------------------------------------------------------------------------*\
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
    
    @file initFSILocalContinuityErrs.H
    
    @author W. Liu
    
    @brief This is a part of the Parallel Partitioned Multi-physical Simu-
    lation Framework provides FEniCS v2019.1.0 <-> MUI v1.0 <-> OpenFOAM v6 
    two-way coupling.

    Incompressible Navier-Stokes equations with two fluid by VOF for fluid domain 
	in OpenFOAM and Structure dynamics equations for structure domain in FEniCS.

    It declares and initialises the cumulative continuity error for the CFD solver
    Located in the src/CFD/OpenFOAM/V6/applications/solvers/pimpleFSIFoam sub-folder
\*---------------------------------------------------------------------------*/

#ifndef initContinuityErrsLocal_H
#define initContinuityErrsLocal_H

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

scalar cumulativeContErrLocal = 0;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// *************************  FILE END  ************************************ //