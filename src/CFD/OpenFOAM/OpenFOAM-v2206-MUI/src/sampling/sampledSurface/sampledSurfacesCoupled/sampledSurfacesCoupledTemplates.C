/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
    Copyright (C) 2015-2022 OpenCFD Ltd.
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
#include "volFields.H"
#include "surfaceFields.H"
#include "polySurfaceFields.H"
#include "HashOps.H"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

template<class Type>
void Foam::sampledSurfacesCoupled::writeSurface
(
    surfaceWriter& writer,
    const Field<Type>& values,
    const word& fieldName
)
{
    fileName outputName = writer.write(fieldName, values);
    // Case-local file name with "<case>" to make relocatable

    dictionary propsDict;
    propsDict.add
    (
        "file",
        time_.relativePath(outputName, true)
    );
    setProperty(fieldName, propsDict);
}


template<class Type, class GeoMeshType>
bool Foam::sampledSurfacesCoupled::storeRegistryField
(
    const sampledSurface& s,
    const word& fieldName,
    const dimensionSet& dims,
    Field<Type>&& values
)
{
    return s.storeRegistryField<Type, GeoMeshType>
    (
        storedObjects(),
        fieldName,
        dims,
        std::move(values),
        IOobject::groupName(name(), s.name())
    );
}





template<class Type>
void Foam::sampledSurfacesCoupled::performAction
(
    const SurfaceField<Type>& fld,
    unsigned request
)
{
    Info << "void Foam::sampledSurfacesCoupled::performAction template<class Type>" <<endl;
    const word& fieldName = fld.name();

    const dimensionSet& dims = fld.dimensions();

    forAll(*this, surfi)
    {
        const sampledSurface& s = (*this)[surfi];

        // Skip surface without faces (eg, failed cut-plane)
        if (!nFaces_[surfi])
        {
            continue;
        }

        Field<Type> values(s.sample(fld));

        if ((request & actions_[surfi]) & ACTION_WRITE)
        {
            writeSurface<Type>(writers_[surfi], values, fieldName);
        }

        if ((request & actions_[surfi]) & ACTION_STORE)
        {
            storeRegistryField<Type, polySurfaceGeoMesh>
            (
                s, fieldName, dims, std::move(values)
            );
        }
    }
}





template<class Container, class Predicate>
bool Foam::sampledSurfacesCoupled::testAny
(
    const Container& items,
    const Predicate& pred
)
{
    for (const auto& item : items)
    {
        if (pred(item))
        {
            return true;
        }
    }

    return false;
}


// ************************************************************************* //
Foam::polySurface* Foam::sampledSurfacesCoupled::getRegistrySurface
(
    const sampledSurface& s
) const
{
    return s.getRegistrySurface
    (
        storedObjects(),
        IOobject::groupName(name(), s.name())
    );
}

bool Foam::sampledSurfacesCoupled::execute()
{
    if (onExecute_)
    {
        return performAction(ACTION_ALL & ~ACTION_WRITE);
    }

    return true;
}



void Foam::sampledSurfacesCoupled::movePoints(const polyMesh& mesh)
{
    if (&mesh == &mesh_)
    {
        expire();
    }
}


void Foam::sampledSurfacesCoupled::readUpdate(const polyMesh::readUpdateState state)
{
    if (state != polyMesh::UNCHANGED)
    {
        // May want to use force expiration here
        expire();
    }
}


bool Foam::sampledSurfacesCoupled::needsUpdate() const
{
    for (const sampledSurface& s : surfaces())
    {
        if (s.needsUpdate())
        {
            return true;
        }
    }

    return false;
}


bool Foam::sampledSurfacesCoupled::expire(const bool force)
{
    // Dimension as fraction of mesh bounding box
    const scalar mergeDim = mergeTol_ * mesh_.bounds().mag();

    label nChanged = 0;

    forAll(*this, surfi)
    {
        sampledSurface& s = (*this)[surfi];

        if (s.invariant() && !force)
        {
            // 'Invariant' - does not change when geometry does
            continue;
        }
        if (s.expire())
        {
            ++nChanged;
        }

        writers_[surfi].expire();
        writers_[surfi].mergeDim(mergeDim);
        nFaces_[surfi] = 0;
    }

    // True if any surfaces just expired
    return nChanged;
}


