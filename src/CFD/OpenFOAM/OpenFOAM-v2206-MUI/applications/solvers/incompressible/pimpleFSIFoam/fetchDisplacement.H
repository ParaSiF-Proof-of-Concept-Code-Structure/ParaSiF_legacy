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
    
    @file fetchDisplacement.H
    
    @author W. Liu
    
    @brief This is a part of the Parallel Partitioned Multi-physical Simu-
    lation Framework provides FEniCS v2019.1.0 <-> MUI v1.0 <-> OpenFOAM v6 
    two-way coupling.

    Incompressible Navier-Stokes equations for fluid domain in OpenFOAM
    Structure dynamics equations for structure domain in FEniCS.

    It is to fetch displacement of the structure boundary from the structure domain 
    in every time and sub-iteration loop of the fluid solver
    Located in the src/CFD/OpenFOAM/V6/applications/solvers/pimpleFSIFoam sub-folder
\*---------------------------------------------------------------------------*/

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.

	// Debug switch for fetchDisplacement.H
	bool debugFetchDisplacement = false;

    mui::point3d rcv_point;

    scalar t = mesh.time().value();

    Info << "{OpenFOAM} Fetch iteration " << totalCurrentIter << nl << endl;
    if((totalCurrentIter-0) >= 0)
    {                    
	cplAlgo_res = 0.0;
	int count = 0;
        forAll(interfaceNames, iN)
        {
            forAll(patchIDs[iN], pI)
            {
                // Loop over points to receive displacement values and assigned to dispVectorVals
                //- Blocking fetch for value passed through interface from coupled solver  

                forAll(dispVectorValsList[iN][pI], pointI)
                {
                    rcv_point[0] = oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::X);
                    rcv_point[1] = oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Y);
                    rcv_point[2] = oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Z);

                    if (rankHasPatchPointsForfetch)
                    {
												if (cplMethod == "Aitken")
												{
                        dispVectorValsList[iN][pI][pointI].component(vector::X) = 
		                            ifs[iN]->fetch(fetchNameX, rcv_point, static_cast<double>(totalCurrentIter-0), spatial_sampler, chrono_sampler,aitken);
                        dispVectorValsList[iN][pI][pointI].component(vector::Y) = 
		                            ifs[iN]->fetch(fetchNameY, rcv_point, static_cast<double>(totalCurrentIter-0), spatial_sampler, chrono_sampler,aitken);
                        dispVectorValsList[iN][pI][pointI].component(vector::Z) = 
		                            ifs[iN]->fetch(fetchNameZ, rcv_point, static_cast<double>(totalCurrentIter-0), spatial_sampler, chrono_sampler,aitken);
																cplAlgo_res = max (cplAlgo_res, aitken.get_residual_L2_Norm(static_cast<double>(totalCurrentIter-0)));
																count++;
												} else if (cplMethod == "FixedRelaxation")
                    {
														dispVectorValsList[iN][pI][pointI].component(vector::X) =
																ifs[iN]->fetch(fetchNameX, rcv_point, static_cast<double>(totalCurrentIter-0), spatial_sampler, chrono_sampler,fr);
														dispVectorValsList[iN][pI][pointI].component(vector::Y) =
																ifs[iN]->fetch(fetchNameY, rcv_point, static_cast<double>(totalCurrentIter-0), spatial_sampler, chrono_sampler,fr);
														dispVectorValsList[iN][pI][pointI].component(vector::Z) =
																ifs[iN]->fetch(fetchNameZ, rcv_point, static_cast<double>(totalCurrentIter-0), spatial_sampler, chrono_sampler,fr);

												}  else if (cplMethod == "Loose")
                    {
													 dispVectorValsList[iN][pI][pointI].component(vector::X) =
															 ifs[iN]->fetch(fetchNameX, rcv_point, static_cast<double>(totalCurrentIter-0), spatial_sampler, chrono_sampler);
													 dispVectorValsList[iN][pI][pointI].component(vector::Y) =
															 ifs[iN]->fetch(fetchNameY, rcv_point, static_cast<double>(totalCurrentIter-0), spatial_sampler, chrono_sampler);
													 dispVectorValsList[iN][pI][pointI].component(vector::Z) =
															 ifs[iN]->fetch(fetchNameZ, rcv_point, static_cast<double>(totalCurrentIter-0), spatial_sampler, chrono_sampler);
                    } else
                    {
		                        FatalError << "cplFSIMethod: " << cplMethod << " in fsiDict is not recognized!" << exit(FatalError);
                    } 
                }

                }

            }
						reduce(cplAlgo_res, maxOp<double>());
        }

    }
    else
    {
        forAll(interfaceNames, iN)
        {
            forAll(patchIDs[iN], pI)
            {
                // Loop over points to receive displacement values and assigned to dispVectorVals
                //- Blocking fetch for value passed through interface from coupled solver  
                forAll(dispVectorValsList[iN][pI], pointI)
                {
                    rcv_point[0] = oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::X);
                    rcv_point[1] = oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Y);
                    rcv_point[2] = oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Z);

                    dispVectorValsList[iN][pI][pointI].component(vector::X) = 0.0; 

                    dispVectorValsList[iN][pI][pointI].component(vector::Y) = 0.0;

                    dispVectorValsList[iN][pI][pointI].component(vector::Z) = 0.0;

                    if((dispVectorValsList[iN][pI][pointI].component(vector::X) != 0.0) && (debugFetchDisplacement == true))
                    {
                        Pout << "dispVectorValsList[iN][pI][pointI].component(vector::X): " << dispVectorValsList[iN][pI][pointI].component(vector::X) << nl << endl;
                    }
                    if((dispVectorValsList[iN][pI][pointI].component(vector::Y) != 0.0) && (debugFetchDisplacement == true))
                    {
                        Pout << "dispVectorValsList[iN][pI][pointI].component(vector::Y): " << dispVectorValsList[iN][pI][pointI].component(vector::Y) << nl << endl;
                    }
                    if((dispVectorValsList[iN][pI][pointI].component(vector::Z) != 0.0) && (debugFetchDisplacement == true))
                    {
                        Pout << "dispVectorValsList[iN][pI][pointI].component(vector::Z): " << dispVectorValsList[iN][pI][pointI].component(vector::Z) << nl << endl;
                    }
                }
            }
        }
    }

    forAll(interfaceNames, iN)
    {
        forAll(patchIDs[iN], pI)
        {

            fixedValuePointPatchVectorField& PointPointerLocal = 
                refCast<fixedValuePointPatchVectorField>
                (
                    pointDisplacement.boundaryFieldRef()[patchIDs[iN][pI]]
                );

            // Loop over points to assign dispVectorVals to cellDisplacement boundaryFieldRef, i.e. move the nodes
            forAll(dispVectorValsList[iN][pI], pointI)
            {
                PointPointerLocal[pointI].component(vector::X) = rigidMotionAmplitudeX_*Foam::sin(2*3.1416*rigidMotionFrqX_*t);
                PointPointerLocal[pointI].component(vector::Y) = rigidMotionAmplitudeY_*Foam::sin(2*3.1416*rigidMotionFrqY_*t);
                PointPointerLocal[pointI].component(vector::Z) = rigidMotionAmplitudeZ_*Foam::sin(2*3.1416*rigidMotionFrqZ_*t);

                    if (applyFetchX)
                    {
                    PointPointerLocal[pointI].component(vector::X) += dispVectorValsList[iN][pI][pointI].component(vector::X);
                    }
                    if (applyFetchY)
                    {
                    PointPointerLocal[pointI].component(vector::Y) += dispVectorValsList[iN][pI][pointI].component(vector::Y);
                    }
                    if (applyFetchZ)
                    {                       
                    PointPointerLocal[pointI].component(vector::Z) += dispVectorValsList[iN][pI][pointI].component(vector::Z);
                    }



								if((dispVectorValsList[iN][pI][pointI].component(vector::X) != 0.0) && (debugFetchDisplacement == true))
                    {
										Pout << "dispVectorValsList[iN][pI][pointI].component(vector::X): " << dispVectorValsList[iN][pI][pointI].component(vector::X) << nl << endl;
                    }
								if((dispVectorValsList[iN][pI][pointI].component(vector::Y) != 0.0) && (debugFetchDisplacement == true))
                    {
										Pout << "dispVectorValsList[iN][pI][pointI].component(vector::Y): " << dispVectorValsList[iN][pI][pointI].component(vector::Y) << nl << endl;
                    }
								if((dispVectorValsList[iN][pI][pointI].component(vector::Z) != 0.0) && (debugFetchDisplacement == true))
                    {
										Pout << "dispVectorValsList[iN][pI][pointI].component(vector::Z): " << dispVectorValsList[iN][pI][pointI].component(vector::Z) << nl << endl;
                }         

                if((PointPointerLocal[pointI].component(vector::X) != 0.0) && (debugFetchDisplacement == true))
                {
                    Pout << "PointPointerLocal[pointI].component(vector::X): " << PointPointerLocal[pointI].component(vector::X) << nl << endl;
                }
                if((PointPointerLocal[pointI].component(vector::Y) != 0.0) && (debugFetchDisplacement == true))
                {
                    Pout << "PointPointerLocal[pointI].component(vector::Y): " << PointPointerLocal[pointI].component(vector::Y) << nl << endl;
                }
                if((PointPointerLocal[pointI].component(vector::Z) != 0.0) && (debugFetchDisplacement == true))
                {
                    Pout << "PointPointerLocal[pointI].component(vector::Z): " << PointPointerLocal[pointI].component(vector::Z) << nl << endl;
                }

            }
            if(Pstream::master())
            {
                if (applyFetchX || applyFetchY || applyFetchZ)
                {
                    if (cplMethod == "Aitken")
                    {
                        muiResidual << interfaceNames[iN] << "\t" << runTime.timeName() << "\t" << timeSteps << "\t" << subIter << "\t" 
                            // << AK[iN].undRelxCpl() << "\t" << AK[iN].pointSize() << "\t" << AK[iN].residualMagSqSum() << "\t"
                            // << AK[iN].residualL2NormMax() << "\t" << AK[iN].residualL2Norm() << "\t" << forceTotal[0].component(vector::X)
                            << "\t" << forceTotal[0].component(vector::Y) << "\t" << forceTotal[0].component(vector::Z) << endl;
                    } else if (cplMethod == "FixedRelaxation")
                    {
                        muiResidual << interfaceNames[iN] << "\t" << runTime.timeName() << "\t" << timeSteps << "\t" << subIter << "\t" 
                            // << FR[iN].undRelxCpl() << "\t" << FR[iN].pointSize() << "\t" << FR[iN].residualMagSqSum() << "\t"
                            // << FR[iN].residualL2NormMax() << "\t" << FR[iN].residualL2Norm() << "\t" << forceTotal[0].component(vector::X)
                            << "\t" << forceTotal[0].component(vector::Y) << "\t" << forceTotal[0].component(vector::Z) << endl;
                    } else if (cplMethod == "IQN_ILS")
                    {
                        muiResidual << interfaceNames[iN] << "\t" << runTime.timeName() << "\t" << timeSteps << "\t" << subIter << "\t" 
                            << IQNILS[iN].undRelxCpl() << "\t" << IQNILS[iN].pointSize() << "\t" << IQNILS[iN].residualMagSqSum() << "\t"
                            << IQNILS[iN].residualL2NormMax() << "\t" << IQNILS[iN].residualL2Norm() << "\t" << forceTotal[0].component(vector::X) 
                            << "\t" << forceTotal[0].component(vector::Y) << "\t" << forceTotal[0].component(vector::Z)<< endl;
                    } else if (cplMethod == "Loose")
                    {
                        // Do nothing for Loose coupling method
                    } else
                    {
                        FatalError << "cplFSIMethod: " << cplMethod << " in fsiDict is not recognized!" << exit(FatalError);
                    }  
                }
            }
        }
    }

