/*****************************************************************************
* Multiscale Universal Interface Code Coupling Library                       *
*                                                                            *
* Copyright (C) 2019 Y. H. Tang, S. Kudo, X. Bian, Z. Li, G. E. Karniadakis  *
*                                                                            *
* This software is jointly licensed under the Apache License, Version 2.0    *
* and the GNU General Public License version 3, you may use it according     *
* to either.                                                                 *
*                                                                            *
* ** Apache License, version 2.0 **                                          *
*                                                                            *
* Licensed under the Apache License, Version 2.0 (the "License");            *
* you may not use this file except in compliance with the License.           *
* You may obtain a copy of the License at                                    *
*                                                                            *
* http://www.apache.org/licenses/LICENSE-2.0                                 *
*                                                                            *
* Unless required by applicable law or agreed to in writing, software        *
* distributed under the License is distributed on an "AS IS" BASIS,          *
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
* See the License for the specific language governing permissions and        *
* limitations under the License.                                             *
*                                                                            *
* ** GNU General Public License, version 3 **                                *
*                                                                            *
* This program is free software: you can redistribute it and/or modify       *
* it under the terms of the GNU General Public License as published by       *
* the Free Software Foundation, either version 3 of the License, or          *
* (at your option) any later version.                                        *
*                                                                            *
* This program is distributed in the hope that it will be useful,            *
* but WITHOUT ANY WARRANTY; without even the implied warranty of             *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
* GNU General Public License for more details.                               *
*                                                                            *
* You should have received a copy of the GNU General Public License          *
* along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
*****************************************************************************/
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
    
    @file mui_config.h
    
    @author W. Liu
    
    @brief This is a part of the Parallel Partitioned Multi-physical Simu-
    lation Framework provides FEniCS v2019.1.0 <-> MUI v1.2 <-> OpenFOAM v6 
    two-way coupling.

    Incompressible Navier-Stokes equations for fluid domain in OpenFOAM
    Structure dynamics equations for structure domain in FEniCS.

    It is config file of MUI for the CFD solver
    Located in the src/CFD/OpenFOAM/V6/applications/solvers/interFSIFoam sub-folder
\*---------------------------------------------------------------------------*/

#ifndef MUI_CONFIG_H
#define MUI_CONFIG_H

#include "util.h"
#include "dim.h"
#include "exception.h"

namespace mui {

struct mui_config {
	using EXCEPTION = exception_segv;
	/// Switch of debug mode
	static const bool DEBUG = false;
    /// Define the dimension of the interface
	static const int D = 3;
    /// Switch of fixed points/dynamic points
	static const bool FIXEDPOINTS = false;
	static const bool QUIET = false;		//- If the library is quiet then it will only issue critical warning messages

    /// MUI type define
	using REAL = double;
	using INT  = int32_t;

	using point_type = point<REAL,D>;
	using time_type  = REAL; // INT-typed time stamp might be an alternative
	using data_types = type_list<int32_t,int64_t,double,float>;

};
}

#endif