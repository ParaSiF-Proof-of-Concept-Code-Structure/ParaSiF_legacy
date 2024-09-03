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
#include "DynamicList.H"
#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.
#include "mui.h"
#include "muiconfig.h"
#endif
// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
Foam::MUICoupledFvPatchField<Type>::MUICoupledFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF
)
:
    fixedValueFvPatchField<Type>(p, iF),
    ifsID(0),
    spatialSampler_("exact"),
    tempSampler_("exact"),
    rSpatialSampler_(1.0),
    rTempSampler_(1.0),
    MUIForgetTime_(1.0),
    numFetchStepsDelay_(0),
    fieldName_(iF.name())
{
WarningInFunction << "MUICoupledFvPatchField uses a default parameteres that might be different from the user's intention"
    << "MUIInterfaceID = " << ifsID  << rSpatialSampler_ <<endl;
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
    ifsID(0),
    spatialSampler_("exact"),
    tempSampler_("exact"),
    rSpatialSampler_(1.0),
    rTempSampler_(1.0),
    MUIForgetTime_(1.0),
    numFetchStepsDelay_(0),
    fieldName_(iF.name())
{    WarningInFunction << "MUICoupledFvPatchField uses a default parameteres that might be different from the user's intention"
    << "MUIInterfaceID = " << ifsID << ", and rSpatialSampler_ = " << rSpatialSampler_ <<endl;
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
    ifsID(readScalar(dict.lookup("MUIInterfaceID"))),
    spatialSampler_(dict.lookup("spatialSampler")),
    tempSampler_(dict.lookup("tempSampler")),
    rSpatialSampler_(readScalar(dict.lookup("rSpatialSampler"))),
    rTempSampler_(readScalar(dict.lookup("rTempSampler"))),
    MUIForgetTime_(readScalar(dict.lookup("forgetTime"))),
    numFetchStepsDelay_(readScalar(dict.lookup("numFetchStepsDelay"))),
    fieldName_(iF.name())
{
    int isMPIInitialized;
    PMPI_Initialized(&isMPIInitialized);


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
    // Evaluate profile
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
    ifsID(ptf.ifsID),
    spatialSampler_(ptf.spatialSampler_),
    tempSampler_(ptf.tempSampler_),
    rSpatialSampler_(ptf.rSpatialSampler_),
    rTempSampler_(ptf.rTempSampler_),
    MUIForgetTime_(ptf.MUIForgetTime_),
    numFetchStepsDelay_(ptf.numFetchStepsDelay_),
    fieldName_(iF.name())
{
    // Evaluate profile since value not mapped
    this->evaluate();
}


template<class Type>
Foam::MUICoupledFvPatchField<Type>::MUICoupledFvPatchField
(
    const MUICoupledFvPatchField<Type>& ptf
)
:
    fixedValueFvPatchField<Type>(ptf),
    ifsID(ptf.ifsID),
    spatialSampler_(ptf.spatialSampler_),
    tempSampler_(ptf.tempSampler_),
    rSpatialSampler_(ptf.rSpatialSampler_),
    rTempSampler_(ptf.rTempSampler_),    
    MUIForgetTime_(ptf.MUIForgetTime_),
    numFetchStepsDelay_(ptf.numFetchStepsDelay_)
{}


template<class Type>
Foam::MUICoupledFvPatchField<Type>::MUICoupledFvPatchField
(
    const MUICoupledFvPatchField<Type>& ptf,
    const DimensionedField<Type, volMesh>& iF
)
:
    fixedValueFvPatchField<Type>(ptf, iF),
    ifsID(ptf.ifsID),
    spatialSampler_(ptf.spatialSampler_),
    tempSampler_(ptf.tempSampler_),
    rSpatialSampler_(ptf.rSpatialSampler_),
    rTempSampler_(ptf.rTempSampler_),    
    MUIForgetTime_(ptf.MUIForgetTime_),
    numFetchStepsDelay_(ptf.numFetchStepsDelay_),
    fieldName_(iF.name())
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
    // scalar t = this->db().time().timeOutputValue() 
    //          - this->db().time().deltaT().value() * numFetchStepsDelay_;
    Field<Type> fethedBCValues(this->patch().Cf().size(),Zero);
    int isMPIInitialized;

    PMPI_Initialized(&isMPIInitialized);

    
    if (this->db().time().isMUIIfsInit && isMPIInitialized)
    { 
        muiFetch(fethedBCValues);
        fvPatchField<Type>::operator=(fethedBCValues);
    } else{
        fvPatchField<Type>::operator=(fethedBCValues);
        WarningInFunction << "MUI  did not fetch the BC from the interface " 
        << ifsID  << endl;

    }
    // Info << fethedBCValues.size
    
    fixedValueFvPatchField<Type>::updateCoeffs(); 
}

///////////////////////////////////////////////////////////////////
template<class Type>
void Foam::MUICoupledFvPatchField<Type>::muiFetch(  Foam::scalarField& values) 
{
    // scalar t = this->db().time().timeOutputValue();
    // t =t - this->db().time().deltaT().value() * numFetchStepsDelay_;
    vectorField CellFaceCenter(this->patch().Cf());

    scalar time = this->db().time().value();
    int subIter = this->db().time().subIter();

    if (this->db().time().subIterationNumber() > 1){
        subIter = subIter - numFetchStepsDelay_;
    } else{
        time = time - this->db().time().deltaT().value() * numFetchStepsDelay_;
    }

    
#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.
    auto& ifs = this->db().time().mui_ifs[ifsID];
    mui::point3d cellCenter;
    mui::temporal_sampler_exact<mui::mui_config> chrono_sampler;
    mui::sampler_gauss<mui::mui_config> spatial_sampler(rSpatialSampler_,rSpatialSampler_/2.0);

      // Make sure the interface is created and MPI is initialised
    if (time>=0){
        forAll(CellFaceCenter, pointI)
        {
            cellCenter[0] = CellFaceCenter[pointI].x();
            cellCenter[1] = CellFaceCenter[pointI].y();
            cellCenter[2] = CellFaceCenter[pointI].z();

            values[pointI] =  ifs->fetch( fieldName_, cellCenter, time,subIter, spatial_sampler, chrono_sampler );
            // std::cout <<  "Foam : Right Locat "<< pointI << " Time " << t<<" ("<< cellCenter[0] << " , "<<   cellCenter[1] << " , "<<  cellCenter[2] << ") ";
            // std::cout << "Vel"<<" ("<< values[pointI].x() << " , "<<   values[pointI].y() << " , "<<  values[pointI].z() << ") "<< std::endl;
        }   
    } 
#endif
    // return;
}
///////////////////////////////////////////////////////////////////
template<class Type>
void Foam::MUICoupledFvPatchField<Type>::muiFetch(  Foam::vectorField& values)
{
    // const scalar t = this->db().time().timeOutputValue() 
    //                - this->db().time().deltaT().value() * numFetchStepsDelay_;

    scalar time = this->db().time().value();
    int subIter = this->db().time().subIter();

    if (subIter - numFetchStepsDelay_ <= 0) {
        subIter = subIter - numFetchStepsDelay_ + this->db().time().subIterationNumber();
        time = time - this->db().time().deltaT().value() * numFetchStepsDelay_;
    } else {
        subIter = subIter - numFetchStepsDelay_;
    }
    // if (this->db().time().subIterationNumber() > 1){
    //     subIter = subIter - numFetchStepsDelay_;
    // } else{
    //     time = time - this->db().time().deltaT().value() * numFetchStepsDelay_;
    // }

    vectorField CellFaceCenter(this->patch().Cf());
#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.
    auto& ifs = this->db().time().mui_ifs[ifsID];
    mui::point3d cellCenter;
    mui::temporal_sampler_exact<mui::mui_config> chrono_sampler;
    mui::sampler_gauss<mui::mui_config> spatial_sampler(rSpatialSampler_,rSpatialSampler_/2.0);   

    // Make sure the interface is created and MPI is initialised
    Info <<  " {OpenFOAM} domain " << this->db().time().MUIDomainName() << 
    " fetches at time  " <<time << " and sub iter "<< subIter <<" Field name " << fieldName_ << endl;
    
    if (time > 0 ){
        forAll(CellFaceCenter, pointI)
        {
            cellCenter[0] = CellFaceCenter[pointI].x();
            cellCenter[1] = CellFaceCenter[pointI].y();
            cellCenter[2] = CellFaceCenter[pointI].z();

            values[pointI].x() =  ifs->fetch( fieldName_+"_x", cellCenter, time,subIter, spatial_sampler, chrono_sampler );
            
            values[pointI].y() =  ifs->fetch( fieldName_+"_y", cellCenter, time,subIter, spatial_sampler, chrono_sampler );
            
            values[pointI].z() =  ifs->fetch( fieldName_+"_z", cellCenter, time,subIter, spatial_sampler, chrono_sampler );
        }
        if (this->db().time().value() >= MUIForgetTime_){
            ifs->forget (this->db().time().value() - MUIForgetTime_ );
        }
    }else{
        WarningInFunction << " {OpenFOAM} domain " << this->db().time().MUIDomainName() <<" : MUI  did not fetch the BC from the interface " 
        << ifsID  << "  Values are set to zero " << endl;
        forAll(CellFaceCenter, pointI)
        {
            if (this->db().time().MUIDomainName() =="topDomain") Pout << "MUI in : Rank ------=== " << Pstream::myProcNo() <<endl;
            values[pointI].x() =  0.0;
            values[pointI].y() =  0.0;
            values[pointI].z() =  0.0;
        }
    }
#endif
    // return;
}
///////////////////////////////////////////////////////////////////

template<class Type>
void Foam::MUICoupledFvPatchField<Type>::muiFetch(  Foam::tensorField& values) 
{
    FatalError << "FOAM: The field type 'tensorField' is not supported for MUI boundary fetch" << endl;
    return;
}
///////////////////////////////////////////////////////////////////

template<class Type>
void Foam::MUICoupledFvPatchField<Type>::muiFetch(  Foam::symmTensorField& values) 
{
    FatalError << "FOAM: The field type 'symmTensorField' is not supported for MUI boundary fetch" << endl;
    return;
}
///////////////////////////////////////////////////////////////////

template<class Type>
void Foam::MUICoupledFvPatchField<Type>::muiFetch(  Foam::sphericalTensorField& values) 
{
    FatalError << "FOAM: The field type  'sphericalTensorField' is not supported for MUI boundary fetch" << endl;
    return;
}
///////////////////////////////////////////////////////////////////

template<class Type>
void Foam::MUICoupledFvPatchField<Type>::write(Ostream& os) const
{
    fvPatchField<Type>::write(os);
    this->writeEntry("value", os);
    os.writeKeyword("MUIInterfaceID")  << ifsID << token::END_STATEMENT << nl;
    os.writeKeyword("spatialSampler") << spatialSampler_ << token::END_STATEMENT << nl;
    os.writeKeyword("tempSampler") << tempSampler_ << token::END_STATEMENT << nl;
    os.writeKeyword("rSpatialSampler") << rSpatialSampler_ << token::END_STATEMENT << nl;
    os.writeKeyword("rTempSampler") << rTempSampler_ << token::END_STATEMENT << nl;
    os.writeKeyword("numFetchStepsDelay") << numFetchStepsDelay_ << token::END_STATEMENT << nl;
    os.writeKeyword("forgetTime") << MUIForgetTime_ << token::END_STATEMENT << nl;
}


// ************************************************************************* //
