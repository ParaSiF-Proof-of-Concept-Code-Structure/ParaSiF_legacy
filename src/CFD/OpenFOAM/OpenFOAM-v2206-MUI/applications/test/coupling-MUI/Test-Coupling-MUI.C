/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2019 OpenCFD Ltd.
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

Description
    Example demonstrating a coupling using the MUI code coupling library

\*---------------------------------------------------------------------------*/

#include "../../../src/coupling/couplingMUI2d/couplingMUI2d.H"
#include "../../../src/coupling/couplingMUI3d/couplingMUI3d.H"
#include "OSspecific.H"
#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.
#include "mui.h"
#endif


using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
// Main program:

int main(int argc, char *argv[])
{
    couplingInterface2d twoDInterfaces;
    couplingInterface3d threeDInterfaces;
#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.
    #include createCoupling.H
#endif

    return 0;
}


// ************************************************************************* //
