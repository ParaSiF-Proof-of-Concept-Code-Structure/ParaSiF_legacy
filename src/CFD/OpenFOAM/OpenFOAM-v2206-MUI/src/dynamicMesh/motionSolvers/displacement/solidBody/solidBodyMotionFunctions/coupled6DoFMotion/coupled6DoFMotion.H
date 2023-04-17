/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
    Copyright (C) 2020 OpenCFD Ltd.
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

Class
    Foam::solidBodyMotionFunctions::coupled6DoFMotion

Description
    coupled 6DoF motion function.

    Obtained by interpolating coupled data for surge (x-translation),
    sway (y-translation), heave (z-translation), roll (rotation about x),
    pitch (rotation about y) and yaw (rotation about z).

SourceFiles
    coupled6DoFMotion.C

\*---------------------------------------------------------------------------*/

#ifndef coupled6DoFMotion_H
#define coupled6DoFMotion_H

#include "solidBodyMotionFunction.H"
#include "primitiveFields.H"
#include "Vector2D.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace solidBodyMotionFunctions
{

/*---------------------------------------------------------------------------*\
                          Class coupled6DoFMotion Declaration
\*---------------------------------------------------------------------------*/

class coupled6DoFMotion
:
    public solidBodyMotionFunction
{
    // Private data

        //- Time data file name read from dictionary
        fileName timeDataFileName_;

        //- Center of gravity read from dictionary
        vector CofG_;

        //- Type used to read in the translation and rotation "vectors"
        typedef Vector2D<vector> translationRotationVectors;

        //- Field of times
        scalarField times_;

        //- Field of translation and rotation "vectors"
        Field<translationRotationVectors> values_;


    // Private Member Functions

        //- No copy construct
        coupled6DoFMotion(const coupled6DoFMotion&) = delete;

        //- No copy assignment
        void operator=(const coupled6DoFMotion&) = delete;


public:

    //- Runtime type information
    TypeName("coupled6DoFMotion");


    // Constructors

        //- Construct from components
        coupled6DoFMotion
        (
            const dictionary& SBMFCoeffs,
            const Time& runTime
        );

        //- Construct and return a clone
        virtual autoPtr<solidBodyMotionFunction> clone() const
        {
            return autoPtr<solidBodyMotionFunction>
            (
                new coupled6DoFMotion
                (
                    SBMFCoeffs_,
                    time_
                )
            );
        }


    //- Destructor
    virtual ~coupled6DoFMotion() = default;


    // Member Functions

        //- Return the solid-body motion transformation septernion
        virtual septernion transformation() const;

        //- Update properties from given dictionary
        virtual bool read(const dictionary& SBMFCoeffs);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace solidBodyMotionFunctions
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
