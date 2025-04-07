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
    
    @file fetchDisplacementInit.H
    
    @author W. Liu
    
    @brief This is a part of the Parallel Partitioned Multi-physical Simu-
    lation Framework provides FEniCS v2019.1.0 <-> MUI v1.2 <-> OpenFOAM v6 
    two-way coupling.

    Incompressible Navier-Stokes equations with two fluid by VOF for fluid domain 
	in OpenFOAM and Structure dynamics equations for structure domain in FEniCS.

    It is to initialise the function of fetch displacement of the structure boundary 
    from the structure domain before the time loop of the fluid solver 
    Located in the src/CFD/OpenFOAM/V6/applications/solvers/interFSIFoam sub-folder
\*---------------------------------------------------------------------------*/

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


    // Declare MUI FSI residual output.
    fileName muiResidualFile("muiFSIResidual.txt");
    OFstream muiResidual(runTime.system()/muiResidualFile);

    pointVectorField* pointDisplacement = 
        const_cast<pointVectorField*>
        (
            &mesh.lookupObject<pointVectorField>
            (
                "pointDisplacement"
            )
        );

    List<List<vectorField>> dispVectorValsList;
    List<List<vectorField>> dispVectorOldValsList;
    List<List<vectorField>> oriCoordFetchPointsVectorValsList;

    bool rankHasPatchPointsForfetch = false;

    forAll(interfaceNames, iN)
    {

        List<vectorField> dispVectorValsListLocal;
        List<vectorField> dispVectorOldValsListLocal;
        List<vectorField> oriCoordFetchPointsVectorValsListLocal;

        forAll(patch6DoFIDs[iN], pI)
        {

            if(pointDisplacement->boundaryFieldRef()[patch6DoFIDs[iN][pI]].size() != 0)
            {
                rankHasPatchPointsForfetch = true;
            }

            // Declare a vectorField to store the fetched displacements.
            vectorField dispVectorValsLocal(pointDisplacement->boundaryFieldRef()[patch6DoFIDs[iN][pI]].size());
            vectorField dispVectorOldValsLocal(pointDisplacement->boundaryFieldRef()[patch6DoFIDs[iN][pI]].size());
            // Declare a vectorField to store the original point coordinates.
            vectorField oriCoordFetchPointsVectorValsLocal(pointDisplacement->boundaryFieldRef()[patch6DoFIDs[iN][pI]].size());

            dispVectorValsListLocal.append(dispVectorValsLocal);
            dispVectorOldValsListLocal.append(dispVectorOldValsLocal);
            oriCoordFetchPointsVectorValsListLocal.append(oriCoordFetchPointsVectorValsLocal);
        }

        dispVectorValsList.append(dispVectorValsListLocal);
        dispVectorOldValsList.append(dispVectorOldValsListLocal);
        oriCoordFetchPointsVectorValsList.append(oriCoordFetchPointsVectorValsListLocal);
    }
   
#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.

        scalar recvXMin = VGREAT;
        scalar recvYMin = VGREAT;
        scalar recvZMin = VGREAT;

        scalar recvXMax = -VGREAT;
        scalar recvYMax = -VGREAT;
        scalar recvZMax = -VGREAT;

#endif

    forAll(interfaceNames, iN)
    {

        forAll(patch6DoFIDs[iN], pI)
        {
            // Loop over points to Store the original point coordinates
            forAll(dispVectorValsList[iN][pI], pointI)
            {
                oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::X) = 
                    mesh.boundaryMesh()[patch6DoFIDs[iN][pI]].localPoints()[pointI].component(vector::X);
                oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Y) = 
                    mesh.boundaryMesh()[patch6DoFIDs[iN][pI]].localPoints()[pointI].component(vector::Y);
                oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Z) = 
                    mesh.boundaryMesh()[patch6DoFIDs[iN][pI]].localPoints()[pointI].component(vector::Z);

                dispVectorValsList[iN][pI][pointI].component(vector::X) = 0.0;
                dispVectorValsList[iN][pI][pointI].component(vector::Y) = 0.0;
                dispVectorValsList[iN][pI][pointI].component(vector::Z) = 0.0;

                dispVectorOldValsList[iN][pI][pointI].component(vector::X) = 0.0;
                dispVectorOldValsList[iN][pI][pointI].component(vector::Y) = 0.0;
                dispVectorOldValsList[iN][pI][pointI].component(vector::Z) = 0.0;
            }
        }
    }

