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

Application
    interFoam

Group
    grpMultiphaseSolvers

Description
    Solver for two incompressible, isothermal immiscible fluids using a VOF
    (volume of fluid) phase-fraction based interface capturing approach,
    with optional mesh motion and mesh topology changes including adaptive
    re-meshing.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "dynamicFvMesh.H"
#include "CMULES.H"
#include "EulerDdtScheme.H"
#include "localEulerDdtScheme.H"
#include "CrankNicolsonDdtScheme.H"
#include "subCycle.H"
#include "immiscibleIncompressibleTwoPhaseMixture.H"
#include "incompressibleInterPhaseTransportModel.H"
#include "turbulentTransportModel.H"
#include "pimpleControl.H"
#include "fvOptions.H"
#include "CorrectPhi.H"
#include "fvcSmooth.H"
#include "fixedValuePointPatchFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.
#include "coupledForces.H"
#include "mui.h"
#include "muiconfig.h"
#endif



int main(int argc, char *argv[])
{
    argList::addNote
    (
        "Solver for two incompressible, isothermal immiscible fluids"
        " using VOF phase-fraction based interface capturing.\n"
        "With optional mesh motion and mesh topology changes including"
        " adaptive re-meshing."
    );
    #include "postProcess.H"

    #include "addCheckCaseOptions.H"
    #include "setRootCaseLists.H"
    #include "createTime.H"
    #include "createDynamicFvMesh.H"
    #include "initContinuityErrs.H"
    #include "createDyMControls.H"
    #include "createFields.H"
    #include "createAlphaFluxes.H"
    #include "initCorrectPhi.H"
    #include "createUfIfPresent.H"

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

            

    if (!LTS)
    {
        #include "CourantNo.H"
        #include "setInitialDeltaT.H"
    }

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
    Info<< "\n {OpenFOMA} :Starting time loop\n" << endl;
    // #ifdef USE_MUI
    // Reads coupling dictionary and populates DynamicLists (mui2dInterfaces / mui3dInterfaces / muiTemplatedInterfaces)
    // These are created by the corresponding header file "createCouplingData.H"
    // #include "createCouplingMUI.H" 
    // #endif
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
            #include "alphaCourantNo.H"
            #include "setDeltaT.H"
        }
        ++runTime;

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
            Info << "{OpenFOMA} : Time = " << runTime.timeName() << ", and sub-Iteration = " 
                 << runTime.subIter() <<"/" << runTime.subIterationNumber() << nl << endl;
            Info << "{OpenFOMA} : Time Steps = " << runTime.timeSteps() << nl << endl;        
            Info << "{OpenFOMA} : Total current iteration = " << runTime.totalCurrentIter() << nl << endl;
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
                    mesh.update();

                    if (mesh.changing())
                    {
                        // Do not apply previous time-step mesh compression flux
                        // if the mesh topology changed
                        if (mesh.topoChanging())
                        {
                            talphaPhi1Corr0.clear();
                        }

                        gh = (g & mesh.C()) - ghRef;
                        ghf = (g & mesh.Cf()) - ghRef;

                        MRF.update();

                        if (correctPhi)
                        {
                            // Calculate absolute flux
                            // from the mapped surface velocity
                            phi = mesh.Sf() & Uf();

                            #include "correctPhi.H"

                            // Make the flux relative to the mesh motion
                            fvc::makeRelative(phi, U);

                            mixture.correct();
                        }

                        if (checkMeshCourantNo)
                        {
                            #include "meshCourantNo.H"
                        }
                    }
                }

                #include "alphaControls.H"
                #include "alphaEqnSubCycle.H"

                mixture.correct();

                if (pimple.frozenFlow())
                {
                    continue;
                }

                #include "UEqn.H"

                // --- Pressure corrector loop
                while (pimple.correct())
                {
                    #include "pEqn.H"
                }

                if (pimple.turbCorr())
                {
                    turbulence->correct();
                }
            }
        }
        runTime.write();

        runTime.printExecutionTime(Info);
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
