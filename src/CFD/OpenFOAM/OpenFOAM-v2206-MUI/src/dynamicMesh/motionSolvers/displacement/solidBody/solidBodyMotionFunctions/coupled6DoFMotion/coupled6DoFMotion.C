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
#include "fvMesh.H"
// #include "Time.H"
#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.
#include "mui.h"
#include "muiconfig.h"

mui::sampler_exact<mui::mui_config> spatial_sampler;
mui::temporal_sampler_exact<mui::mui_config> chrono_sampler;

#endif

    
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

    mui::point3d rcv_point(0,0,0);
    std::vector<std::pair<mui::point3d, double>> ptsVluInit;
    ptsVluInit.push_back(std::make_pair(rcv_point,0.0));
    if (cplMethod == "Loose")
	{
        
    } else if (cplMethod == "FixedRelaxation") {
        fr1.initialise(initUndRelxCpl,ptsVluInit);
        fr2.initialise(initUndRelxCpl,ptsVluInit);
        fr3.initialise(initUndRelxCpl,ptsVluInit);
        fr4.initialise(initUndRelxCpl,ptsVluInit);
        fr5.initialise(initUndRelxCpl,ptsVluInit);
        fr6.initialise(initUndRelxCpl,ptsVluInit);

    } else if (cplMethod == "Aitken") {
        aitken1.initialise(initUndRelxCpl,1.0,ptsVluInit);
        aitken2.initialise(initUndRelxCpl,1.0,ptsVluInit);
        aitken3.initialise(initUndRelxCpl,1.0,ptsVluInit);
        aitken4.initialise(initUndRelxCpl,1.0,ptsVluInit);
        aitken5.initialise(initUndRelxCpl,1.0,ptsVluInit);
        aitken6.initialise(initUndRelxCpl,1.0,ptsVluInit);
    } else 
    {
        FatalError << "cplMethod: " << cplMethod << " in fsiDict is not recognized!" << exit(FatalError);

    }
    Info << "==========================================================" <<endl;
    Info << "{OpenFOAM} : Using MUI coupling algorithm " << cplMethod <<endl;
    Info << "==========================================================" <<endl;
    

    
    

}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

Foam::septernion
Foam::solidBodyMotionFunctions::coupled6DoFMotion::transformation() const
{
    auto& runTime = this->time_.db().time();
    auto& ifs = runTime.mui_ifs[ifsID];
    mui::point3d locf( 0.0, 0.0, 0.0 );
    
    
    translationRotationVectors TRV;
    double smllVal=10e-16;
        
    // fsiDict.readIfPresent("cplMethod", cplMethod);
    Info << "==========================================================" <<endl;
    Info << "{OpenFOMA} : 6DoF is fetching values at Time = " << runTime.timeName() << 
        ", and sub-Iteration = "  << runTime.subIter() <<"/" << runTime.subIterationNumber() << endl;

    if (cplMethod == "Loose")

	{
        TRV[0][0] = ifs->fetch( "dispX", locf, runTime.value(),runTime.subIter(), spatial_sampler, chrono_sampler )+smllVal;
        TRV[0][1] = ifs->fetch( "dispY", locf, runTime.value(),runTime.subIter(), spatial_sampler, chrono_sampler )+smllVal;
        TRV[0][2] = ifs->fetch( "dispZ", locf, runTime.value(),runTime.subIter(), spatial_sampler, chrono_sampler )+smllVal;

        TRV[1][0] = ifs->fetch( "angleX", locf, runTime.value(),runTime.subIter(), spatial_sampler, chrono_sampler );
        TRV[1][1] = ifs->fetch( "angleY", locf, runTime.value(),runTime.subIter(), spatial_sampler, chrono_sampler );
        TRV[1][2] = ifs->fetch( "angleZ", locf, runTime.value(),runTime.subIter(), spatial_sampler, chrono_sampler );
    } else if (cplMethod == "Aitken")
	{
        TRV[0][0] = ifs->fetch( "dispX", locf, runTime.value(),runTime.subIter(), spatial_sampler, chrono_sampler,aitken1 )+smllVal;
        TRV[0][1] = ifs->fetch( "dispY", locf, runTime.value(),runTime.subIter(), spatial_sampler, chrono_sampler,aitken2 )+smllVal;
        TRV[0][2] = ifs->fetch( "dispZ", locf, runTime.value(),runTime.subIter(), spatial_sampler, chrono_sampler,aitken3 )+smllVal;

        TRV[1][0] = ifs->fetch( "angleX", locf, runTime.value(),runTime.subIter(), spatial_sampler, chrono_sampler,aitken4 );
        TRV[1][1] = ifs->fetch( "angleY", locf, runTime.value(),runTime.subIter(), spatial_sampler, chrono_sampler,aitken5 );
        TRV[1][2] = ifs->fetch( "angleZ", locf, runTime.value(),runTime.subIter(), spatial_sampler, chrono_sampler,aitken6 );
    } else if (cplMethod == "FixedRelaxation")
    {
        TRV[0][0] = ifs->fetch( "dispX", locf, runTime.value(),runTime.subIter(), spatial_sampler, chrono_sampler,fr1 )+smllVal;
        TRV[0][1] = ifs->fetch( "dispY", locf, runTime.value(),runTime.subIter(), spatial_sampler, chrono_sampler,fr2 )+smllVal;
        TRV[0][2] = ifs->fetch( "dispZ", locf, runTime.value(),runTime.subIter(), spatial_sampler, chrono_sampler,fr3 )+smllVal;

        TRV[1][0] = ifs->fetch( "angleX", locf, runTime.value(),runTime.subIter(), spatial_sampler, chrono_sampler,fr4 );
        TRV[1][1] = ifs->fetch( "angleY", locf, runTime.value(),runTime.subIter(), spatial_sampler, chrono_sampler,fr5 );
        TRV[1][2] = ifs->fetch( "angleZ", locf, runTime.value(),runTime.subIter(), spatial_sampler, chrono_sampler,fr6 );
    }

    Info << "{OpenFOMA} : 6DoF using cupling method " << cplMethod << " with under relaxation of " << initUndRelxCpl<<endl;
    Info << "{OpenFOMA} : 6DoF is fetched discplacement values  "<< TRV << endl;
    Info << "==========================================================" <<endl;
    quaternion R(quaternion::XYZ, TRV[1]);
    septernion TR(septernion(-CofG_ + -TRV[0])*R*septernion(CofG_));
    updateIterCounter();
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
    


    Foam::IOdictionary fsiDict(
        Foam::IOobject(
            "fsiDict",                    // Name of the dictionary file
            this->time_.system(),
            this->time_,       // Read the constant directory
            Foam::IOobject::MUST_READ, // Read the file if it has been modified
            Foam::IOobject::NO_WRITE      // Do not write the dictionary
        )
    );

    cplMethod = static_cast<word>(fsiDict.lookup("cplMethod"));
    initUndRelxCpl = fsiDict.get<scalar>("initUndRelxCpl");
    ifsID = fsiDict.get<int>("ifsID");

    Info << "==========================================" <<endl;
    Info << "======= " << cplMethod << " ================ "<<endl;
    Info << "======= " << initUndRelxCpl << " ================ "<<endl;
    Info << "======= " << ifsID << " ================ "<<endl;
    Info << "==========================================" <<endl;



    // SBMFCoeffs_.readEntry("ifsID", initUndRelxCpl);
    // SBMFCoeffs_.readEntry("ifsID", ifsID);

    // cplMethod = temp;


    return true;
}


// ************************************************************************* //