#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.

    if (args.cplRunControl().cplRun())
    {

        const pointField& meshPoints = mesh.points();


        if (rankHasPatchPointsForfetch)
        {
            forAll(interfaceNames, iN)
            {

                forAll(patch6DoFIDs[iN], pI)
                {
                    forAll(oriCoordFetchPointsVectorValsList[iN][pI], pointI)
                    {
                        recvXMin = 
                            ((oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::X)) < recvXMin) ? (oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::X)) : recvXMin;
                        recvYMin = 
                            ((oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Y)) < recvYMin) ? (oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Y)) : recvYMin;
                        recvZMin = 
                            ((oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Z)) < recvZMin) ? (oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Z)) : recvZMin;

                        recvXMax = 
                            ((oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::X)) > recvXMax) ? (oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::X)) : recvXMax;
                        recvYMax = 
                            ((oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Y)) > recvYMax) ? (oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Y)) : recvYMax;
                        recvZMax = 
                            ((oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Z)) > recvZMax) ? (oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Z)) : recvZMax;
                    }
                }
            }
        } else
        {
            forAll(meshPoints, pts)
            {
                recvXMin = 
                    (meshPoints[pts][0] < recvXMin) ? meshPoints[pts][0] : recvXMin;
                recvYMin = 
                    (meshPoints[pts][1] < recvYMin) ? meshPoints[pts][1] : recvYMin;
                recvZMin = 
                    (meshPoints[pts][2] < recvZMin) ? meshPoints[pts][2] : recvZMin;

                recvXMax = 
                    (meshPoints[pts][0] > recvXMax) ? meshPoints[pts][0] : recvXMax;
                recvYMax = 
                    (meshPoints[pts][1] > recvYMax) ? meshPoints[pts][1] : recvYMax;
                recvZMax = 
                    (meshPoints[pts][2] > recvZMax) ? meshPoints[pts][2] : recvZMax;
            }
        }
    }

#endif

