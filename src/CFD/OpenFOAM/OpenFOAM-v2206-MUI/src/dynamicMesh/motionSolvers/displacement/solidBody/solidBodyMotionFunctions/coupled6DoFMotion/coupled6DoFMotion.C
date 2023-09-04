/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
    Copyright (C) 2019-2020 OpenCFD Ltd.
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "coupled6DoFMotion.H"
#include "addToRunTimeSelectionTable.H"
#include "Tuple2.H"
#include "IFstream.H"
#include "interpolateSplineXY.H"
#include "unitConversion.H"
#include "mui.h"
#include <algorithm>
#include "coupled6DoF_config.h"
#include "DynamicList.H"
#include ".../OpenFOAM-v2206-MUI/applications/solvers/multiphase/interFoamCoupled/couplingVarExternal.H"
bool ifsInit, updateRF;
double oldTime;

typedef std::unique_ptr<mui::uniface<mui::coupled6DoF_config>> MuiUnifacePtr3d;
std::vector<MuiUnifacePtr3d> ifs;
mui::sampler_exact<mui::coupled6DoF_config> spatial_sampler;
mui::temporal_sampler_exact<mui::coupled6DoF_config> chrono_sampler;



// Total iteration counter
int totalCurrentIter;
double initUndRelxCpl;
std::string cplMethod;
mui::algo_aitken<mui::coupled6DoF_config> aitken1;
mui::algo_aitken<mui::coupled6DoF_config> aitken2;
mui::algo_aitken<mui::coupled6DoF_config> aitken3;
mui::algo_aitken<mui::coupled6DoF_config> aitken4;
mui::algo_aitken<mui::coupled6DoF_config> aitken5;
mui::algo_aitken<mui::coupled6DoF_config> aitken6;
mui::algo_fixed_relaxation<mui::coupled6DoF_config> fr1;
mui::algo_fixed_relaxation<mui::coupled6DoF_config> fr2;
mui::algo_fixed_relaxation<mui::coupled6DoF_config> fr3;
mui::algo_fixed_relaxation<mui::coupled6DoF_config> fr4;
mui::algo_fixed_relaxation<mui::coupled6DoF_config> fr5;
mui::algo_fixed_relaxation<mui::coupled6DoF_config> fr6;

    
// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace solidBodyMotionFunctions
{
    defineTypeNameAndDebug(coupled6DoFMotion, 0);
    addToRunTimeSelectionTable
    (
        solidBodyMotionFunction,
        coupled6DoFMotion,
        dictionary
    );
}
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::solidBodyMotionFunctions::coupled6DoFMotion::coupled6DoFMotion
(
    const dictionary& SBMFCoeffs,
    const Time& runTime
)
:
    solidBodyMotionFunction(SBMFCoeffs, runTime)
{
    read(SBMFCoeffs);
}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

