/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2015-2016 OpenFOAM Foundation
    Copyright (C) 2020-2021 OpenCFD Ltd.
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

#include "MUICoupledFvPatchField.H"
#include "mui.h"
#include <algorithm>
#include "coupled6DoF_config.h"
#include "muiConfigOF.H"
#include "DynamicList.H"

typedef std::unique_ptr<mui::uniface<mui::coupled6DoF_config>> MuiUnifacePtr3d;
std::vector<MuiUnifacePtr3d> ifs_GeneralBC;
bool ifsInit_GeneralBC = false ; // Bool to check if MUI interface is initiated

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
Foam::MUICoupledFvPatchField<Type>::MUICoupledFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF
)
:
    fixedValueFvPatchField<Type>(p, iF),
    MUIDomainName_("OpenFOAM"),
    MUIInterfaceName_("MUICouledBoundaryCondition"),
    rSampler_(1.0),
    fieldName_(iF.name()),
    field(iF)
{
    WarningInFunction << "MUICoupledFvPatchField uses a default parameteres that might be different from the user's intention"
    << "MUIDomainName = " << MUIDomainName_ << ", MUIInterfaceName = " << MUIInterfaceName_ << ", and rSampler = " << rSampler_ <<endl; 
}

template<class Type>
Foam::MUICoupledFvPatchField<Type>::MUICoupledFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const Field<Type>& fld
)
:
    fixedValueFvPatchField<Type>(p, iF, fld),
    MUIDomainName_("OpenFOAM"),
    MUIInterfaceName_("MUICouledBoundaryCondition"),
    rSampler_(1.0),
    fieldName_(iF.name()),
    field(iF)
{
    WarningInFunction << "MUICoupledFvPatchField uses a default parameteres that might be different from the user's intention"
    << "MUIDomainName = " << MUIDomainName_ << ", MUIInterfaceName = " << MUIInterfaceName_ << ", and rSampler = " << rSampler_ <<endl; 
}

template<class Type>
Foam::MUICoupledFvPatchField<Type>::MUICoupledFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchField<Type>(p, iF, dict, false),
    MUIDomainName_(dict.lookup("MUIDomainName")),
    MUIInterfaceName_(dict.lookup("MUIInterfaceName")),
    rSampler_(readScalar(dict.lookup("rSampler"))),
    fieldName_(iF.name()),
    field(iF)
{
    // Ensure direction vector is normalized
    
    int isMPIInitialized;
    PMPI_Initialized(&isMPIInitialized);
    Info << "================================== MUI Coupling ==================================="<< endl; 
    Info << " OpenFOAM: " << "For patch " <<  this->patch().name() << " an MUI interface to be created "<< endl; 
    Info <<  " OpenFOAM: MUI interface is " << MUIDomainName_ << "/" << MUIInterfaceName_ << endl;
    Info <<  " OpenFOAM: MUI will couple the " <<iF.type() << " that has name " << iF.name() <<endl;
    if (!ifsInit_GeneralBC && isMPIInitialized){
        std::vector<std::string> ifsName;
        ifsName.emplace_back(MUIInterfaceName_);
        ifsInit_GeneralBC = true;
        ifs_GeneralBC=mui::create_uniface<mui::coupled6DoF_config>( MUIDomainName_, ifsName );
    }
    Info << " OpenFOAM: Success.... the intrface "  << MUIDomainName_ << "/" << MUIInterfaceName_ << " has been created" <<endl;
    Info << "================================== MUI Coupling ==================================="<< endl; 

    // Evaluate profile
    //this->evaluate();
    if (dict.found("value"))
    {
       fvPatchField<Type>::operator=(Field<Type>("value", dict, p.size()) );
    }
    else
    {
        fvPatchField<Type>::operator=(this->patchInternalField());
        //evaluate(Pstream::commsTypes::blocking);
    }
    // this->evaluate();
}

