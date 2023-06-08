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
#include "muiConfigOF.H"
#include "DynamicList.H"

typedef std::unique_ptr<mui::uniface1d> MuiUnifacePtr1d;
MuiUnifacePtr1d interface;
mui::sampler_exact1d<double> spatial_sampler;
mui::temporal_sampler_exact1d chrono_sampler;
bool ifsInit;
double oldTime;


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

  string interfaceName="mpi://OpenFoam_6DoF/ifs";
  if (! ifsInit){
    interface.reset(new mui::uniface1d(interfaceName));
    ifsInit = true;
    oldTime=0;
  }

    scalar t = time_.value();

    if (t != oldTime)  {   // To be corrected and send the time value independant of the time and space
      interface->push( "crrntTime", 0, t );
      interface->commit( oldTime );
    }

    //
    Info << "OpenFoam: 6DoF is fetching discplacement values at time " << t << endl;
    translationRotationVectors TRV;
    TRV[0][0] = interface->fetch( "disp_x", 0, t, spatial_sampler, chrono_sampler );
    TRV[0][1] = interface->fetch( "disp_y", 0, t, spatial_sampler, chrono_sampler );
    TRV[0][2] = interface->fetch( "disp_z", 0, t, spatial_sampler, chrono_sampler );

    TRV[1][0] = interface->fetch( "angle_x", 0, t, spatial_sampler, chrono_sampler );
    TRV[1][1] = interface->fetch( "angle_y", 0, t, spatial_sampler, chrono_sampler );
    TRV[1][2] = interface->fetch( "angle_z", 0, t, spatial_sampler, chrono_sampler );
    oldTime = t; // To be corrected and send the time value independant of the time and space
    Info << "OpenFoam: 6DoF is fetched discplacement values are " << TRV << endl;
    // Convert the rotational motion from deg to rad
    TRV[1] *= degToRad();

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
