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

/**
 * @file fixedRelaxation.h
 * @author W. Liu
 * @date 24 February 2021
 * @brief C wrapper (header file) of Fixed Relaxation Coupling Method 
 * (muiCouplingFixedRelaxation class) of FSI Coupling utility
 */

#ifndef MUICOUPLINGFIXEDRELAXATIONCAPI_H
#define MUICOUPLINGFIXEDRELAXATIONCAPI_H

#include "mui_c_wrapper_3d.h"
#include <stdarg.h>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

/* namespace muiCoupling
{ */


/*---------------------------------------------------------------------------*\
                 Class muiCouplingFixedRelaxation Declaration
\*---------------------------------------------------------------------------*/

//- Compiler judgment: C++ / C
#ifdef __cplusplus

extern "C" 
{

    //- With the C++ Compiler   
    class muiCouplingFixedRelaxation;

    typedef muiCouplingFixedRelaxation CAPI_muiCouplingFixedRelaxation;

#else

    //- With the C Compiler, an opaque pointer is used
    typedef struct CAPI_muiCouplingFixedRelaxation CAPI_muiCouplingFixedRelaxation;

#endif // __cplusplus


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

//- Constructor
CAPI_muiCouplingFixedRelaxation* create_muiCouplingFixedRelaxation
(
    int Narg, 
    ...
);

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

void delete_muiCouplingFixedRelaxation
(
    CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation
);


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

//- The const qualificators maps from the member function to pointers to the class instances.

    //- Return under relaxation factor of the coupling
    double muiCouplingFixedRelaxation_undRelxCpl(CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation);

    //- Return x axis component of the delta displacement
    double muiCouplingFixedRelaxation_getXDeltaDisp(    CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation,
                                                        int pointN);

    //- Return y axis component of the delta displacement
    double muiCouplingFixedRelaxation_getYDeltaDisp(    CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation,
                                                        int pointN);

    //- Return z axis component of the delta displacement
    double muiCouplingFixedRelaxation_getZDeltaDisp(    CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation,
                                                        int pointN);

    //- Return No. of points
    int muiCouplingFixedRelaxation_pointSize(CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation);

    //- Return square sum of the residual
    double muiCouplingFixedRelaxation_residualMagSqSum(CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation);

    //- Return maximum value of the residual L-2 Norm
    double muiCouplingFixedRelaxation_residualL2NormMax(CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation);

    //- Return the value of the residual L-2 Norm
    double muiCouplingFixedRelaxation_residualL2Norm(CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation);

    // Edit

    //- Initialize coupling method
    void muiCouplingFixedRelaxation_initialize(CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation,
                                                int Narg,
                                                ...);

    //- Collection of coupling process at this iteration
    void muiCouplingFixedRelaxation_collectResidual(    CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation, 
                                                        double fetchMUIx,
                                                        double fetchMUIy,
                                                        double fetchMUIz,
                                                        double disPreX,
                                                        double disPreY,
                                                        double disPreZ,
                                                        int pointN);
                            
    //- Collection of residual calculation at this iteration
    void muiCouplingFixedRelaxation_process(CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation);


// * * * * * * * * * * * * * * * Global Functions  * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

//- Compiler judgment: C++ / C
#ifdef __cplusplus

} // extern "C"

#endif // __cplusplus

/* } */ // End namespace muiCoupling

#endif // End MUICOUPLINGFIXEDRELAXATIONCAPI_H

// ************************************************************************* //