bool Foam::sampledSurfacesCoupled::update()
{
    if (!needsUpdate())
    {
        return false;
    }

    label nUpdated = 0;

    forAll(*this, surfi)
    {
        sampledSurface& s = (*this)[surfi];

        if (s.update())
        {
            ++nUpdated;
            writers_[surfi].expire();
        }

        nFaces_[surfi] = returnReduce(s.faces().size(), sumOp<label>());
    }

    return nUpdated;
}


Foam::scalar Foam::sampledSurfacesCoupled::mergeTol() noexcept
{
    return mergeTol_;
}


Foam::scalar Foam::sampledSurfacesCoupled::mergeTol(const scalar tol) noexcept
{
    const scalar old(mergeTol_);
    mergeTol_ = tol;
    return old;
}

bool Foam::sampledSurfacesCoupled::read(const dictionary& dict)
{
    fvMeshFunctionObject::read(dict);

    PtrList<sampledSurface>::clear();
    writers_.clear();
    actions_.clear();
    nFaces_.clear();
    fieldSelection_.clear();

    verbose_ = dict.getOrDefault("verbose", false);
    onExecute_ = dict.getOrDefault("sampleOnExecute", false);

    sampleFaceScheme_ =
        dict.getOrDefault<word>("sampleScheme", "cell");

    sampleNodeScheme_ =
        dict.getOrDefault<word>("interpolationScheme", "cellPoint");
    
    // MUIDomainName_ = dict.getOrDefault<word>("MUIDomainName", "OpenFOAM");
    MUIIfsID_ = dict.getOrDefault<int>("MUIInterfaceID", 0);
    writeIterfaceSurfaceFile_ = dict.getOrDefault<bool>("writeIterfaceSurfaceFile", false);
    const entry* eptr = dict.findEntry("MUICoupledSurfaces");

    // Surface writer type and format options
    const word writerType =
        (eptr ? dict.get<word>("surfaceFormat") : word::null);

    const dictionary formatOptions(dict.subOrEmptyDict("formatOptions"));

    // Store on registry?
    const bool dfltStore = dict.getOrDefault("store", false);

    if (eptr && eptr->isDict())
    {
        PtrList<sampledSurface> surfs(eptr->dict().size());

        actions_.resize(surfs.size(), ACTION_WRITE); // Default action
        writers_.resize(surfs.size());
        nFaces_.resize(surfs.size(), Zero);

        label surfi = 0;

        for (const entry& dEntry : eptr->dict())
        {
            if (!dEntry.isDict())
            {
                continue;
            }

            const dictionary& surfDict = dEntry.dict();

            autoPtr<sampledSurface> surf =
                sampledSurface::New
                (
                    dEntry.keyword(),
                    mesh_,
                    surfDict
                );

            if (!surf || !surf->enabled())
            {
                continue;
            }

            // Define the surface
            surfs.set(surfi, surf);

            // Define additional action(s)
            if (surfDict.getOrDefault("store", dfltStore))
            {
                actions_[surfi] |= ACTION_STORE;
            }

            // Define surface writer, but do NOT yet attach a surface
            writers_.set
            (
                surfi,
                newWriter(writerType, formatOptions, surfDict)
            );

            writers_[surfi].isPointData(surfs[surfi].isPointData());

            // Use outputDir/TIME/surface-name
            writers_[surfi].useTimeDir(true);
            writers_[surfi].verbose(verbose_);

            ++surfi;
        }

        surfs.resize(surfi);
        actions_.resize(surfi);
        writers_.resize(surfi);
        surfaces().transfer(surfs);
    }
    else if (eptr)
    {
        // This is slightly trickier.
        // We want access to the individual dictionaries used for construction

        DynamicList<dictionary> capture;

        PtrList<sampledSurface> input
        (
            eptr->stream(),
            sampledSurface::iNewCapture(mesh_, capture)
        );

        PtrList<sampledSurface> surfs(input.size());

        actions_.resize(surfs.size(), ACTION_WRITE); // Default action
        writers_.resize(surfs.size());
        nFaces_.resize(surfs.size(), Zero);

        label surfi = 0;

        forAll(input, inputi)
        {
            const dictionary& surfDict = capture[inputi];

            autoPtr<sampledSurface> surf = input.release(inputi);

            if (!surf || !surf->enabled())
            {
                continue;
            }

            // Define the surface
            surfs.set(surfi, surf);

            // Define additional action(s)
            if (surfDict.getOrDefault("store", dfltStore))
            {
                actions_[surfi] |= ACTION_STORE;
            }

            // Define surface writer, but do NOT yet attach a surface
            writers_.set
            (
                surfi,
                newWriter(writerType, formatOptions, surfDict)
            );

            writers_[surfi].isPointData(surfs[surfi].isPointData());

            // Use outputDir/TIME/surface-name
            writers_[surfi].useTimeDir(true);
            writers_[surfi].verbose(verbose_);

            ++surfi;
        }

        surfs.resize(surfi);
        actions_.resize(surfi);
        writers_.resize(surfi);
        surfaces().transfer(surfs);
    }


    const auto& surfs = surfaces();

    // Have some surfaces, so sort out which fields are needed and report

    if (surfs.size())
    {
        nFaces_.resize(surfs.size(), Zero);

        dict.readEntry("fields", fieldSelection_);
        fieldSelection_.uniq();

        forAll(*this, surfi)
        {
            const sampledSurface& s = (*this)[surfi];

            if (!surfi)
            {
                Info<< "Sampled surface:" << nl;
            }

            Info<< "    " << s.name() << " -> " << writers_[surfi].type();
            if (actions_[surfi] & ACTION_STORE)
            {
                Info<< ", store on registry ("
                    << IOobject::groupName(name(), s.name()) << ')';
            }
            Info<< nl;
            Info<< "        ";
            s.print(Info, 0);
            Info<< nl;
        }
        Info<< nl;
    }

    if (debug && Pstream::master())
    {
        Pout<< "sample fields:" << fieldSelection_ << nl
            << "sample surfaces:" << nl << '(' << nl;

        for (const sampledSurface& s : surfaces())
        {
            Pout<< "  " << s << nl;
        }
        Pout<< ')' << endl;
    }

    // Ensure all surfaces and merge information are expired
    expire(true);

    return true;
}



