/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2017 OpenFOAM Foundation
    Copyright (C) 2016-2022 OpenCFD Ltd.
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

#include "sampledSurfacesCoupled.H"
#include "polySurface.H"
#include "primitivePatchInterpolation.H"
#include "mapPolyMesh.H"
#include "volFields.H"
#include "surfaceFields.H"
#include "ListOps.H"
#include "Time.H"
#include "IndirectList.H"
#include "addToRunTimeSelectionTable.H"

#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.
#include "mui.h"
#include "muiconfig.h"
#endif


//*************************  MUI interface ***********************************//



// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //
namespace Foam
{
    defineTypeNameAndDebug(sampledSurfacesCoupled, 0);

    addToRunTimeSelectionTable
    (
        functionObject,
        sampledSurfacesCoupled,
        dictionary
    );
}

Foam::scalar Foam::sampledSurfacesCoupled::mergeTol_ = 1e-10;

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //



Foam::autoPtr<Foam::surfaceWriter> Foam::sampledSurfacesCoupled::newWriter
(
    word writeType,
    const dictionary& formatOptions,
    const dictionary& surfDict
)
{
    // Per-surface adjustment
    surfDict.readIfPresent<word>("surfaceFormat", writeType);

    dictionary options = formatOptions.subOrEmptyDict(writeType);

    options.merge
    (
        surfDict.subOrEmptyDict("formatOptions").subOrEmptyDict(writeType)
    );

    return surfaceWriter::New(writeType, options);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::sampledSurfacesCoupled::sampledSurfacesCoupled
(
    const word& name,
    const Time& runTime,
    const dictionary& dict
)
:
    functionObjects::fvMeshFunctionObject(name, runTime, dict),
    PtrList<sampledSurface>(),
    loadFromFiles_(false),
    verbose_(false),
    onExecute_(false),
    outputPath_
    (
        time_.globalPath()/functionObject::outputPrefix/name
    ),
    fieldSelection_(),
    sampleFaceScheme_(),
    sampleNodeScheme_(),
    writers_(),
    actions_(),
    nFaces_()
{
    outputPath_.clean();  // Remove unneeded ".."

    
    read(dict);


    }


Foam::sampledSurfacesCoupled::sampledSurfacesCoupled
(
    const word& name,
    const objectRegistry& obr,
    const dictionary& dict,
    const bool loadFromFiles
)
:
    functionObjects::fvMeshFunctionObject(name, obr, dict),
    PtrList<sampledSurface>(),
    loadFromFiles_(loadFromFiles),
    verbose_(false),
    onExecute_(false),
    outputPath_
    (
        time_.globalPath()/functionObject::outputPrefix/name
    ),
    fieldSelection_(),
    sampleFaceScheme_(),
    sampleNodeScheme_(),
    writers_(),
    actions_(),
    nFaces_()
{
    outputPath_.clean();  // Remove unneeded ".."

    read(dict);
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::sampledSurfacesCoupled::verbose(const bool on) noexcept
{
    bool old(verbose_);
    verbose_ = on;
    return old;
}



bool Foam::sampledSurfacesCoupled::performAction(unsigned request)  //  1- Called by bool Foam::sampledSurfacesCoupled::write()
{
    // Update surfaces and store
    bool ok = false;

    forAll(*this, surfi)
    {
        sampledSurface& s = (*this)[surfi];

        if (request & actions_[surfi])
        {
            if (s.update())
            {
                writers_[surfi].expire();
            }

            nFaces_[surfi] = returnReduce(s.faces().size(), sumOp<label>());

            ok = ok || nFaces_[surfi];
            // Store surfaces (even empty ones) otherwise we miss geometry
            // updates.
            // Any associated fields will be removed if the size changes

            if ((request & actions_[surfi]) & ACTION_STORE)
            {
                storeRegistrySurface(s);
            }
        }
    }
    if (!ok)
    {
        // No surface with an applicable action or with faces to sample
        Info << "============================ MUI coupling ============================" <<endl;
        Info <<"Warniung: OpenFoam,  Domain "<< time_.MUIDomainName() <<" :No surface with an applicable action or with faces to sample" << endl;
        Info << "======================================================================" <<endl;
        return true;
    }


    // Determine availability of fields.
    // Count per-surface number of fields, including Ids etc
    // which only seems to be needed for VTK legacy

    IOobjectList objects = preCheckFields();

    // Update writers

    forAll(*this, surfi)
    {
        const sampledSurface& s = (*this)[surfi];

        if (((request & actions_[surfi]) & ACTION_WRITE) && nFaces_[surfi])
        {
            surfaceWriter& outWriter = writers_[surfi];

            if (outWriter.needsUpdate())
            {
                outWriter.setSurface(s);
            }
            outWriter.open(outputPath_/s.name());
            outWriter.beginTime(obr_.time());
            // Face-id field, but not if the writer manages that itself
            if (!s.isPointData() && s.hasFaceIds() && !outWriter.usesFaceIds())
            {
                // This is still quite horrible.

                Field<label> ids(s.faceIds());

                if
                (
                    returnReduce
                    (
                        !ListOps::found(ids, lessOp1<label>(0)),
                        andOp<bool>()
                    )
                )
                {
                    // From 0-based to 1-based, provided there are no
                    // negative ids (eg, encoded solid/side)

                    ids += label(1);
                }
                writeSurface(outWriter, ids, "Ids");
            }

        }
    }
#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.
    // Sample fields
    int isMPIInitialized;
    PMPI_Initialized(&isMPIInitialized);
    if (time_.isMUIIfsInit && isMPIInitialized)
    { 
    performAction<volScalarField>(objects, request);
    performAction<volVectorField>(objects, request);
    // performAction<volSphericalTensorField>(objects, request);
    // performAction<volSymmTensorField>(objects, request);
    // performAction<volTensorField>(objects, request);


        // time_.mui_ifs[0]->commit( time_.time().timeOutputValue() );
        time_.mui_ifs[0]->commit( time_.value(), time_.subIter() );
        //time_.mui_ifs[0]->forget(???);
        Info << "{OpenFoam} : Domain "<< time_.MUIDomainName()<<" Commited surface sampler at time "<< time_.value() 
        << " and sub iter " << time_.subIter() <<"/" << time_.subIterationNumber() << endl;
    } else{
        FatalError << "MUICoupledSurface smapling: MUI is not initialised "<< endl;
    }

    
#endif

    return true;
}


bool Foam::sampledSurfacesCoupled::write()
{
    return performAction(ACTION_ALL);
}


void Foam::sampledSurfacesCoupled::updateMesh(const mapPolyMesh& mpm)
{
    if (&mpm.mesh() == &mesh_)
    {
        expire();
    }

    // pointMesh and interpolation will have been reset in mesh.update
}

// ************************************************************************* //

template<class GeoField>
void Foam::sampledSurfacesCoupled::performAction
(
    const IOobjectList& objects,
    unsigned request
)
{
    wordList fieldNames;
        fieldNames = mesh_.thisDb().sortedNames<GeoField>(fieldSelection_);
    

    for (const word& fieldName : fieldNames)
    {
        if (verbose_)
        {
            Info<< "sampleWrite: " << fieldName << endl;
        }        
        performAction
        (
            mesh_.thisDb().lookupObject<GeoField>(fieldName),
            request
        );
        
    }
}

// **********************************************************************************************

template<class Type>
void Foam::sampledSurfacesCoupled::performAction
(
    const VolumeField<Type>& fld,
    unsigned request
)
{
    // The sampler for this field
    autoPtr<interpolation<Type>> samplePtr;

    // The interpolator for this field
    autoPtr<interpolation<Type>> interpPtr;

    const word& fieldName = fld.name();

    const dimensionSet& dims = fld.dimensions();
    forAll(*this, surfi)
    {
        const sampledSurface& s = operator[](surfi);

        // Skip surface without faces (eg, failed cut-plane)
        if (!nFaces_[surfi])
        {
            continue;
        }

        Field<Type> values;
        


        if (s.isPointData())
        {
            if (!interpPtr)
            {
                interpPtr = interpolation<Type>::New
                (
                    sampleNodeScheme_,
                    fld
                );
            }
            
            values = s.interpolate(*interpPtr);
        }
        else
        {
            if (!samplePtr)
            {
                samplePtr = interpolation<Type>::New
                (
                    sampleFaceScheme_,
                    fld
                );
            }

            values = s.sample(*samplePtr);
        }

        if ((request & actions_[surfi]) & ACTION_WRITE)
        {
            muiPush<Type>(writers_[surfi], values,s.points(), fieldName);
        }

        if ((request & actions_[surfi]) & ACTION_STORE)
        {
            if (s.isPointData())
            {
                storeRegistryField<Type, polySurfacePointGeoMesh>
                (
                    s, fieldName, dims, std::move(values)
                );
            }
            else
            {
                storeRegistryField<Type, polySurfaceGeoMesh>
                (
                    s, fieldName, dims, std::move(values)
                );
            }
        }
    }
}


//// Push vector field /////////////////////
template<class Type> void Foam::sampledSurfacesCoupled::muiPush
(
    surfaceWriter& writer,
    const Field<Foam::Vector<double>>& values,
    const Field<Foam::Vector<double>>& points,
    const word& fieldName
)
{

  

#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.


    mui::point3d cellCenter;
    forAll(points, indx)
    {
        
        cellCenter[0] = points[indx][0];
        cellCenter[1] = points[indx][1];
        cellCenter[2] = points[indx][2];
        time_.mui_ifs[0]->push(fieldName+"_x", cellCenter, values[indx][0]);
        time_.mui_ifs[0]->push(fieldName+"_y", cellCenter, values[indx][1]);
        time_.mui_ifs[0]->push(fieldName+"_z", cellCenter, values[indx][2]);
    }
#endif
    
    if (writeIterfaceSurfaceFile_){
        fileName outputName = writer.write(fieldName, values);
        dictionary propsDict;
        propsDict.add
        (
            "file",
            time_.relativePath(outputName, true)
        );
        setProperty(fieldName, propsDict);
    }
}

///                Push scaler Field     
template<class Type> void Foam::sampledSurfacesCoupled::muiPush
(
    surfaceWriter& writer,
    const Field<double>& values,
    const Field<Foam::Vector<double>>& points,
    const word& fieldName
)
{

#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.

    mui::point3d cellCenter;
    forAll(points, indx)
    {
        cellCenter[0] = points[indx][0];
        cellCenter[1] = points[indx][1];
        cellCenter[2] = points[indx][2];
        time_.mui_ifs[0]->push(fieldName, cellCenter, values[indx]);
    }
#endif
    if (writeIterfaceSurfaceFile_){
        fileName outputName = writer.write(fieldName, values);
        dictionary propsDict;
        propsDict.add
        (
            "file",
            time_.relativePath(outputName, true)
        );
        setProperty(fieldName, propsDict);
    }
}


