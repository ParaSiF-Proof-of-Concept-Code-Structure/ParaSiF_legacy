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
    pimpleFSIFoam.C

Group
    grpIncompressibleSolvers

Description
    Transient solver for incompressible, turbulent flow of Newtonian fluids
    on a moving mesh.

    \heading Solver details
    The solver uses the PIMPLE (merged PISO-SIMPLE) algorithm to solve the
    continuity equation:

        \f[
            \div \vec{U} = 0
        \f]

    and momentum equation:

        \f[
            \ddt{\vec{U}} + \div \left( \vec{U} \vec{U} \right) - \div \gvec{R}
          = - \grad p + \vec{S}_U
        \f]

    Where:
    \vartable
        \vec{U} | Velocity
        p       | Pressure
        \vec{R} | Stress tensor
        \vec{S}_U | Momentum source
    \endvartable

    Sub-models include:
    - turbulence modelling, i.e. laminar, RAS or LES
    - run-time selectable MRF and finite volume options, e.g. explicit porosity

    \heading Required fields
    \plaintable
        U       | Velocity [m/s]
        p       | Kinematic pressure, p/rho [m2/s2]
        \<turbulence fields\> | As required by user selection
    \endplaintable

Note
   The motion frequency of this solver can be influenced by the presence
   of "updateControl" and "updateInterval" in the dynamicMeshDict.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "dynamicFvMesh.H"
#include "singlePhaseTransportModel.H"
#include "turbulentTransportModel.H"
#include "pimpleControl.H"
#include "CorrectPhi.H"
#include "fvOptions.H"
#include "localEulerDdtScheme.H"
#include "fvcSmooth.H"
#include "pointMesh.H"   // Added by Omar
#include "pointPatchField.H"  // Added by Omar
#include "fixedValuePointPatchFields.H"

#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.
#include "coupledForces.H"
#include "mui.h"
#include "muiconfig.h"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::addNote
    (
        "Transient solver for incompressible, turbulent flow"
        " of Newtonian fluids on a moving mesh."
    );

    #include "postProcess.H"

    #include "addCheckCaseOptions.H"
    #include "setRootCaseLists.H"
    #include "createTime.H"
    #include "createDynamicFvMesh.H"
    #include "createDynamicFvMeshOri.H"
    #include "initContinuityErrs.H"
    #include "initFSILocalContinuityErrs.H"
    #include "createDyMControls.H"
    #include "createFields.H"
    #include "createUfIfPresent.H"
    #include "CourantNo.H"
    #include "setInitialDeltaT.H"

    #include "couplingVar.H"
    Foam::functionObjects::coupledForces* forces = nullptr;
    Info << "=====================================================" <<endl;
    if (couplingMode=="singlePoint"){
        Info << "Coupling Force mode is  singlePoint" <<endl;
        forces = new Foam::functionObjects::coupledForces(fsiForceName, runTime, fsiForceDict, true);
    }else if(couplingMode=="boundaryPatch"){
        Info << "Coupling Force mode is  boundaryPatch" <<endl;
    #include "pushForceInit.H"
    #include "fetchDisplacementInit.H"
    } else {
        FatalIOErrorIn("", fsiDict)
          << "The selected couplingMode entry is invalid. The valid options are singlePoint and boundaryPatch" << exit(FatalIOError);
    }
    Info << "=====================================================" <<endl;

    turbulence->validate();

    if (!LTS)
    {
        #include "CourantNo.H"
        #include "setInitialDeltaT.H"
    }

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    Info<< "\n {OpenFOMA} :Starting time loop\n" << endl;
    Info<< "ExecutionStartTime = " << runTime.elapsedCpuTime() << " s"
        << "  StartClockTime = " << runTime.elapsedClockTime() << " s"
        << nl << endl;
    runTime.updateCurrentIter(0);
    runTime.updateTimeSteps(0);
    while (runTime.run())
    {
        #include "readDyMControls.H"

        if (LTS)
        {
            #include "setRDeltaT.H"
        }
        else
        {
            #include "CourantNo.H"
            #include "setDeltaT.H"
        }

        ++runTime;

        runTime.updateTimeSteps();
        if (runTime.changeSubIter())
        {
            scalar tTemp=runTime.value();
            if (tTemp >= runTime.changeSubIterTime())
            {
                runTime.updateSubIterNum(runTime.newSubIterationNumber());
            }
        }

        runTime.updateSubIter(0);
        while (runTime.subIter()<runTime.subIterationNumber())
        {
            runTime.updateSubIter();
            runTime.updateCurrentIter();
            Info << "========================================================" << endl;
            Info << "{OpenFOMA} : Time = " << runTime.timeName() << ", and sub-Iteration = " 
                 << runTime.subIter() <<"/" << runTime.subIterationNumber() << endl;
            Info << "{OpenFOMA} : Time Steps = " << runTime.timeSteps() << endl;        
            Info << "{OpenFOMA} : Total current iteration = " << runTime.totalCurrentIter() << endl;
#ifdef USE_MUI
            if (couplingMode=="singlePoint"){
            forces->execute();
            }else if(couplingMode=="boundaryPatch"){
            #include "pushForce.H"
            #include "fetchDisplacement.H"
            }
#endif            
        // --- Pressure-velocity PIMPLE corrector loop
            while (pimple.loop())
            {
                if (pimple.firstIter() || moveMeshOuterCorrectors)
                {
                    // Do any mesh changes
                    mesh.controlledUpdate();

                    if (mesh.changing())
                    {
                        MRF.update();

                        if (correctPhi)
                        {
                            // Calculate absolute flux
                            // from the mapped surface velocity
                            phi = mesh.Sf() & Uf();

                            #include "correctPhi.H"

                            // Make the flux relative to the mesh motion
                            fvc::makeRelative(phi, U);
                        }

                        if (checkMeshCourantNo)
                        {
                            #include "meshCourantNo.H"
                        }
                    }
                }

                #include "UEqn.H"

                // --- Pressure corrector loop
                while (pimple.correct())
                {
                    #include "pEqn.H"
                }

                if (pimple.turbCorr())
                {
                    laminarTransport.correct();
                    turbulence->correct();
                }
            }
        }
        runTime.write();

        runTime.printExecutionTime(Info);
    }

    Info<< "End\n" << endl;
    #ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.
        //- If this is not a parallel run then need to finalise MPI (otherwise this is handled by MUI due to the use of split_by_app() function)
        if (!args.parRunControl().parRun())
        {
            MPI_Finalize();
        }
    #endif
    return 0;
}


// ************************************************************************* //