#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.

    // define spatial and temporal samplers
    mui::sampler_pseudo_n2_linear3d<double> spatial_sampler(rSampler);
    mui::chrono_sampler_exact3d chrono_sampler;

    std::vector<muiCoupling::muiCouplingAitken> AK; 
    AK.reserve(interfaceNames.size());
    std::vector<muiCoupling::muiCouplingFixedRelaxation> FR; 
    FR.reserve(interfaceNames.size());
    std::vector<muiCoupling::muiCouplingIQNILS> IQNILS; 
    IQNILS.reserve(interfaceNames.size());

    // Construct an application of muiCouplingAitken
    muiCoupling::muiCouplingAitken AKLocal((double)(initUndRelxCpl));
    // Construct an application of muiCouplingFixedRelaxation
    muiCoupling::muiCouplingFixedRelaxation FRLocal((double)(initUndRelxCpl));
    // Construct an application of muiCouplingIQNILS
    muiCoupling::muiCouplingIQNILS IQNILSLocal((double)(initUndRelxCpl));

    forAll(interfaceNames, iN)
    {
        AK.emplace_back(AKLocal);
        FR.emplace_back(FRLocal);
        IQNILS.emplace_back(IQNILSLocal);
    }

    if (args.cplRunControl().cplRun())
    {

        int totPointSize = 0;

        forAll(interfaceNames, iN)
        {

            forAll(patch6DoFIDs[iN], pI)
            {

                totPointSize += pointDisplacement->boundaryFieldRef()[patch6DoFIDs[iN][pI]].size();
            }

            if (cplMethod == "Aitken")
            {
                AK[iN].initialize((int)(totPointSize));
                Info << "{OpenFOAM} interface " << interfaceNames[iN] << " Aitken coupling method selected!" << endl;

                muiResidual << "MUI CouplingLib Utility" << "\t" << "Aitken method" << endl;
                muiResidual << "interfaceNames"<< "\t" << "Time" << "\t" << "Time-Step" << "\t" << "Sub-Iteration" << "\t" 
                    << "UnderRelx-Factor" << "\t" << "pointSize" << "\t" 
                    << "squareSumResidual" << "\t" << "maxResidualL-2Norm" << "\t" 
                    << "residualL2Norm" << "\t" << "forceX" 
                            << "\t" << "forceY" << "\t" << "forceZ"<< endl;

            } else if (cplMethod == "FixedRelaxation")
            {
                FR[iN].initialize((int)(totPointSize));
                Info << "{OpenFOAM} interface " << interfaceNames[iN] << " Fixed Relaxation coupling method selected!" << endl;

                muiResidual << "MUI CouplingLib Utility" << "\t" << "FixedRelaxation method" << endl;
                muiResidual << "interfaceNames"<< "\t" << "Time" << "\t" << "Time-Step" << "\t" << "Sub-Iteration" << "\t" 
                    << "UnderRelx-Factor" << "\t" << "pointSize" << "\t" 
                    << "squareSumResidual" << "\t" << "maxResidualL-2Norm" << "\t" 
                    << "residualL2Norm" << "\t" << "forceX" 
                            << "\t" << "forceY" << "\t" << "forceZ"<< endl;

            } else if (cplMethod == "IQN_ILS")
            {
                IQNILS[iN].initialize((int)(totPointSize), 
                                      (double)(maxUndRelxCpl),
                                      (int)(numAitkenIterations));
                Info << "{OpenFOAM} interface " << interfaceNames[iN] << " IQN-ILS coupling method selected!" << endl;

                muiResidual << "MUI CouplingLib Utility" << "\t" << "IQN-ILS method" << endl;
                muiResidual << "interfaceName"<< "\t" << "Time" << "\t" << "Time-Step" << "\t" << "Sub-Iteration" << "\t" 
                    << "UnderRelx-Factor" << "\t" << "pointSize" << "\t" 
                    << "squareSumResidual" << "\t" << "maxResidualL-2Norm" << "\t" 
                    << "residualL2Norm" << "\t" << "forceX" 
                            << "\t" << "forceY" << "\t" << "forceZ"<< endl;

            } else if (cplMethod == "Loose")
            {
                Info << "{OpenFOAM} Loose coupling method selected!" << endl;
            } else
            {
                FatalError << "cplFSIMethod: " << cplMethod << " in fsiDict does not recognized!" << exit(FatalError);
            }
        }

        Pout << "recvXMin = " << recvXMin << endl;
        Pout << "recvYMin = " << recvYMin << endl;
        Pout << "recvZMin = " << recvZMin << endl;

        Pout << "recvXMax = " << recvXMax << endl;
        Pout << "recvYMax = " << recvYMax << endl;
        Pout << "recvZMax = " << recvZMax << endl;

        // Annouce send span
        mui::geometry::box3d recv_region( {recvXMin, recvYMin, recvZMin}, {recvXMax, recvYMax, recvZMax} );

        // Set the maximum time step size
        int totalTimeSteps = ceil((runTime.endTime().value() - runTime.startTime().value())/runTime.deltaT().value());

        forAll(interfaceNames, iN)
        {
            ifs[iN]->announce_recv_span( 0, totalTimeSteps, recv_region );

            // Commit ZERO step
            ifs[iN]->commit(0);
            Info << "MUI interface " << interfaceNames[iN] << " commit zero step" << nl << endl;
        }
    }

#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// *************************  FILE END  ************************************ //