template<class Type>
Foam::MUICoupledFvPatchField<Type>::MUICoupledFvPatchField
(
    const MUICoupledFvPatchField<Type>& ptf,
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchField<Type>(p, iF),  // Don't map
    MUIDomainName_(ptf.MUIDomainName_),
    MUIInterfaceName_(ptf.MUIInterfaceName_),
    rSampler_(ptf.rSampler_),
    fieldName_(iF.name()),
    field(iF)

{
    // Evaluate profile since value not mapped
    this->evaluate();
}

template<class Type>
Foam::MUICoupledFvPatchField<Type>::MUICoupledFvPatchField
(
    const MUICoupledFvPatchField<Type>& ptf,
    const DimensionedField<Type, volMesh>& iF
)
:
    fixedValueFvPatchField<Type>(ptf, iF),
    MUIDomainName_(ptf.MUIDomainName_),
    MUIInterfaceName_(ptf.MUIInterfaceName_),
    rSampler_(ptf.rSampler_),
    fieldName_(iF.name()),
    field(iF)
{

}
// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
void Foam::MUICoupledFvPatchField<Type>::updateCoeffs()
{
    if (this->updated())
    {
        return;
    }
    const scalar t = this->db().time().timeOutputValue();
    Field<Type> fethedBCValues(this->patch().Cf().size());
    int isMPIInitialized;

    PMPI_Initialized(&isMPIInitialized);

    Info <<  " FOAM interface " << MUIDomainName_ << "/" << MUIInterfaceName_<< 
    " fetches at time  " <<t << " Field name " << fieldName_ << endl;
    if (ifsInit_GeneralBC && isMPIInitialized)
    { 
        muiFetch(fethedBCValues);
        fvPatchField<Type>::operator=(fethedBCValues);
    } else{
        WarningInFunction << "MUI  not fetch the BC from the interface " 
        << MUIDomainName_ << "/" << MUIInterfaceName_ << endl;
    }
    // Info << fethedBCValues.size
    
    fixedValueFvPatchField<Type>::updateCoeffs();    
}

///////////////////////////////////////////////////////////////////
template<class Type>
void Foam::MUICoupledFvPatchField<Type>::muiFetch(  Foam::scalarField& values) 
{
    const scalar t = this->db().time().timeOutputValue();
    vectorField CellFaceCenter(this->patch().Cf());
    mui::point3d cellCenter;
    mui::temporal_sampler_exact<mui::coupled6DoF_config> chrono_sampler;
    mui::sampler_gauss<mui::coupled6DoF_config> spatial_sampler(rSampler_,rSampler_/2.0);
    
      // Make sure the interface is created and MPI is initialised
    forAll(CellFaceCenter, pointI)
    {
        cellCenter[0] = CellFaceCenter[pointI].x();
        cellCenter[1] = CellFaceCenter[pointI].y();
        cellCenter[2] = CellFaceCenter[pointI].z();

        values[pointI] =  ifs_GeneralBC[0]->fetch( fieldName_, cellCenter, static_cast<double>(t), spatial_sampler, chrono_sampler );
        // std::cout <<  "Foam : Right Locat "<< pointI << " Time " << t<<" ("<< cellCenter[0] << " , "<<   cellCenter[1] << " , "<<  cellCenter[2] << ") ";
        // std::cout << "Vel"<<" ("<< values[pointI].x() << " , "<<   values[pointI].y() << " , "<<  values[pointI].z() << ") "<< std::endl;
    }    return;
}
///////////////////////////////////////////////////////////////////
template<class Type>
void Foam::MUICoupledFvPatchField<Type>::muiFetch(  Foam::vectorField& values) 
{
    const scalar t = this->db().time().timeOutputValue();
    vectorField CellFaceCenter(this->patch().Cf());
    mui::point3d cellCenter;
    mui::temporal_sampler_exact<mui::coupled6DoF_config> chrono_sampler;
    mui::sampler_gauss<mui::coupled6DoF_config> spatial_sampler(rSampler_,rSampler_/2.0);
    
      // Make sure the interface is created and MPI is initialised
    forAll(CellFaceCenter, pointI)
    {
        cellCenter[0] = CellFaceCenter[pointI].x();
        cellCenter[1] = CellFaceCenter[pointI].y();
        cellCenter[2] = CellFaceCenter[pointI].z();

        values[pointI].x() =  ifs_GeneralBC[0]->fetch( fieldName_+"_x", cellCenter, static_cast<double>(t), spatial_sampler, chrono_sampler );
        values[pointI].y() =  ifs_GeneralBC[0]->fetch( fieldName_+"_y", cellCenter, static_cast<double>(t), spatial_sampler, chrono_sampler );
        values[pointI].z() =  ifs_GeneralBC[0]->fetch( fieldName_+"_z", cellCenter, static_cast<double>(t), spatial_sampler, chrono_sampler );
        // std::cout <<  "Foam : Right Locat "<< pointI << " Time " << t<<" ("<< cellCenter[0] << " , "<<   cellCenter[1] << " , "<<  cellCenter[2] << ") ";
        // std::cout << "Vel"<<" ("<< values[pointI].x() << " , "<<   values[pointI].y() << " , "<<  values[pointI].z() << ") "<< std::endl;
    }
    
    return;
}
///////////////////////////////////////////////////////////////////

template<class Type>
void Foam::MUICoupledFvPatchField<Type>::muiFetch(  Foam::tensorField& values) 
{
    FatalError << "FOAM: The field type '"<<  field.type() << "' is not supported for MUI boundary fetch" << endl;
    return;
}

template<class Type>
void Foam::MUICoupledFvPatchField<Type>::muiFetch(  Foam::symmTensorField& values) 
{
    FatalError << "FOAM: The field type '"<<  field.type() << "' is not supported for MUI boundary fetch" << endl;
    return;
}

template<class Type>
void Foam::MUICoupledFvPatchField<Type>::muiFetch(  Foam::sphericalTensorField& values) 
{
    FatalError << "FOAM: The field type '"<<  field.type() << "' is not supported for MUI boundary fetch" << endl;
    return;
}

template<class Type>
void Foam::MUICoupledFvPatchField<Type>::write(Ostream& os) const
{
    fvPatchField<Type>::write(os);
    this->writeEntry("value", os);
    os.writeKeyword("MUIDomainName")  << MUIDomainName_ << token::END_STATEMENT << nl;
    os.writeKeyword("MUIInterfaceName") << MUIInterfaceName_ << token::END_STATEMENT << nl;
    os.writeKeyword("rSampler") << rSampler_ << token::END_STATEMENT << nl;
}


// ************************************************************************* //
