/*---------------------------------------------------------------------------*\
    Parallel Partitioned Multi-physical Simulation Framework (ParaSiF)

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

    @file pushForceInit.H

    @author W. Liu

    @brief This is a part of the Parallel Partitioned Multi-physical Simu-
    lation Framework provides FEniCS v2019.1.0 <-> MUI v1.0 <-> OpenFOAM v6
    two-way coupling.

    Incompressible Navier-Stokes equations with two fluid by VOF for fluid domain
	  in OpenFOAM and Structure dynamics equations for structure domain in FEniCS.

    It initialises the function of push forces and/or tractions of the structure boundary
    to the structure domain before the time loop of the fluid solver
    Located in the src/CFD/OpenFOAM/V6/applications/solvers/interFSIFoam sub-folder
\*---------------------------------------------------------------------------*/

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    forAll(interfaceNames, iN)
    {

        List<int> localPatchIDs;

        forAll(patchNames[iN], pN)
        {
            int localPatchID = mesh.boundaryMesh().findPatchID(patchNames[iN][pN]);

            Info << "{OpenFOMA} : PatchName = " << patchNames[iN][pN] << " has ID = " << localPatchID << endl;

            // Check if patch exists
            if(localPatchID == -1)
            {
                FatalError << "{OpenFOMA} : PatchName " << patchNames[iN][pN] << " not found!" << exit(FatalError);
            }

            localPatchIDs.append(localPatchID);

        }

        patchIDs.append(localPatchIDs);
        List<int> local6DoFPatchIDs;

        forAll(patch6DoFNames[iN], pN)
        {
            int local6DoFPatchID = mesh.boundaryMesh().findPatchID(patch6DoFNames[iN][pN]);

            Info << "{OpenFOMA} : Patch6DoFName = " << patch6DoFNames[iN][pN] << " has ID = " << local6DoFPatchID << endl;

            // Check if patch exists
            if(local6DoFPatchID == -1)
            {
                FatalError << "Patch6DoFName " << patch6DoFNames[iN][pN] << " not found!" << exit(FatalError);
            }

            local6DoFPatchIDs.append(local6DoFPatchID);

        }

        patch6DoFIDs.append(local6DoFPatchIDs);

    }


    forAll(interfaceNames, iN)
    {

        scalar totalFaceNLocal;
        List<vectorField> oriCoordPushPointsVectorValsListLocal;

        forAll(patchIDs[iN], pI)
        {
            if(mesh.boundaryMesh()[patchIDs[iN][pI]].size() != 0)
            {
                rankHasPatchPointsForPush = true;
            }

            totalFaceNLocal += mesh.boundaryMesh()[patchIDs[iN][pI]].size();
            reduce(totalFaceNLocal, sumOp<scalar>());
            vectorField oriCoordPushPointsVectorValsLocal(mesh.Cf().boundaryField()[patchIDs[iN][pI]].size());
            oriCoordPushPointsVectorValsListLocal.append(oriCoordPushPointsVectorValsLocal);
        }
        totalFaceNList.append(totalFaceNLocal);
        oriCoordPushPointsVectorValsList.append(oriCoordPushPointsVectorValsListLocal);
    }


    // Loop over faces to Store the original face center coordinates
    forAll(interfaceNames, iN)
    {
        forAll(patchIDs[iN], pI)
        {
            // ToDo: use map function to link face ID with coordinates
            forAll(oriCoordPushPointsVectorValsList[iN][pI], faceI)
            {
               oriCoordPushPointsVectorValsList[iN][pI][faceI].component(vector::X) =
                    mesh.Cf().boundaryField()[patchIDs[iN][pI]][faceI].component(vector::X);
                oriCoordPushPointsVectorValsList[iN][pI][faceI].component(vector::Y) =
                    mesh.Cf().boundaryField()[patchIDs[iN][pI]][faceI].component(vector::Y);
                oriCoordPushPointsVectorValsList[iN][pI][faceI].component(vector::Z) =
                    mesh.Cf().boundaryField()[patchIDs[iN][pI]][faceI].component(vector::Z);
            }
        }
    }

