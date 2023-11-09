/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | foam-extend: Open Source CFD
   \\    /   O peration     |
    \\  /    A nd           | For copyright notice see file Copyright
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of foam-extend.

    foam-extend is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    foam-extend is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with foam-extend.  If not, see <http://www.gnu.org/licenses/>.

Class
    coupledVelocityFvPatchVectorField

Description
    Boundary condition specifies vector condition fetched using MUI

SourceFiles
    coupledVelocityFvPatchVectorField.C

Author
    Omar A. Mahfoze.  All rights reserved

\*---------------------------------------------------------------------------*/

#ifndef coupledVelocityFvPatchVectorField_H
#define coupledVelocityFvPatchVectorField_H

#include "fvPatchFields.H"
#include "fixedValueFvPatchFields.H"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
              Class coupledVelocityFvPatchField Declaration
\*---------------------------------------------------------------------------*/

class coupledVelocityFvPatchVectorField
:
    public fixedValueFvPatchVectorField
{
    // Private data

  

        // MUI inteface name
        bool ifsInit_inlet_vel = false ; // Bool to check if MUI interface is initiated
        const word MUIDomainName_ ;
        const word MUIInterfaceName_ ;
        scalar rSampler_;



public:

    //- Runtime type information
    TypeName("coupledVelocity");


    // Constructors

        //- Construct from patch and internal field
        coupledVelocityFvPatchVectorField
        (
            const fvPatch&,
            const DimensionedField<vector, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        coupledVelocityFvPatchVectorField
        (
            const fvPatch&,
            const DimensionedField<vector, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given coupledVelocityFvPatchVectorField
        //  onto a new patch
        coupledVelocityFvPatchVectorField
        (
            const coupledVelocityFvPatchVectorField&,
            const fvPatch&,
            const DimensionedField<vector, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchVectorField> clone() const
        {
            return tmp<fvPatchVectorField>
            (
                new coupledVelocityFvPatchVectorField(*this)
            );
        }

        //- Construct as copy setting internal field reference
        coupledVelocityFvPatchVectorField
        (
            const coupledVelocityFvPatchVectorField&,
            const DimensionedField<vector, volMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<fvPatchVectorField> clone
        (
            const DimensionedField<vector, volMesh>& iF
        ) const
        {
            return tmp<fvPatchVectorField>
            (
                new coupledVelocityFvPatchVectorField(*this, iF)
            );
        }


    // Member functions

        //- Update coefficients
        virtual void updateCoeffs();

        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