#else

    forAll(interfaceNames, iN)
    {
        forAll(patchIDs[iN], pI)
        {
            fixedValuePointPatchVectorField& PointPointerLocal = 
                refCast<fixedValuePointPatchVectorField>
                (
                    pointDisplacement.boundaryFieldRef()[patchIDs[iN][pI]]
                );

            PointPointerLocal[pointI].component(vector::X) = rigidMotionAmplitudeX_*Foam::sin(2*3.1416*rigidMotionFrqX_*t);
            PointPointerLocal[pointI].component(vector::Y) = rigidMotionAmplitudeY_*Foam::sin(2*3.1416*rigidMotionFrqY_*t);
            PointPointerLocal[pointI].component(vector::Z) = rigidMotionAmplitudeZ_*Foam::sin(2*3.1416*rigidMotionFrqZ_*t);

            // Loop over points to receive displacement values and assigned to dispVectorVals
            forAll(dispVectorValsList[iN][pI], pointI)
            {
                dispVectorValsList[iN][pI][pointI].component(vector::X) = 
                    0.0;
                dispVectorValsList[iN][pI][pointI].component(vector::Y) = 
                    ((oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::X)/0.04)*0.001)*Foam::sin(2.0*3.14*15.625* (runTime.value()));
                dispVectorValsList[iN][pI][pointI].component(vector::Z) = 
                    0.0;
            }

            // Loop over points to assign dispVectorVals to cellDisplacement boundaryFieldRef, i.e. move the nodes
            forAll(dispVectorValsList[iN][pI], pointI)
            {
                if (applyFetchX)
                {
                    PointPointerLocal[pointI].component(vector::X) += dispVectorValsList[iN][pI][pointI].component(vector::X);
                }
                if (applyFetchY)
                {
                    PointPointerLocal[pointI].component(vector::Y) += dispVectorValsList[iN][pI][pointI].component(vector::Y);
                }
                if (applyFetchZ)
                {
                    PointPointerLocal[pointI].component(vector::Z) += dispVectorValsList[iN][pI][pointI].component(vector::Z);
                }

            }
        }
    }
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// *************************  FILE END  ************************************ //
