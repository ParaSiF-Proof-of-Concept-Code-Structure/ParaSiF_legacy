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
// typedef std::unique_ptr<mui::uniface1d> MuiUnifacePtr1d;
// MuiUnifacePtr1d interface;
// mui::sampler_exact1d<double> spatial_sampler;
// mui::temporal_sampler_exact1d chrono_sampler;
// DynamicList<mui::uniface<mui::config_1d>*> mui1dInterfaces;
// DynamicList<mui::uniface<mui::config_2d>*> mui2dInterfaces;
// DynamicList<mui::uniface<mui::config_3d>*> mui3dInterfaces;
// DynamicList<mui::uniface<mui::config_of>*> muiTemplatedInterfaces;

// mui::uniface1d interface( "mpi://pong/ifs" );
typedef std::unique_ptr<mui::uniface1d> MuiUnifacePtr1d;
MuiUnifacePtr1d interface;
mui::sampler_exact1d<double> spatial_sampler;
mui::temporal_sampler_exact1d chrono_sampler;



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
  // Info << "Open Foam XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX 1" <<endl ;
  // typedef std::unique_ptr<mui::uniface1d> MuiUnifacePtr1d;
  // Info << "Open Foam XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX 2" <<endl ;
  // MuiUnifacePtr1d interface;
  // mui::sampler_exact1d<double> spatial_sampler;
  // mui::temporal_sampler_exact1d chrono_sampler;

    scalar t = time_.value();

    if (t < times_[0])
    {
        FatalErrorInFunction
            << "current time (" << t
            << ") is less than the minimum in the data table ("
            << times_[0] << ')'
            << exit(FatalError);
    }

    if (t > times_.last())
    {
        FatalErrorInFunction
            << "current time (" << t
            << ") is greater than the maximum in the data table ("
            << times_.last() << ')'
            << exit(FatalError);
    }
Info << "Open Foam XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX 3" <<endl ;
std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 99);
    int random_number = dis(gen);
    mui::point1d push_point;
    mui::point1d fetch_point;
    Info << "Open Foam XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX 4" <<endl ;
    double disp_x, disp_y,disp_z;
    double angle_x, angle_y,angle_z;
    Info << "Open Foam XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX 5" <<endl ;
    double crrntTime= 0.00055;
    fetch_point[0] = 0;
    Info << "Open Foam XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX 6" <<endl ;
    string interfaceName="mpi://p"+std::to_string(random_number)+"/ifs";
    interface.reset(new mui::uniface1d(interfaceName));

    //
    // disp_x = interface->fetch( "disp_x", fetch_point, crrntTime, spatial_sampler, chrono_sampler );
    // disp_y = interface->fetch( "disp_y", fetch_point, crrntTime, spatial_sampler, chrono_sampler );
    // disp_z = interface->fetch( "disp_z", fetch_point, crrntTime, spatial_sampler, chrono_sampler );
    //
    // angle_x = interface->fetch( "angle_x", fetch_point, crrntTime, spatial_sampler, chrono_sampler );
    // angle_y = interface->fetch( "angle_y", fetch_point, crrntTime, spatial_sampler, chrono_sampler );
    // angle_z = interface->fetch( "angle_z", fetch_point, crrntTime, spatial_sampler, chrono_sampler );


    cout << "Pong disp_x = " << disp_x << endl;
    cout << "Pong disp_y = " << disp_y << endl;
    cout << "Pong disp_z = " << disp_z << endl;

    cout << "Pong angle_x = " << angle_x << endl;
    cout << "Pong angle_y = " << angle_y << endl;
    cout << "Pong angle_z = " << angle_z << endl;


    translationRotationVectors TRV = interpolateSplineXY
    (
        t,
        times_,
        values_
    );
    Info << "++++++++++++++++ Current time = " << t<< " , Interpolated values = " << TRV << endl;
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



    // mui::uniface1d interface( "mpi://foam/ifs" );
    // mui::sampler_exact1d<double> spatial_sampler;
    // interface.reset(new mui::uniface1d("mpi://foam/ifs"));




    if (newTimeDataFileName != timeDataFileName_)
    {
        timeDataFileName_ = newTimeDataFileName;

        IFstream dataStream(timeDataFileName_);

        if (dataStream.good())
        {
            List<Tuple2<scalar, translationRotationVectors>> timeValues
            (
                dataStream
            );

            times_.setSize(timeValues.size());
            values_.setSize(timeValues.size());

            forAll(timeValues, i)
            {
                times_[i] = timeValues[i].first();
                values_[i] = timeValues[i].second();
                // Info << "Read from the table Time = " << times_[i] << "and values "<< values_[i]<<endl;
            }
        }
        else
        {
            FatalErrorInFunction
                << "Cannot open time data file " << timeDataFileName_
                << exit(FatalError);
        }
    }

    SBMFCoeffs_.readEntry("CofG", CofG_);

    return true;
}


// ************************************************************************* //
