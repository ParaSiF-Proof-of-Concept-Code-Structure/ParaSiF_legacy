/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
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

InNamespace
    Foam

Description
    Interpolates y values from one curve to another with a different x
    distribution.

    Uses Catmull-Rom spline interpolation between points.

SourceFiles
    interpolateCubicSplineXY.C

\*---------------------------------------------------------------------------*/

#ifndef interpolateCubicSplineXY_H
#define interpolateCubicSplineXY_H

#include "scalar.H"
#include "primitiveFieldsFwd.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

scalarField  thomasAlgorithm
(
     scalarField a,
     scalarField b,
     scalarField c,
     scalarField d
);

template<class Type>
Field<Type>  thomasAlgorithm_Field
(
     scalarField& a,
     scalarField& b,
     scalarField& c,
     Field<Type>& d
);

template<class Type>
Field<Type> interpolateCubicSplineXY
(
    const scalarField& xNew,
    const scalarField& xOld,
    const Field<Type>& yOld
);


template<class Type>
Type interpolateCubicSplineXY
(
    const scalar x,
    const scalarField& xOld,
    const Field<Type>& yOld
);



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "interpolateCubicSplineXY.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
