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
    lation Framework provides FEniCS v2019.1.0 <-> MUI v1.2 <-> OpenFOAM v6
    two-way coupling.

    Incompressible Navier-Stokes equations with two fluid by VOF for fluid domain
	in OpenFOAM and Structure dynamics equations for structure domain in FEniCS.

    It is to fetch displacement of the structure boundary from the structure domain
    in every time and sub-iteration loop of the fluid solver
    Located in the src/CFD/OpenFOAM/V6/applications/solvers/interFSIFoam sub-folder
\*---------------------------------------------------------------------------*/

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef USE_MUI // included if the switch -DUSE_MUI included during compilation.

    mui::point3d rcv_point;

 //   scalar t = mesh.time().value();

    Info << "{OpenFOAM} Fetch iteration " << totalCurrentIter << nl << endl;
    if((totalCurrentIter-0) >= 0)
    {                    
        forAll(interfaceNames, iN)
        {
            int pointCounter = 0;

			bool patchIDToFetch = false;

            forAll(patch6DoFIDs[iN], pI)
            {
				forAll(patchIDs[iN], ptI)
				{
					if(patch6DoFIDs[iN][pI] == patchIDs[iN][ptI])
					{
						patchIDToFetch = true;
						break;
					}
				}
                // Loop over points to receive displacement values and assigned to dispVectorVals
                //- Blocking fetch for value passed through interface from coupled solver  
				if(patchIDToFetch == true)
				{
					forAll(dispVectorValsList[iN][pI], pointI)
					{
						pointCounter++;

						rcv_point[0] = oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::X);
						rcv_point[1] = oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Y);
						rcv_point[2] = oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Z);
	/* *+ */
						if (rankHasPatchPointsForfetch)
						{

							double fetchTempX = 
								ifs[iN]->fetch(fetchNameX, rcv_point, (double)(totalCurrentIter-0), spatial_sampler, chrono_sampler);
							double fetchTempY = 
								ifs[iN]->fetch(fetchNameY, rcv_point, (double)(totalCurrentIter-0), spatial_sampler, chrono_sampler);
							double fetchTempZ = 
								ifs[iN]->fetch(fetchNameZ, rcv_point, (double)(totalCurrentIter-0), spatial_sampler, chrono_sampler);


							double angle2DRotationTemp = rotation2DAmplitude_*Foam::sin(2*3.1416*rotation2DFrq_*t);

							dispVectorValsList[iN][pI][pointI].component(vector::X) = fetchTempX * Foam::cos(angle2DRotationTemp) - fetchTempY * Foam::sin(angle2DRotationTemp);
							dispVectorValsList[iN][pI][pointI].component(vector::Y) = fetchTempX * Foam::sin(angle2DRotationTemp) + fetchTempY * Foam::cos(angle2DRotationTemp);
							dispVectorValsList[iN][pI][pointI].component(vector::Z) = fetchTempZ;

						}

						if(dispVectorValsList[iN][pI][pointI].component(vector::X) != 0.0)
						{
						//	Pout << "dispVectorValsList[iN][pI][pointI].component(vector::X): " << dispVectorValsList[iN][pI][pointI].component(vector::X) << nl << endl;
						}
						if(dispVectorValsList[iN][pI][pointI].component(vector::Y) != 0.0)
						{
						//	Pout << "dispVectorValsList[iN][pI][pointI].component(vector::Y): " << dispVectorValsList[iN][pI][pointI].component(vector::Y) << nl << endl;
						}
						if(dispVectorValsList[iN][pI][pointI].component(vector::Z) != 0.0)
						{
						//	Pout << "dispVectorValsList[iN][pI][pointI].component(vector::Z): " << dispVectorValsList[iN][pI][pointI].component(vector::Z) << nl << endl;
						}
	//        *+ */
						if (cplMethod == "Aitken")
						{
							AK[iN].collectResidual( dispVectorValsList[iN][pI][pointI].component(vector::X),
													dispVectorValsList[iN][pI][pointI].component(vector::Y),
													dispVectorValsList[iN][pI][pointI].component(vector::Z),
													dispVectorOldValsList[iN][pI][pointI].component(vector::X),
													dispVectorOldValsList[iN][pI][pointI].component(vector::Y),
													dispVectorOldValsList[iN][pI][pointI].component(vector::Z),
													pointCounter);
						} else if (cplMethod == "FixedRelaxation")
						{
							FR[iN].collectResidual( dispVectorValsList[iN][pI][pointI].component(vector::X),
													dispVectorValsList[iN][pI][pointI].component(vector::Y),
													dispVectorValsList[iN][pI][pointI].component(vector::Z),
													dispVectorOldValsList[iN][pI][pointI].component(vector::X),
													dispVectorOldValsList[iN][pI][pointI].component(vector::Y),
													dispVectorOldValsList[iN][pI][pointI].component(vector::Z),
													pointCounter);
						} else if (cplMethod == "IQN_ILS")
						{
							IQNILS[iN].collectResidual( dispVectorValsList[iN][pI][pointI].component(vector::X),
														dispVectorValsList[iN][pI][pointI].component(vector::Y),
														dispVectorValsList[iN][pI][pointI].component(vector::Z),
														dispVectorOldValsList[iN][pI][pointI].component(vector::X),
														dispVectorOldValsList[iN][pI][pointI].component(vector::Y),
														dispVectorOldValsList[iN][pI][pointI].component(vector::Z),
														pointCounter);
						} else if (cplMethod == "Loose")
						{
							// Do nothing for Loose coupling method
						} else
						{
							FatalError << "cplFSIMethod: " << cplMethod << " in fsiDict does not recognized!" << exit(FatalError);
						} 
					}

					if (cplMethod == "Aitken")
					{
						AK[iN].process(totalCurrentIter,subIter);
					} else if (cplMethod == "FixedRelaxation")
					{
						FR[iN].process();
					} else if (cplMethod == "IQN_ILS")
					{
						IQNILS[iN].process(totalCurrentIter);
					} else if (cplMethod == "Loose")
					{
						// Do nothing for Loose coupling method
					} else
					{
						FatalError << "cplFSIMethod: " << cplMethod << " in fsiDict does not recognized!" << exit(FatalError);
					}
				}
				patchIDToFetch = false;
            }
        }

    }
    else
    {
        forAll(interfaceNames, iN)
        {
            forAll(patch6DoFIDs[iN], pI)
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

                    if(dispVectorValsList[iN][pI][pointI].component(vector::X) != 0.0)
                    {
                        // Pout << "dispVectorValsList[iN][pI][pointI].component(vector::X): " << dispVectorValsList[iN][pI][pointI].component(vector::X) << nl << endl;
                    }
                    if(dispVectorValsList[iN][pI][pointI].component(vector::Y) != 0.0)
                    {
                        // Pout << "dispVectorValsList[iN][pI][pointI].component(vector::Y): " << dispVectorValsList[iN][pI][pointI].component(vector::Y) << nl << endl;
                    }
                    if(dispVectorValsList[iN][pI][pointI].component(vector::Z) != 0.0)
                    {
                        // Pout << "dispVectorValsList[iN][pI][pointI].component(vector::Z): " << dispVectorValsList[iN][pI][pointI].component(vector::Z) << nl << endl;
                    }
                }
            }
        }
    }

    forAll(interfaceNames, iN)
    {
        int pointCounter = 0;

		bool patchIDToFetch = false;

        forAll(patch6DoFIDs[iN], pI)
        {

            fixedValuePointPatchVectorField& PointPointerLocal = 
                refCast<fixedValuePointPatchVectorField>
                (
                    pointDisplacement->boundaryFieldRef()[patch6DoFIDs[iN][pI]]
                );
/*             //*****************************->
            forAll(dispVectorValsList[iN][pI], pointI)
            {
                dispVectorValsList[iN][pI][pointI].component(vector::X) = 0.0;
                    double amp_local = 0.001;
				if (oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Z) == 0.0)
				{
					dispVectorValsList[iN][pI][pointI].component(vector::Y) = 0.0;
					Pout << " Z0: oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Z): " << oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Z) << nl << endl;
				} else
				{
					dispVectorValsList[iN][pI][pointI].component(vector::Y) = 
						amp_local*((oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::X)/0.1))*Foam::sin(2.0*3.14*8333.333* (runTime.value()));
					Pout << " ZN0: oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Z): " << oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Z) << nl << endl;
                }

				dispVectorValsList[iN][pI][pointI].component(vector::Z) = 0.0;
            }
            //*****************************<- */
            // Loop over points to assign dispVectorVals to cellDisplacement boundaryFieldRef, i.e. move the nodes
            forAll(dispVectorValsList[iN][pI], pointI)
            {

				mui::point3d rot_pointTemp;

				rot_pointTemp[0] = oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::X);
				rot_pointTemp[1] = oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Y);
				rot_pointTemp[2] = oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Z);

				double angle2DRotationTemp = rotation2DAmplitude_*Foam::sin(2*3.1416*rotation2DFrq_*t);

				PointPointerLocal[pointI].component(vector::X) = Foam::cos(angle2DRotationTemp) * (rot_pointTemp[0] - rotation2DOriginX_) - Foam::sin(angle2DRotationTemp) * (rot_pointTemp[1] - rotation2DOriginY_) + rotation2DOriginX_ - rot_pointTemp[0];
				PointPointerLocal[pointI].component(vector::Y) = Foam::sin(angle2DRotationTemp) * (rot_pointTemp[0] - rotation2DOriginX_) + Foam::cos(angle2DRotationTemp) * (rot_pointTemp[1] - rotation2DOriginY_) + rotation2DOriginY_ - rot_pointTemp[1];


                PointPointerLocal[pointI].component(vector::X) += rigidMotionAmplitudeX_*Foam::sin(2*3.1416*rigidMotionFrqX_*t);
                PointPointerLocal[pointI].component(vector::Y) += rigidMotionAmplitudeY_*Foam::sin(2*3.1416*rigidMotionFrqY_*t);
                PointPointerLocal[pointI].component(vector::Z) = rigidMotionAmplitudeZ_*Foam::sin(2*3.1416*rigidMotionFrqZ_*t);


				forAll(patchIDs[iN], ptI)
				{
					if(patch6DoFIDs[iN][pI] == patchIDs[iN][ptI])
					{
						patchIDToFetch = true;
						break;
					}
				}
				
				if(patchIDToFetch == true)
				{
					pointCounter++;

					if (cplMethod == "Aitken")
					{
						if (applyFetchX)
						{
							PointPointerLocal[pointI].component(vector::X) += dispVectorOldValsList[iN][pI][pointI].component(vector::X) + AK[iN].getXDeltaDisp(pointCounter);
							dispVectorOldValsList[iN][pI][pointI].component(vector::X) += AK[iN].getXDeltaDisp(pointCounter);
						}
						if (applyFetchY)
						{
							PointPointerLocal[pointI].component(vector::Y) += dispVectorOldValsList[iN][pI][pointI].component(vector::Y) + AK[iN].getYDeltaDisp(pointCounter);
							dispVectorOldValsList[iN][pI][pointI].component(vector::Y) += AK[iN].getYDeltaDisp(pointCounter);
						}
						if (applyFetchZ)
						{                       
							PointPointerLocal[pointI].component(vector::Z) += dispVectorOldValsList[iN][pI][pointI].component(vector::Z) + AK[iN].getZDeltaDisp(pointCounter);
							dispVectorOldValsList[iN][pI][pointI].component(vector::Z) += AK[iN].getZDeltaDisp(pointCounter);
						}

					} else if (cplMethod == "FixedRelaxation")
					{
						if (applyFetchX)
						{
							PointPointerLocal[pointI].component(vector::X) += dispVectorOldValsList[iN][pI][pointI].component(vector::X) + FR[iN].getXDeltaDisp(pointCounter);
							dispVectorOldValsList[iN][pI][pointI].component(vector::X) += FR[iN].getXDeltaDisp(pointCounter);
						}
						if (applyFetchY)
						{
							PointPointerLocal[pointI].component(vector::Y) += dispVectorOldValsList[iN][pI][pointI].component(vector::Y) + FR[iN].getYDeltaDisp(pointCounter);
							dispVectorOldValsList[iN][pI][pointI].component(vector::Y) += FR[iN].getYDeltaDisp(pointCounter);
						}
						if (applyFetchZ)
						{
							PointPointerLocal[pointI].component(vector::Z) += dispVectorOldValsList[iN][pI][pointI].component(vector::Z) + FR[iN].getZDeltaDisp(pointCounter);
							dispVectorOldValsList[iN][pI][pointI].component(vector::Z) += FR[iN].getZDeltaDisp(pointCounter);
						}
					} else if (cplMethod == "IQN_ILS")
					{

						if (applyFetchX)
						{
							PointPointerLocal[pointI].component(vector::X) += dispVectorOldValsList[iN][pI][pointI].component(vector::X) + IQNILS[iN].getXDeltaDisp(pointCounter);
							dispVectorOldValsList[iN][pI][pointI].component(vector::X) += IQNILS[iN].getXDeltaDisp(pointCounter);
						}
						if (applyFetchY)
						{
							PointPointerLocal[pointI].component(vector::Y) += dispVectorOldValsList[iN][pI][pointI].component(vector::Y) + IQNILS[iN].getYDeltaDisp(pointCounter);
							dispVectorOldValsList[iN][pI][pointI].component(vector::Y) += IQNILS[iN].getYDeltaDisp(pointCounter);
						}
						if (applyFetchZ)
						{
							PointPointerLocal[pointI].component(vector::Z) += dispVectorOldValsList[iN][pI][pointI].component(vector::Z) + IQNILS[iN].getZDeltaDisp(pointCounter);
							dispVectorOldValsList[iN][pI][pointI].component(vector::Z) += IQNILS[iN].getZDeltaDisp(pointCounter);
						}

					} else if (cplMethod == "Loose")
					{

						PointPointerLocal[pointI].component(vector::X) += dispVectorValsList[iN][pI][pointI].component(vector::X);
						PointPointerLocal[pointI].component(vector::Y) += dispVectorValsList[iN][pI][pointI].component(vector::Y);
						PointPointerLocal[pointI].component(vector::Z) += dispVectorValsList[iN][pI][pointI].component(vector::Z);

					} else
					{
						FatalError << "cplFSIMethod: " << cplMethod << " in fsiDict does not recognized!" << exit(FatalError);
					}         

					if(PointPointerLocal[pointI].component(vector::X) != 0.0)
					{
						//Pout << "PointPointerLocal[pointI].component(vector::X): " << PointPointerLocal[pointI].component(vector::X) << nl << endl;
					}
					if(PointPointerLocal[pointI].component(vector::Y) != 0.0)
					{
						//Pout << "PointPointerLocal[pointI].component(vector::Y): " << PointPointerLocal[pointI].component(vector::Y) << nl << endl;
					}
					if(PointPointerLocal[pointI].component(vector::Z) != 0.0)
					{
						//Pout << "PointPointerLocal[pointI].component(vector::Z): " << PointPointerLocal[pointI].component(vector::Z) << nl << endl;
					}


					if(Pstream::master())
					{
						if (applyFetchX || applyFetchY || applyFetchZ)
						{
							if (cplMethod == "Aitken")
							{
								muiResidual << interfaceNames[iN] << "\t" << runTime.timeName() << "\t" << timeSteps << "\t" << subIter << "\t" 
									<< AK[iN].undRelxCpl() << "\t" << AK[iN].pointSize() << "\t" << AK[iN].residualMagSqSum() << "\t"
									<< AK[iN].residualL2NormMax() << "\t" << AK[iN].residualL2Norm() << "\t" << forceTotal[0].component(vector::X) 
									<< "\t" << forceTotal[0].component(vector::Y) << "\t" << forceTotal[0].component(vector::Z) << endl;
							} else if (cplMethod == "FixedRelaxation")
							{
								muiResidual << interfaceNames[iN] << "\t" << runTime.timeName() << "\t" << timeSteps << "\t" << subIter << "\t" 
									<< FR[iN].undRelxCpl() << "\t" << FR[iN].pointSize() << "\t" << FR[iN].residualMagSqSum() << "\t"
									<< FR[iN].residualL2NormMax() << "\t" << FR[iN].residualL2Norm() << "\t" << forceTotal[0].component(vector::X) 
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
								FatalError << "cplFSIMethod: " << cplMethod << " in fsiDict does not recognized!" << exit(FatalError);
							}  
						}
					}
				}
			}
			patchIDToFetch = false;
        }
    }

#else

    forAll(interfaceNames, iN)
    {
        forAll(patch6DoFIDs[iN], pI)
        {
            fixedValuePointPatchVectorField& PointPointerLocal = 
                refCast<fixedValuePointPatchVectorField>
                (
                    pointDisplacement->boundaryFieldRef()[patch6DoFIDs[iN][pI]]
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