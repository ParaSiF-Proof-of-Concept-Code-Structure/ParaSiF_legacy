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


    // Define the name of MUI interfaceName
    List<word> interfaceNames;
    List<List<word>> patchNames;
    List<List<word>> patch6DoFNames;
    double forgetTime = 1.0;

    word sendNameX;
    word sendNameY;
    word sendNameZ;

    word fetchNameX;
    word fetchNameY;
    word fetchNameZ;

    double rSampler;

    //- Tight/Loose coupling method
    word cplMethod;

    //- Initial under relaxation factor of the coupling
    double initUndRelxCpl;

    //- Whether to push force or traction
    bool pushForce;

    //- Whether apply the fetched X value or not(i.e. one-way coupling)
    bool applyFetchX;
    //- Whether apply the fetched Y value or not(i.e. one-way coupling)
    bool applyFetchY;
    //- Whether apply the fetched Z value or not(i.e. one-way coupling)
    bool applyFetchZ;

    //- Number of time steps that will not push forces/tractions through MUI.
    double pushDelay;

    //- Whether it is a 2D case, i.e. whether to have a 2D force correction before push?
    //- A 2D case is the case that the front and back boundaries are empty boundary condition.
    //- There usually has one cell between the front and back boundaries.
    bool twoDimCorrection;

    //- The cell height of the only cell between the two empty boundaries [m].
    //- Valid only when twoDimCorrection is true.
    double twoDimCellHeight;

    scalar rigidMotionFrqX_;
    scalar rigidMotionFrqY_;
    scalar rigidMotionFrqZ_;

    scalar rigidMotionAmplitudeX_;
    scalar rigidMotionAmplitudeY_;
    scalar rigidMotionAmplitudeZ_;

    scalar rotation2DFrq_ = 0.0;
    scalar rotation2DAmplitude_ = 0.0;
    scalar rotation2DOriginX_ = 0.0;
    scalar rotation2DOriginY_ = 0.0;
    scalar rotation2DOriginZ_ = 0.0;

    string fsiFile("fsiDict");

    IOdictionary fsiDict
    (
        IOobject
        (
            fsiFile,
            runTime.system(),
            mesh,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    );

    //Read fsi dictionary file if it exists
    if (fsiDict.headerOk())
    {
        const PtrList<entry> interfaceConfigurations
        (
            fsiDict.lookup("interfaceLists")
        );

        //Iterate through interface lists
        forAll(interfaceConfigurations, cC)
        {
            const entry& interfaceI = interfaceConfigurations[cC];
            const dictionary& interfaceIDict = interfaceI.dict();

            interfaceNames.append(static_cast<word>(interfaceIDict.dictName()));

            List<word> patchNamesLocal(interfaceIDict.lookup("PatchNames"));

            patchNames.append(patchNamesLocal);

            List<word> patch6DoFNamesLocal(interfaceIDict.lookup("Patch6DoFNames"));

            patch6DoFNames.append(patch6DoFNamesLocal);

        }

        word cplFSIMethod;
        if (!(fsiDict.readIfPresent("cplFSIMethod", cplFSIMethod)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing cplFSIMethod entry" << exit(FatalIOError);
        }
        cplMethod = cplFSIMethod;


        double forgetFSITime;
        if (!(fsiDict.readIfPresent("forgetFSITime", forgetFSITime)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing forgetFSITime entry" << exit(FatalIOError);
        }
        forgetTime = forgetFSITime;

        word sendFSINameX;
        word sendFSINameY;
        word sendFSINameZ;
        word fetchFSINameX;
        word fetchFSINameY;
        word fetchFSINameZ;

        if (!(fsiDict.readIfPresent("sendFSINameX", sendFSINameX)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing sendFSINameX entry" << exit(FatalIOError);
        }
        if (!(fsiDict.readIfPresent("sendFSINameY", sendFSINameY)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing sendFSINameY entry" << exit(FatalIOError);
        }
        if (!(fsiDict.readIfPresent("sendFSINameZ", sendFSINameZ)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing sendFSINameZ entry" << exit(FatalIOError);
        }
        if (!(fsiDict.readIfPresent("fetchFSINameX", fetchFSINameX)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing fetchFSINameX entry" << exit(FatalIOError);
        }
        if (!(fsiDict.readIfPresent("fetchFSINameY", fetchFSINameY)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing fetchFSINameY entry" << exit(FatalIOError);
        }
        if (!(fsiDict.readIfPresent("fetchFSINameZ", fetchFSINameZ)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing fetchFSINameZ entry" << exit(FatalIOError);
        }
        sendNameX = sendFSINameX;
        sendNameY = sendFSINameY;
        sendNameZ = sendFSINameZ;
        fetchNameX = fetchFSINameX;
        fetchNameY = fetchFSINameY;
        fetchNameZ = fetchFSINameZ;

        if (!(fsiDict.readIfPresent("pushFSIForce", pushForce)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing pushFSIForce entry" << exit(FatalIOError);
        }

        if (!(fsiDict.readIfPresent("applyFSIFetchX", applyFetchX)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing applyFSIFetchX entry" << exit(FatalIOError);
        }

        if (!(fsiDict.readIfPresent("applyFSIFetchY", applyFetchY)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing applyFSIFetchY entry" << exit(FatalIOError);
        }

        if (!(fsiDict.readIfPresent("applyFSIFetchZ", applyFetchZ)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing applyFSIFetchZ entry" << exit(FatalIOError);
        }

        if (!(fsiDict.readIfPresent("searchAreaSampler", rSampler)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing searchAreaSampler entry" << exit(FatalIOError);
        }

        if (cplMethod != "Loose")
        {
            if (!(fsiDict.readIfPresent("initFSIUndRelxCpl", initUndRelxCpl)))
            {
                FatalIOErrorIn("", fsiDict)
                               << "Missing initFSIUndRelxCpl entry" << exit(FatalIOError);
            }
        }


        if (!(fsiDict.readIfPresent("pushFSIDelay", pushDelay)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing pushFSIDelay entry" << exit(FatalIOError);
        }

        if (!(fsiDict.readIfPresent("twoDimFSICorrection", twoDimCorrection)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing twoDimFSICorrection entry" << exit(FatalIOError);
        }

        if (twoDimCorrection)
        {
            if (!(fsiDict.readIfPresent("twoDimFSICellHeight", twoDimCellHeight)))
            {
                FatalIOErrorIn("", fsiDict)
                               << "Missing twoDimFSICellHeight entry" << exit(FatalIOError);
            }
        }

        if (!(fsiDict.readIfPresent("rigidMotionFrqX", rigidMotionFrqX_)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing rigidMotionFrqX entry" << exit(FatalIOError);
        }

        if (!(fsiDict.readIfPresent("rigidMotionFrqY", rigidMotionFrqY_)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing rigidMotionFrqY entry" << exit(FatalIOError);
        }

        if (!(fsiDict.readIfPresent("rigidMotionFrqZ", rigidMotionFrqZ_)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing rigidMotionFrqZ entry" << exit(FatalIOError);
        }

        if (!(fsiDict.readIfPresent("rigidMotionAmplitudeX", rigidMotionAmplitudeX_)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing rigidMotionAmplitudeX entry" << exit(FatalIOError);
        }

        if (!(fsiDict.readIfPresent("rigidMotionAmplitudeY", rigidMotionAmplitudeY_)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing rigidMotionAmplitudeY entry" << exit(FatalIOError);
        }

        if (!(fsiDict.readIfPresent("rigidMotionAmplitudeZ", rigidMotionAmplitudeZ_)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing rigidMotionAmplitudeZ entry" << exit(FatalIOError);
        }
        if (!(fsiDict.readIfPresent("rotation2DFrq", rotation2DFrq_)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing rotation2DFrq entry" << exit(FatalIOError);
        }

        if (!(fsiDict.readIfPresent("rotation2DAmplitude", rotation2DAmplitude_)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing rotation2DAmplitude entry" << exit(FatalIOError);
        }

        if (!(fsiDict.readIfPresent("rotation2DOriginX", rotation2DOriginX_)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing rotation2DOriginX entry" << exit(FatalIOError);
        }

        if (!(fsiDict.readIfPresent("rotation2DOriginY", rotation2DOriginY_)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing rotation2DOriginY entry" << exit(FatalIOError);
        }

        if (!(fsiDict.readIfPresent("rotation2DOriginZ", rotation2DOriginZ_)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing rotation2DOriginZ entry" << exit(FatalIOError);
        }
    }
    else
    {
            FatalIOErrorIn("", fsiDict)
                           << "Missing fsiDict" << exit(FatalIOError);
    }

    List<List<int>> patchIDs;
    List<List<int>> patch6DoFIDs;
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

    volVectorField * faceForces = new volVectorField
    (
        IOobject
        (
            "faceForces",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensionedVector
        (
            "faceForces",
            dimensionSet(1,1,-2,0,0,0,0),
            vector::zero
        )
    );

    volVectorField * faceForces2D = new volVectorField
    (
        IOobject
        (
            "faceForces2D",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensionedVector
        (
            "faceForces2D",
            dimensionSet(1,1,-2,0,0,0,0),
            vector::zero
        )
    );

    volVectorField * faceTraction = new volVectorField
    (
        IOobject
        (
            "faceTraction",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensionedVector
        (
            "faceTraction",
            dimensionSet(1,-1,-2,0,0,0,0),
            vector::zero
        )
    );

    IOdictionary transportProperties
    (
        IOobject
        (
            "transportProperties",
            runTime.constant(),
            mesh,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    );

    bool rankHasPatchPointsForPush = false;

    List<scalar> totalFaceNList;
    // Declare a vectorField to store the original point coordinates.
    List<List<vectorField>> oriCoordPushPointsVectorValsList;

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

#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.

    // Define the name of MUI interfaces
    std::vector<std::string> interfaces;

    forAll(interfaceNames, iN)
    {
        interfaces.emplace_back(interfaceNames[iN]);
    }
    // Declare MUI objects using MUI configure file

    auto& ifs = runTime.mui_ifs;


    scalar sendXMin = VGREAT;
    scalar sendYMin = VGREAT;
    scalar sendZMin = VGREAT;

    scalar sendXMax = -VGREAT;
    scalar sendYMax = -VGREAT;
    scalar sendZMax = -VGREAT;

#endif

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