#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.
    if (args.cplRunControl().cplRun())
    {

        if (rankHasPatchPointsForPush)
        {

            forAll(interfaceNames, iN)
            {

                forAll(patchIDs[iN], pI)
                {
                    forAll(oriCoordPushPointsVectorValsList[iN][pI], faceI)
                    {
                        sendXMin =
                            ( (oriCoordPushPointsVectorValsList[iN][pI][faceI].component(vector::X)) < sendXMin) ? (oriCoordPushPointsVectorValsList[iN][pI][faceI].component(vector::X)) : sendXMin;
                        sendYMin =
                            ((oriCoordPushPointsVectorValsList[iN][pI][faceI].component(vector::Y)) < sendYMin) ? (oriCoordPushPointsVectorValsList[iN][pI][faceI].component(vector::Y)) : sendYMin;
                        sendZMin =
                            ((oriCoordPushPointsVectorValsList[iN][pI][faceI].component(vector::Z)) < sendZMin) ? (oriCoordPushPointsVectorValsList[iN][pI][faceI].component(vector::Z)) : sendZMin;

                        sendXMax =
                            ((oriCoordPushPointsVectorValsList[iN][pI][faceI].component(vector::X)) > sendXMax) ? (oriCoordPushPointsVectorValsList[iN][pI][faceI].component(vector::X)) : sendXMax;
                        sendYMax =
                            ((oriCoordPushPointsVectorValsList[iN][pI][faceI].component(vector::Y)) > sendYMax) ? (oriCoordPushPointsVectorValsList[iN][pI][faceI].component(vector::Y)) : sendYMax;
                        sendZMax =
                            ((oriCoordPushPointsVectorValsList[iN][pI][faceI].component(vector::Z)) > sendZMax) ? (oriCoordPushPointsVectorValsList[iN][pI][faceI].component(vector::Z)) : sendZMax;
                    }
                }

            }
        } else
        {

            const pointField& meshPoints = mesh.points();

            forAll(meshPoints, pts)
            {
                sendXMin =
                    (meshPoints[pts][0] < sendXMin) ? meshPoints[pts][0] : sendXMin;
                sendYMin =
                    (meshPoints[pts][1] < sendYMin) ? meshPoints[pts][1] : sendYMin;
                sendZMin =
                    (meshPoints[pts][2] < sendZMin) ? meshPoints[pts][2] : sendZMin;

                sendXMax =
                    (meshPoints[pts][0] > sendXMax) ? meshPoints[pts][0] : sendXMax;
                sendYMax =
                    (meshPoints[pts][1] > sendYMax) ? meshPoints[pts][1] : sendYMax;
                sendZMax =
                    (meshPoints[pts][2] > sendZMax) ? meshPoints[pts][2] : sendZMax;
            }
        }

    }

#endif


#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.

    if (args.cplRunControl().cplRun())
    {

        Pout << "sendXMin = " << sendXMin << endl;
        Pout << "sendYMin = " << sendYMin << endl;
        Pout << "sendZMin = " << sendZMin << endl;

        Pout << "sendXMax = " << sendXMax << endl;
        Pout << "sendYMax = " << sendYMax << endl;
        Pout << "sendZMax = " << sendZMax << endl;

        // Annouce send span
        mui::geometry::box<mui::mui_config> send_region( {sendXMin, sendYMin, sendZMin}, {sendXMax, sendYMax, sendZMax} );
        // Set the maximum time step size
        int totalTimeSteps = ceil((runTime.endTime().value() - runTime.startTime().value())/runTime.deltaT().value());

        forAll(interfaceNames, iN)
        {
            ifs[iN]->announce_send_span( 0, totalTimeSteps, send_region );
        }

    }

#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// *************************  FILE END  ************************************ //
