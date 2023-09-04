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
// #include "coupledForces.H"
#include "couplingVarExternal.H"
#include "coupledForces.H"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

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
    
    

    if (!LTS)
    {
        #include "CourantNo.H"
        #include "setInitialDeltaT.H"
    }

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
    Info<< "\nStarting time loop\n" << endl;
    totalCurrentIter = 0;
    Foam::functionObjects::coupledForces forces(fsiForceName,runTime,fsiForceDict,true);

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

        timeSteps++;
        
        Info<< "Time = " << runTime.timeName() << nl << endl;
        Info<< "Time Steps = " << timeSteps << nl << endl;

        if (changeSubIter)
        {
            scalar tTemp=runTime.value();

            if (tTemp >= changeSubIterTime)
            {
                subIterationNum = subIterationNumNew;
            }
        }
        for(int subIter = 1; subIter <= subIterationNum; ++subIter)
        {

            Info<< "Sub-Iteration number = " << subIter << nl << endl;

            totalCurrentIter++;

            Info << "Total current iteration = " << totalCurrentIter << nl << endl;
            forces.execute();
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
            
            
            // IOdictionary fsiDict
            // (
            //     IOobject
            //     (
            //         fsiFile,    // dictionary name
            //         runTime.system(),     // dict is found in "constant"
            //         mesh,                   // registry for the dict
            //         IOobject::MUST_READ,    // must exist, otherwise failure
            //         IOobject::NO_WRITE      // dict is only read by the solver
            //     )
            // );

            

            
            
        }
        runTime.write();
        
        runTime.printExecutionTime(Info);
        
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