Foam::polySurface* Foam::sampledSurfacesCoupled::storeRegistrySurface
(
    const sampledSurface& s
)
{
    return s.storeRegistrySurface
    (
        storedObjects(),
        IOobject::groupName(name(), s.name())
    );
}


bool Foam::sampledSurfacesCoupled::removeRegistrySurface
(
    const sampledSurface& s
)
{
    return s.removeRegistrySurface
    (
        storedObjects(),
        IOobject::groupName(name(), s.name())
    );
}



Foam::IOobjectList Foam::sampledSurfacesCoupled::preCheckFields()
{
    wordList allFields;    // Just needed for warnings
    HashTable<wordHashSet> selected;

    IOobjectList objects(0);

    if (loadFromFiles_)
    {
        // Check files for a particular time
        objects = IOobjectList(obr_, obr_.time().timeName());

        allFields = objects.names();
        selected = objects.classes(fieldSelection_);
    }
    else
    {
        // Check currently available fields
        allFields = obr_.names();
        selected = obr_.classes(fieldSelection_);
    }

    // Parallel consistency (no-op in serial)
    Pstream::mapCombineAllGather(selected, HashSetOps::plusEqOp<word>());


    DynamicList<label> missed(fieldSelection_.size());

    // Detect missing fields
    forAll(fieldSelection_, i)
    {
        if (!ListOps::found(allFields, fieldSelection_[i]))
        {
            missed.append(i);
        }
    }

    if (missed.size())
    {
        WarningInFunction
            << nl
            << "Cannot find "
            << (loadFromFiles_ ? "field file" : "registered field")
            << " matching "
            << UIndirectList<wordRe>(fieldSelection_, missed) << endl;
    }


    // Currently only support volume and surface field types
    label nVolumeFields = 0;
    label nSurfaceFields = 0;

    forAllConstIters(selected, iter)
    {
        const word& clsName = iter.key();
        const label n = iter.val().size();

        if (fieldTypes::volume.found(clsName))
        {
            nVolumeFields += n;
        }
        else if (fieldTypes::surface.found(clsName))
        {
            nSurfaceFields += n;
        }
    }

    // Now propagate field counts (per surface)
    // - can update writer even when not writing without problem

    forAll(*this, surfi)
    {
        const sampledSurface& s = (*this)[surfi];
        surfaceWriter& outWriter = writers_[surfi];

        outWriter.nFields
        (
            nVolumeFields
          + (s.withSurfaceFields() ? nSurfaceFields : 0)
          +
            (
                // Face-id field, but not if the writer manages that itself
                !s.isPointData() && s.hasFaceIds() && !outWriter.usesFaceIds()
              ? 1 : 0
            )
        );
    }

    return objects;
}
