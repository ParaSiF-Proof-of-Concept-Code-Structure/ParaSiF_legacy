/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2017 OpenFOAM Foundation
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

Application
    laplacianFoamMUI

Group
    grpBasicSolvers

Description
    Laplace equation solver for a scalar quantity with example MUI coupling capability.

    \heading Solver details
    The solver is applicable to, e.g. for thermal diffusion in a solid.  The
    equation is given by:

    \f[
        \ddt{T}  = \div \left( D_T \grad T \right)
    \f]

    Where:
    \vartable
        T     | Scalar field which is solved for, e.g. temperature
        D_T   | Diffusion coefficient
    \endvartable

    \heading Required fields
    \plaintable
        T     | Scalar field which is solved for, e.g. temperature
    \endplaintable

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "fvOptions.H"
#include "simpleControl.H"

// The main MUI header
// This will only work if the mpi-MUI wmake rule is included in the application Make options file
#include "mui.h"
// This is only needed if creating templated MUI interfaces
#include "muiConfigOF.H"

// Create DynamicLists (mui2dInterfaces / mui3dInterfaces / muiTemplatedInterfaces),
// these must be created after "mui.h" and before "createCouplingMUI.H", a list is only needed if
// interfaces are defined in the corresponding "couplingDict" dictionary file
// These should be declared as extern if the MUI interface is to be used beyond the top-level main() function call.
//extern DynamicList<mui::uniface<mui::config_2d>*> mui2dInterfaces;
//extern DynamicList<mui::uniface<mui::config_3d>*> mui3dInterfaces;
//extern DynamicList<mui::uniface<mui::config_of>*> muiTemplatedInterfaces;
DynamicList<mui::uniface<mui::config_2d>*> mui2dInterfaces;
DynamicList<mui::uniface<mui::config_3d>*> mui3dInterfaces;
DynamicList<mui::uniface<mui::config_of>*> muiTemplatedInterfaces;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::addNote
    (
        "Laplace equation solver for a scalar quantity."
    );

    #include "postProcess.H"

    #include "addCheckCaseOptions.H"
    #include "setRootCaseLists.H"
    #include "createTime.H"
    #include "createMesh.H"

    simpleControl simple(mesh);

    #include "createFields.H"

    // Reads coupling dictionary and populates DynamicLists (mui2dInterfaces / mui3dInterfaces / muiTemplatedInterfaces)
    // These are created by the corresponding header file "createCouplingData.H"
    #include "createCouplingMUI.H"

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    Info<< "\nCalculating temperature distribution\n" << endl;

    // Create 2D/3D/Templated MUI spatial sampler objects to use during fetch operations
    // Alternative sampler types can be found or added in the /spatial_samplers folder of the MUI library
    mui::sampler_exact<mui::config_2d> muiSpatialSampler2d;
    mui::sampler_exact<mui::config_3d> muiSpatialSampler3d;
    mui::sampler_exact<mui::config_of> muiSpatialSamplerT;

    // Create 2D/3D/Templated MUI temporal sampler objects to use during fetch operations
    // Alternative temporal (chrono) samplers can be found or added in the /chrono_samplers folder of the MUI library
    mui::temporal_sampler_exact<mui::config_3d> muiChronoSampler3d;
    mui::temporal_sampler_exact<mui::config_2d> muiChronoSampler2d;
    mui::temporal_sampler_exact<mui::config_of> muiChronoSamplerT;

    mui::config_2d::REAL muiPushValue2d = 1; // Value to push to 2D interfaces
    mui::config_3d::REAL muiPushValue3d = 2; // Value to push to 3D interfaces
    mui::config_of::REAL muiPushValueT = 3; // Value to push to templated interfaces

    // Storage for received values from 2D/3D/Templated interfaces
    List<mui::config_2d::REAL> muiFetchValues2d(mui2dInterfaces.size());
    List<mui::config_3d::REAL> muiFetchValues3d(mui3dInterfaces.size());
    List<mui::config_of::REAL> muiFetchValuesT(muiTemplatedInterfaces.size());

    // Local iteration count to use in MUI push and fetch functions.
    //
    // Note - The correct value to use as a time stamp depends on whether "iterationCoupling" is enabled in
    // the "couplingDict" dictionary. If disabled then a direct scalar time value should be used instead.
    // Iteration based coupling is always preferable wherever possible as it is integer based and therefore
    // ensures accuracy in any MUI blocking functions.
    //
    // If a templated MUI interface is being used then ensure that "time_type" in the corresponding struct is
    // correct, for standard 2D/3D interfaces then "time_type" is assumed to be equal to the value of an MUI REAL
    // data type (typically double) - integer based iterative time stamping is still preferable in this case to avoid
    // any rounding problems but a small extra MPI communication overhead will be encountered as the supplied integer
    // stamps will be cast to REAL, it is recommended to us a templated type for complete control.
    int iterationCount = 0;

    while (simple.loop())
    {
        Info<< "Time = " << runTime.timeName() << nl << endl;

        // For all defined 2D interfaces, push a value and commit (send) to the corresponding domain
        for(label i = 0; i < mui2dInterfaces.size(); i++)
        {
            mui::point<mui::config_2d::REAL, mui::config_2d::D> muiPush2d_1(0, 0);
            mui::point<mui::config_2d::REAL, mui::config_2d::D> muiPush2d_2(1, 1);

            // 2 points are pushed simply to avoid an MUI warning about dimensionality
            mui2dInterfaces[i]->push("value2d", muiPush2d_1, muiPushValue2d);
            mui2dInterfaces[i]->push("value2d", muiPush2d_2, muiPushValue2d);

            // Commit (transmit by MPI) the points
            mui2dInterfaces[i]->commit(iterationCount);

            Info << "MUI interface " << mui2dInterfaces[i]->uri_host() << "/" << mui2dInterfaces[i]->uri_path()
                 << " value committed: " << muiPushValue2d << " at Iteration = " << iterationCount << endl;
        }

        // For all defined 3D interfaces, push a value and commit (send) to the corresponding domain
        for(label i = 0; i < mui3dInterfaces.size(); i++)
        {
            mui::point<mui::config_3d::REAL, mui::config_3d::D> muiPush3d_1(0, 0, 0);
            mui::point<mui::config_3d::REAL, mui::config_3d::D> muiPush3d_2(1, 1, 1);

            // 2 points are pushed simply to avoid an MUI warning about dimensionality
            mui3dInterfaces[i]->push("value3d", muiPush3d_1, muiPushValue3d);
            mui3dInterfaces[i]->push("value3d", muiPush3d_2, muiPushValue3d);

            // Commit (transmit by MPI) the points
            mui3dInterfaces[i]->commit(iterationCount);

            Info << "MUI interface " << mui3dInterfaces[i]->uri_host() << "/" << mui3dInterfaces[i]->uri_path()
                 << " value committed: " << muiPushValue3d << " at Iteration = " << iterationCount << endl;
        }

        // For all defined templated interfaces, push a value and commit (send) to the corresponding domain
        for(label i = 0; i < muiTemplatedInterfaces.size(); i++)
        {
            mui::point<mui::config_of::REAL, mui::config_of::D> muiPushT_1(0, 0, 0);
            mui::point<mui::config_of::REAL, mui::config_of::D> muiPushT_2(1, 1, 1);

            // 2 points are pushed simply to avoid an MUI warning about dimensionality
            muiTemplatedInterfaces[i]->push("valueT", muiPushT_1, muiPushValueT);
            muiTemplatedInterfaces[i]->push("valueT", muiPushT_2, muiPushValueT);

            // Commit (transmit by MPI) the points
            muiTemplatedInterfaces[i]->commit(iterationCount);

            Info << "MUI interface " << muiTemplatedInterfaces[i]->uri_host() << "/" << muiTemplatedInterfaces[i]->uri_path()
                 << " value committed: " << muiPushValueT << " at Iteration = " << iterationCount << endl;
        }

        while (simple.correctNonOrthogonal())
        {
            fvScalarMatrix TEqn
            (
                fvm::ddt(T) - fvm::laplacian(DT, T)
             ==
                fvOptions(T)
            );

            fvOptions.constrain(TEqn);
            TEqn.solve();
            fvOptions.correct(T);
        }

        #include "write.H"

        // For all defined 2D interfaces, fetch a value from the corresponding domain, this is a blocking operation
        for(label i = 0; i < mui2dInterfaces.size(); i++)
        {
            mui::point<mui::config_2d::REAL, mui::config_2d::D> muiFetch2d(0, 0);

            muiFetchValues2d[i] = mui2dInterfaces[i]->fetch("value2d", muiFetch2d, iterationCount,
                                                            muiSpatialSampler2d, muiChronoSampler2d);

            Info << "MUI interface " << mui2dInterfaces[i]->uri_host() << "/" << mui2dInterfaces[i]->uri_path()
                 << " value fetched: " << muiFetchValues2d[i] << " at Iteration = " << iterationCount << endl;
        }

        // For all defined 3D interfaces, fetch a value from the corresponding domain, this is a blocking operation
        for(label i = 0; i < mui3dInterfaces.size(); i++)
        {
            mui::point<mui::config_3d::REAL, mui::config_3d::D> muiFetch3d(0, 0, 0);

            muiFetchValues3d[i] = mui3dInterfaces[i]->fetch("value3d", muiFetch3d, iterationCount,
                                                            muiSpatialSampler3d, muiChronoSampler3d);

            Info << "MUI interface " << mui3dInterfaces[i]->uri_host() << "/" << mui3dInterfaces[i]->uri_path()
                 << " value fetched: " << muiFetchValues3d[i] << " at Iteration = " << iterationCount << endl;
        }

        // For all defined templated interfaces, fetch a value from the corresponding domain, this is a blocking operation
        for(label i = 0; i < muiTemplatedInterfaces.size(); i++)
        {
            mui::point<mui::config_of::REAL, mui::config_of::D> muiFetchT(0, 0, 0);

            muiFetchValuesT[i] = muiTemplatedInterfaces[i]->fetch("valueT", muiFetchT, iterationCount,
                                                            muiSpatialSamplerT, muiChronoSamplerT);

            Info << "MUI interface " << muiTemplatedInterfaces[i]->uri_host() << "/" << muiTemplatedInterfaces[i]->uri_path()
                 << " value fetched: " << muiFetchValuesT[i] << " at Iteration = " << iterationCount << endl;
        }

        runTime.printExecutionTime(Info);

        iterationCount++;
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