Foam::septernion
Foam::solidBodyMotionFunctions::coupled6DoFMotion::transformation() const
{

  std::vector<std::string> interfaces;
	std::string domainName="OpenFoam_6DoF";
	std::string appName="threeDInterface0";
    interfaces.emplace_back(appName);
    // auto ifs = mui::create_uniface<mui::coupledForce_config>( "OpenFoam_forces", interfaces );
    mui::point3d locf( 0.0, 0.0, 0.0 );
    
    


  if (! ifsInit){
    ifs=mui::create_uniface<mui::coupled6DoF_config>( domainName, interfaces );

    ifsInit = true;
    oldTime=0;
  }

  if (! updateRF){

    fr1.setUnderRelaxationFactor(initUndRelxCpl);
    fr2.setUnderRelaxationFactor(initUndRelxCpl);
    fr3.setUnderRelaxationFactor(initUndRelxCpl);
    fr4.setUnderRelaxationFactor(initUndRelxCpl);
    fr5.setUnderRelaxationFactor(initUndRelxCpl);
    fr6.setUnderRelaxationFactor(initUndRelxCpl);
    aitken1.setUnderRelaxationFactor(initUndRelxCpl);
    aitken2.setUnderRelaxationFactor(initUndRelxCpl);
    aitken3.setUnderRelaxationFactor(initUndRelxCpl);
    aitken4.setUnderRelaxationFactor(initUndRelxCpl);
    aitken5.setUnderRelaxationFactor(initUndRelxCpl);
    aitken6.setUnderRelaxationFactor(initUndRelxCpl);
    updateRF=true;
  }
        

    auto t=totalCurrentIter;

    //
    Info << "OpenFoam: 6DoF is fetching discplacement values at time " << t << endl;
    translationRotationVectors TRV;
    double smllVal=10e-16;
    if (cplMethod == "Loose")

	{
        TRV[0][0] = ifs[0]->fetch( "dispX", locf, t, spatial_sampler, chrono_sampler )+smllVal;
        TRV[0][1] = ifs[0]->fetch( "dispY", locf, t, spatial_sampler, chrono_sampler )+smllVal;
        TRV[0][2] = ifs[0]->fetch( "dispZ", locf, t, spatial_sampler, chrono_sampler )+smllVal;

        TRV[1][0] = ifs[0]->fetch( "angleX", locf, t, spatial_sampler, chrono_sampler );
        TRV[1][1] = ifs[0]->fetch( "angleY", locf, t, spatial_sampler, chrono_sampler );
        TRV[1][2] = ifs[0]->fetch( "angleZ", locf, t, spatial_sampler, chrono_sampler );
    } else if (cplMethod == "Aitken")
	{
        TRV[0][0] = ifs[0]->fetch( "dispX", locf, t, spatial_sampler, chrono_sampler,aitken1 )+smllVal;
        TRV[0][1] = ifs[0]->fetch( "dispY", locf, t, spatial_sampler, chrono_sampler,aitken2 )+smllVal;
        TRV[0][2] = ifs[0]->fetch( "dispZ", locf, t, spatial_sampler, chrono_sampler,aitken3 )+smllVal;

        TRV[1][0] = ifs[0]->fetch( "angleX", locf, t, spatial_sampler, chrono_sampler,aitken4 );
        TRV[1][1] = ifs[0]->fetch( "angleY", locf, t, spatial_sampler, chrono_sampler,aitken5 );
        TRV[1][2] = ifs[0]->fetch( "angleZ", locf, t, spatial_sampler, chrono_sampler,aitken6 );
    } else if (cplMethod == "FixedRelaxation")
    {
        TRV[0][0] = ifs[0]->fetch( "dispX", locf, t, spatial_sampler, chrono_sampler,fr1 )+smllVal;
        TRV[0][1] = ifs[0]->fetch( "dispY", locf, t, spatial_sampler, chrono_sampler,fr2 )+smllVal;
        TRV[0][2] = ifs[0]->fetch( "dispZ", locf, t, spatial_sampler, chrono_sampler,fr3 )+smllVal;

        TRV[1][0] = ifs[0]->fetch( "angleX", locf, t, spatial_sampler, chrono_sampler,fr4 );
        TRV[1][1] = ifs[0]->fetch( "angleY", locf, t, spatial_sampler, chrono_sampler,fr5 );
        TRV[1][2] = ifs[0]->fetch( "angleZ", locf, t, spatial_sampler, chrono_sampler,fr6 );
    }
    oldTime = t; // To be corrected and send the time value independant of the time and space
    Info << "OpenFoam: 6DoF usinf cupling method " << cplMethod << " with under relaxation of " << initUndRelxCpl<<endl;
    Info << "OpenFoam: 6DoF is fetched discplacement values at totalCurrentIter = "<< totalCurrentIter <<"  are  " << TRV << endl;
    // Info << "Aitken :::: RF" << fr3.get_under_relaxation_factor(t) <<endl;
    // Info << "Aitken :::: Res" << fr3.get_residual_L2_Norm(t) <<endl;

    // Convert the rotational motion from deg to rad
    // TRV[1] *= degToRad();

    quaternion R(quaternion::XYZ, TRV[1]);
    septernion TR(septernion(-CofG_ + -TRV[0])*R*septernion(CofG_));

    DebugInFunction << "Time = " << t << " transformation: " << TR << endl;

    return TR;
}


bool Foam::solidBodyMotionFunctions::coupled6DoFMotion::read
(
    const dictionary& SBMFCoeffs
)
{
    solidBodyMotionFunction::read(SBMFCoeffs);

    // If the timeDataFileName has changed read the file

    fileName newTimeDataFileName
    (
        SBMFCoeffs_.get<fileName>("timeDataFileName").expand()
    );

    SBMFCoeffs_.readEntry("CofG", CofG_);

    return true;
}


// ************************************************************************* //
