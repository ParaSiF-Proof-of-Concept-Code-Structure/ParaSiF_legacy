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
 * @file fixedRelaxation_CAPI.cpp
 * @author W. Liu
 * @date 24 February 2021
 * @brief C wrapper (source file) of Fixed Relaxation Coupling Method 
 * (muiCouplingFixedRelaxation class) of FSI Coupling utility
 */

#include "fixedRelaxation.h"
#include "fixedRelaxation_inl.H"
#include "mui_c_wrapper_3d.h"
#include <iostream>
#include <stdarg.h>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
extern "C" {
    
namespace muiCoupling
{

using namespace std;


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //
typedef muiCouplingFixedRelaxation CAPI_muiCouplingFixedRelaxation;
//                                   [ NONE ]                               //

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

//- Constructor - default
CAPI_muiCouplingFixedRelaxation* create_muiCouplingFixedRelaxation(int Narg, ...)
{
    switch(Narg)
    {
        case 1:
        {
            va_list ap;
            va_start(ap, Narg);
            
            double initUndRelxCpl;
            
            for(int counter = 0; counter < Narg; ++counter)
            {
                initUndRelxCpl = va_arg(ap, double); 
            }
            
            std::cout << "C API, create_muiCouplingFixedRelaxation" << std::endl;
            return new CAPI_muiCouplingFixedRelaxation(initUndRelxCpl); 
            
            break;
        }
        
        case 2:
        {
            va_list ap;
            va_start(ap, Narg);

            int pointSize;
            double initUndRelxCpl;

            for(int counter = 0; counter < Narg; ++counter)
            {
                if(counter == 0)
                {
                    pointSize = va_arg(ap, int);
                } else if (counter == 1)
                {
                    initUndRelxCpl = va_arg(ap, double);
                }
            }
            
            std::cout << "C API, create_muiCouplingFixedRelaxation" << std::endl;
            return new CAPI_muiCouplingFixedRelaxation(pointSize, initUndRelxCpl);
            
            break;
        }

        case 3:
        {
            va_list ap;
            va_start(ap, Narg);

            int pointSize;
            double initUndRelxCpl;
            MPI_Comm *Cworld;

            for(int counter = 0; counter < Narg; ++counter)
            {
                if(counter == 0)
                {
                    pointSize = va_arg(ap, int);
                } else if (counter == 1)
                {
                    initUndRelxCpl = va_arg(ap, double);
                    if (initUndRelxCpl <= 1e-5)
                    {
                        initUndRelxCpl=0.8;
                    }
                } else if (counter == 2)
                {
                    Cworld = va_arg(ap, MPI_Comm*);
                }
            }

            std::cout << "C API, create_muiCouplingFixedRelaxation" << std::endl;
            return new CAPI_muiCouplingFixedRelaxation(pointSize, initUndRelxCpl, Cworld);
            
            break;
        }

        default:
        {
            printf("Invalid number of arguments for 'create_muiCouplingFixedRelaxation()' function'\n");
            printf("Please provide 1, 2 or 3 argument numbers'\n");
            exit(1);
        }
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

void delete_muiCouplingFixedRelaxation(CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation){
    std::cout << "C API, delete_muiCouplingFixedRelaxation" << std::endl;
    delete muiCouplingFixedRelaxation;
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

double muiCouplingFixedRelaxation_undRelxCpl(CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation)
{

    return muiCouplingFixedRelaxation->undRelxCpl();
 
}

double muiCouplingFixedRelaxation_getXDeltaDisp(CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation,
                                                int pointN)
{

    return muiCouplingFixedRelaxation->getXDeltaDisp(pointN);
                                                  
}

double muiCouplingFixedRelaxation_getYDeltaDisp(CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation,
                                                int pointN)
{

    return muiCouplingFixedRelaxation->getYDeltaDisp(pointN);
                                                  
}

double muiCouplingFixedRelaxation_getZDeltaDisp(CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation,
                                                int pointN)
{

    return muiCouplingFixedRelaxation->getZDeltaDisp(pointN);
                                                  
}

int muiCouplingFixedRelaxation_pointSize(CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation)
{
 
    return muiCouplingFixedRelaxation->pointSize();
 
}

//- Return square sum of the residual
double muiCouplingFixedRelaxation_residualMagSqSum(CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation)
{

    return muiCouplingFixedRelaxation->residualMagSqSum();

}

//- Return maximum value of the residual L-2 Norm
double muiCouplingFixedRelaxation_residualL2NormMax(CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation)
{

    return muiCouplingFixedRelaxation->residualL2NormMax();

}
//- Return the value of the residual L-2 Norm
double muiCouplingFixedRelaxation_residualL2Norm(CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation)
{

    return muiCouplingFixedRelaxation->residualL2Norm();

}

//- Initialize coupling method
void muiCouplingFixedRelaxation_initialize( CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation,
                                            int Narg,
                                            ...)
{
    switch(Narg)
    {
        case 0:
        {

            muiCouplingFixedRelaxation->initialize();

            break;
        }

        case 1:
        {
            va_list ap;
            va_start(ap, Narg);

            int pointSize;

            for(int counter = 0; counter < Narg; ++counter)
            {
                pointSize = va_arg(ap, int);
            }

            muiCouplingFixedRelaxation->initialize(pointSize);
            
            break;
        }

        case 2:
        {
            va_list ap;
            va_start(ap, Narg);

            int pointSize;
            MPI_Comm *Cworld;

            for(int counter = 0; counter < Narg; ++counter)
            {
                if(counter == 0)
                {
                    pointSize = va_arg(ap, int);
                }else if (counter == 1)
                {
                    Cworld = va_arg(ap, MPI_Comm*);
                }
            }

            muiCouplingFixedRelaxation->initialize(pointSize, Cworld);

            break;
        }

        default:
        {
            printf("Invalid number of arguments for 'muiCouplingFixedRelaxation_initialize()' function'\n");
            printf("Please provide 0, 1 or 2 argument numbers'\n");
            exit(1);
        }
    }
}

void muiCouplingFixedRelaxation_collectResidual(    CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation,
                                                    double fetchMUIx,
                                                    double fetchMUIy,
                                                    double fetchMUIz,
                                                    double disPreX,
                                                    double disPreY,
                                                    double disPreZ,
                                                    int pointN)
{

     muiCouplingFixedRelaxation->collectResidual(   fetchMUIx,
                                                    fetchMUIy,
                                                    fetchMUIz,
                                                    disPreX,
                                                    disPreY,
                                                    disPreZ,
                                                    pointN);
 
}

void muiCouplingFixedRelaxation_process(CAPI_muiCouplingFixedRelaxation* muiCouplingFixedRelaxation)
{

    muiCouplingFixedRelaxation->process();

}

// * * * * * * * * * * * * * * * Global Functions  * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace muiCoupling

}
// ************************************************************************* //
