/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2017 OpenFOAM Foundation
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
    coupledVelocityFvPatchVectorField

Description
    Boundary condition specifies vector condition fetched using MUI

SourceFiles
    coupledVelocityFvPatchVectorField.C

Author
    Omar A. Mahfoze.  All rights reserved

\*---------------------------------------------------------------------------*/
// NOTES: 
//    * Rename coupled6DoF_config 
//    * Chech if the MPI_Intiat has been invocted to create MUI interface withen the constractor 
//    * define the samplers in the Dict
//    * Ability to choose the smaplers and there options from the Dict
//    * Clear the inteface baffer every coupled of time steps 
#include "coupledVelocityFvPatchVectorField.H"
#include "addToRunTimeSelectionTable.H"
#include "fvPatchFieldMapper.H"
#include "volFields.H"
#include "surfaceFields.H"
#include "mpi.h"
#include "mui.h"
#include <algorithm>
#include "coupled6DoF_config.h"
#include "muiConfigOF.H"
#include "DynamicList.H"

// DynamicList<mui::uniface<mui::config_2d>*> mui2dInterfaces;
// DynamicList<mui::uniface<mui::config_3d>*> mui3dInterfaces;
// DynamicList<mui::uniface<mui::config_of>*> muiTemplatedInterfaces;


typedef std::unique_ptr<mui::uniface<mui::coupled6DoF_config>> MuiUnifacePtr3d;
std::vector<MuiUnifacePtr3d> ifs_inlet;


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

coupledVelocityFvPatchVectorField::coupledVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchVectorField(p, iF)
{}

coupledVelocityFvPatchVectorField::coupledVelocityFvPatchVectorField
(
    const coupledVelocityFvPatchVectorField& ptf,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchVectorField(ptf, iF)
{}

coupledVelocityFvPatchVectorField::coupledVelocityFvPatchVectorField
(
    const coupledVelocityFvPatchVectorField& ptf,
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchVectorField(ptf, p, iF, mapper),
    MUIDomainName_(ptf.MUIDomainName_),
    MUIInterfaceName_(ptf.MUIInterfaceName_),
    rSampler_(ptf.rSampler_)
{}

coupledVelocityFvPatchVectorField::coupledVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchVectorField(p, iF),
    MUIDomainName_(dict.lookup("MUIDomainName")),
    MUIInterfaceName_(dict.lookup("MUIInterfaceName")),
    rSampler_(readScalar(dict.lookup("rSampler")))


{
    if (dict.found("value"))
    {
       fvPatchField<vector>::operator=(vectorField("value", dict, p.size()) );
    }
    else
    {
        fvPatchField<vector>::operator=(this->patchInternalField());
        //evaluate(Pstream::commsTypes::blocking);
    }
}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void coupledVelocityFvPatchVectorField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    // Define time scalar: t (second)
    const scalar t = this->db().time().timeOutputValue();

    const vectorField r(patch().Cf());

    vectorField  velx(patch().Cf().size(), Foam::vector(0,0,0));
    mui::point3d cellCenter;
    // // vectorField myField(patch().type());

    // Check if an interface has been created, if not create one.
    if (! ifsInit_inlet_vel){
        std::vector<std::string> ifsName;
        ifsName.emplace_back(MUIInterfaceName_);
        ifsInit_inlet_vel = true;
        ifs_inlet=mui::create_uniface<mui::coupled6DoF_config>( MUIDomainName_, ifsName );
    }
    
 
    mui::temporal_sampler_exact<mui::coupled6DoF_config> chrono_sampler;
    mui::sampler_gauss<mui::coupled6DoF_config> spatial_sampler(rSampler_,rSampler_/2.0);
    forAll(patch().Cf(), pointI)
    {
        cellCenter[0] = patch().Cf()[pointI].x();
        cellCenter[1] = patch().Cf()[pointI].y();
        cellCenter[2] = patch().Cf()[pointI].z();

        velx[pointI].x() =  ifs_inlet[0]->fetch( "U_x", cellCenter, static_cast<double>(t), spatial_sampler, chrono_sampler );
        velx[pointI].y() =  ifs_inlet[0]->fetch( "U_y", cellCenter, static_cast<double>(t), spatial_sampler, chrono_sampler );
        velx[pointI].z() =  ifs_inlet[0]->fetch( "U_z", cellCenter, static_cast<double>(t), spatial_sampler, chrono_sampler );

        // std::cout <<  "Foam : Right Locat "<< pointI << " Time " << t<<" ("<< cellCenter[0] << " , "<<   cellCenter[1] << " , "<<  cellCenter[2] << ") ";
        // std::cout << "Vel"<<" ("<< velx[pointI].x() << " , "<<   velx[pointI].y() << " , "<<  velx[pointI].z() << ") "<< std::endl;
        
    }
    vectorField::operator=(velx);    
    
}



// Write
void coupledVelocityFvPatchVectorField::write(Ostream& os) const
{
    fvPatchVectorField::write(os);
    os.writeKeyword("MUIDomainName")
        << MUIDomainName_ << token::END_STATEMENT << nl;
    os.writeKeyword("MUIInterfaceName")
        << MUIInterfaceName_ << token::END_STATEMENT << nl;
    os.writeKeyword("rSampler")
        << rSampler_ << token::END_STATEMENT << nl;
        
    writeEntry("value", os);  
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePatchTypeField(fvPatchVectorField, coupledVelocityFvPatchVectorField);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
