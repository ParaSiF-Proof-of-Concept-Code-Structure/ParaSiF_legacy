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

Application
    interFSIFoam

Description
    Solver for 2 incompressible, isothermal immiscible fluids using a VOF
    (volume of fluid) phase-fraction based interface capturing approach,
    with optional mesh motion and mesh topology changes including adaptive
    re-meshing.

\*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*\
    Parallel Partitioned Multi-physical Simulation Framework (parMupSiF)

    Copyright (C) 2021 Engineering and Environment Group, Scientific 
    Computing Department, Science and Technology Facilities Council, 
    UK Research and Innovation. All rights reserved.

    This code is licensed under the GNU General Public License version 3

    ** GNU General Public License, version 3 **

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
    *********************************************************************
    
    @file interFSIFoam.C
    
    @author W. Liu
    
    @brief This is a part of the Parallel Partitioned Multi-physical Simu-
    lation Framework provides FEniCS v2019.1.0 <-> MUI v1.2 <-> OpenFOAM v6 
    two-way coupling.

    Incompressible Navier-Stokes equations with two fluid by VOF for fluid domain 
	in OpenFOAM and Structure dynamics equations for structure domain in FEniCS.

    It is a variation of the OpenFOAM built-in solver -- interFoam
    Located in the src/CFD/OpenFOAM/V6/applications/solvers/interFSIFoam sub-folder
\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "dynamicFvMesh.H"
#include "CMULES.H"
#include "EulerDdtScheme.H"
#include "localEulerDdtScheme.H"
#include "CrankNicolsonDdtScheme.H"
#include "subCycle.H"
#include "immiscibleIncompressibleTwoPhaseMixture.H"
#include "turbulentTransportModel.H"
#include "pimpleControl.H"
#include "fvOptions.H"
#include "CorrectPhi.H"
#include "fvcSmooth.H"
#include "fixedValuePointPatchFields.H"

#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.
    #include "mui.h"
    #include "aitken_inl.H"
    #include "fixedRelaxation_inl.H"
    #include "iqnils_inl.H"
#endif


#define pi 3.141592653589793238

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{

const dimensionedScalar gunits("gunits", dimensionSet(0,1,-2,0,0,0,0), 9.81);

    #include "postProcess.H"

    #include "setRootCaseLists.H"
    #include "createTime.H"
    #include "createDynamicFvMesh.H"
    #include "createDynamicFvMeshOri.H"
    #include "initContinuityErrs.H"
    #include "initFSILocalContinuityErrs.H" 
    #include "createDyMControls.H"
    #include "createFields.H"
    #include "createAlphaFluxes.H"
    #include "createDummyFields.H"
    #include "initCorrectPhi.H"
    #include "createUfIfPresent.H"

    turbulence->validate();

    if (!LTS)
    {
        #include "CourantNo.H"
        #include "setInitialDeltaT.H"
    }


    #include "pushForceInit.H"
    #include "fetchDisplacementInit.H"

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
    Info<< "\nStarting time loop\n" << endl;

    Info<< "ExecutionStartTime = " << runTime.elapsedCpuTime() << " s"
        << "  StartClockTime = " << runTime.elapsedClockTime() << " s"
        << nl << endl;

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

        runTime++;
        timeSteps++;
/* 
        if (runTime.value() <= 0.32)
        {
            g=gunits*Foam::sin(0.0698132*1.3*runTime.value())*vector(1,0,0)-gunits*Foam::cos(0.0698132*1.3*runTime.value())*vector(0,1,0);
        } else if ((runTime.value() > 0.32) && (runTime.value() < 0.47))
        {
            g=gunits*Foam::sin(0.0698132*13*runTime.value()*runTime.value()*runTime.value())*vector(1,0,0)-gunits*Foam::cos(0.0698132*13*runTime.value()*runTime.value()*runTime.value())*vector(0,1,0);
        }else
        {
            g=gunits*Foam::sin(0.0698132*Foam::sin((runTime.value()-0.38)*pi*2*0.61))*vector(1,0,0)-gunits*Foam::cos(0.0698132*Foam::sin((runTime.value()-0.38)*pi*2*0.61))*vector(0,1,0);
        } */
        
        g=(gunits)*Foam::sin(0.0698132*Foam::sin(runTime.value()*pi*2*0.83))*vector(1,0,0)-gunits*Foam::cos(0.0698132*Foam::sin(runTime.value()*pi*2*0.83))*vector(0,1,0);
 
        
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

 //           #include "resetFields.H"

            Info<< "sub-Iteration = " << subIter << nl << endl;

            totalCurrentIter++;

            Info << "total current iteration = " << totalCurrentIter << nl << endl;

 //           #include "rigidMeshMotion.H"

           #include "pushForce.H"
           #include "fetchDisplacement.H"

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
            //if (subIter == 1)
            //{
				#include "alphaControls.H"
				#include "alphaEqnSubCycle.H"

				mixture.correct();
            //}
            
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

        Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
            << "  ClockTime = " << runTime.elapsedClockTime() << " s"
            << nl << endl;
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
