/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2018 OpenFOAM Foundation
     \\/     M anipulation  |
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

#include "parabolicInletVelocityFvPatchVectorField.H"
#include "volFields.H"
#include "addToRunTimeSelectionTable.H"
#include "fvPatchFieldMapper.H"
#include "surfaceFields.H"
#include "mathematicalConstants.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::parabolicInletVelocityFvPatchVectorField::
parabolicInletVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchField<vector>(p, iF),
    maxVelocity_(0),
 //   Tperiod_(0),
	axis_(pTraits<vector>::zero),
	origin_(pTraits<vector>::zero)
{}


Foam::parabolicInletVelocityFvPatchVectorField::
parabolicInletVelocityFvPatchVectorField
(
    const parabolicInletVelocityFvPatchVectorField& ptf,
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchField<vector>(ptf, p, iF, mapper),
    maxVelocity_(ptf.maxVelocity_),
//	Tperiod_(ptf.Tperiod_),
	axis_(ptf.axis_),
	origin_(ptf.origin_)
{}


Foam::parabolicInletVelocityFvPatchVectorField::
parabolicInletVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchField<vector>(p, iF, dict),
	maxVelocity_(readScalar(dict.lookup("maxVelocity"))),
	axis_(dict.lookup("axis")),
	origin_(dict.lookup("origin"))
{}

Foam::parabolicInletVelocityFvPatchVectorField::
parabolicInletVelocityFvPatchVectorField
(
    const parabolicInletVelocityFvPatchVectorField& ptf
)
:
    fixedValueFvPatchField<vector>(ptf),
	maxVelocity_(ptf.maxVelocity_),
//	Tperiod_(ptf.Tperiod_),
	axis_(ptf.axis_),
	origin_(ptf.origin_)
{}


Foam::parabolicInletVelocityFvPatchVectorField::
parabolicInletVelocityFvPatchVectorField
(
    const parabolicInletVelocityFvPatchVectorField& ptf,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchField<vector>(ptf, iF),
	maxVelocity_(ptf.maxVelocity_),
//	Tperiod_(ptf.Tperiod_),
	axis_(ptf.axis_),
	origin_(ptf.origin_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::parabolicInletVelocityFvPatchVectorField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    const scalar t = this->db().time().timeOutputValue();

    const vector axisHat = axis_/mag(axis_);

    const vectorField r(patch().Cf() - origin_);

	scalar vMax;

	scalar Tperiod_ = 1.0;

	if (t < Tperiod_)
	{
		vMax = maxVelocity_ * (1.0 - std::cos(constant::mathematical::pi*(t/Tperiod_))) / 2.0;
	}
	else
	{
		vMax = maxVelocity_;
	}
	
    operator==(axisHat * (vMax*r.component(vector::Y)*(0.4-r.component(vector::Y))*
				(sqr(0.4)-sqr(r.component(vector::Z))))/
				(pow(0.2,2.0)*sqr(0.4)));

    fixedValueFvPatchField<vector>::updateCoeffs();
}


void Foam::parabolicInletVelocityFvPatchVectorField::write(Ostream& os) const
{
    fvPatchField<vector>::write(os);
    os.writeKeyword("maxVelocity") << maxVelocity_ << token::END_STATEMENT << nl;
    os.writeKeyword("axis") << axis_ << token::END_STATEMENT << nl;
    os.writeKeyword("origin") << origin_ << token::END_STATEMENT << nl;
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
   makePatchTypeField
   (
       fvPatchVectorField,
       parabolicInletVelocityFvPatchVectorField
   );
}


// ************************************************************************* //
