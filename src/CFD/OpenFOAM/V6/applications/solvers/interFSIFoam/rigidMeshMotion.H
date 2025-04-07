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

    @file rigidMeshMotion.H

    @author W. Liu

    @brief This is a part of the Parallel Partitioned Multi-physical Simu-
    lation Framework provides FEniCS v2019.1.0 <-> MUI v1.2 <-> OpenFOAM v6
    two-way coupling.

    Incompressible Navier-Stokes equations with two fluid by VOF for fluid domain
	in OpenFOAM and Structure dynamics equations for structure domain in FEniCS.

    Located in the src/CFD/OpenFOAM/V6/applications/solvers/interFSIFoam sub-folder
\*---------------------------------------------------------------------------*/

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    scalar t = mesh.time().value();

/* 
    List<int> localRigidPatchIDs;

    forAll(mesh.boundaryMesh().names(), pN)
    {
        int localRigidPatchID = mesh.boundaryMesh().findPatchID(mesh.boundaryMesh().names()[pN]);

        Info << "PatchNameForRigidMotion = " << mesh.boundaryMesh().names()[pN] << " has ID = " << localRigidPatchID  
        << " has types: " << mesh.boundaryMesh().types()[pN] << endl;

        // Check if patch exists
        if(localRigidPatchID == -1)
        {
            FatalError << "PatchName " << mesh.boundaryMesh().names()[pN] << " not found!" << exit(FatalError);
        }

        if (mesh.boundaryMesh().types()[pN] != "processor")
        {
            localRigidPatchIDs.append(localRigidPatchID);
        }

    } */
/* 
    forAll(localRigidPatchIDs, pI)
    {
        fixedValuePointPatchVectorField& PointPointerLocal = 
            refCast<fixedValuePointPatchVectorField>
            (
                pointDisplacement->boundaryFieldRef()[localRigidPatchIDs[pI]]
            );

        label found = 0;

        forAll(patchRigidMUIIDs, pNN)
        {
            if (localRigidPatchIDs[pI] == patchRigidMUIIDs[pNN])
            {
                found = 1;
            }
        }

        if (found == 0)
        {
            forAll(pointDisplacement->boundaryFieldRef()[localRigidPatchIDs[pI]], pointI)
            {
                PointPointerLocal[pointI].component(vector::X) = rigidMotionAmplitudeX_*Foam::sin(2*3.1416*rigidMotionFrqX_*t);
                PointPointerLocal[pointI].component(vector::Y) = rigidMotionAmplitudeY_*Foam::sin(2*3.1416*rigidMotionFrqY_*t);
                PointPointerLocal[pointI].component(vector::Z) = rigidMotionAmplitudeZ_*Foam::sin(2*3.1416*rigidMotionFrqZ_*t);
            }
        } else
        {
            forAll(pointDisplacement->boundaryFieldRef()[localRigidPatchIDs[pI]], pointI)
            {
                PointPointerLocal[pointI].component(vector::X) = rigidMotionAmplitudeX_*Foam::sin(2*3.1416*rigidMotionFrqX_*t);
                PointPointerLocal[pointI].component(vector::Y) = rigidMotionAmplitudeY_*Foam::sin(2*3.1416*rigidMotionFrqY_*t);
                PointPointerLocal[pointI].component(vector::Z) = rigidMotionAmplitudeZ_*Foam::sin(2*3.1416*rigidMotionFrqZ_*t);
            }
        }

    } */
    forAll(interfaceNames, iN)
    {
		forAll(patch6DoFIDs[iN], pI)
		{
			fixedValuePointPatchVectorField& PointPointerLocal = 
				refCast<fixedValuePointPatchVectorField>
				(
					pointDisplacement->boundaryFieldRef()[patch6DoFIDs[iN][pI]]
				);

				forAll(PointPointerLocal, pointI)
				{
					forAll(dispVectorValsList[iN][pI], pointI)
					{
						mui::point3d rot_pointTemp;

						rot_pointTemp[0] = oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::X);
						rot_pointTemp[1] = oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Y);
						rot_pointTemp[2] = oriCoordFetchPointsVectorValsList[iN][pI][pointI].component(vector::Z);

						double angle2DRotationTemp = rotation2DAmplitude_*Foam::sin(2*3.1416*rotation2DFrq_*t);

						PointPointerLocal[pointI].component(vector::X) += Foam::cos(angle2DRotationTemp) * (rot_pointTemp[0] - rotation2DOriginX_) - Foam::sin(angle2DRotationTemp) * (rot_pointTemp[1] - rotation2DOriginY_) + rotation2DOriginX_ - rot_pointTemp[0];
						PointPointerLocal[pointI].component(vector::Y) += Foam::sin(angle2DRotationTemp) * (rot_pointTemp[0] - rotation2DOriginX_) + Foam::cos(angle2DRotationTemp) * (rot_pointTemp[1] - rotation2DOriginY_) + rotation2DOriginY_ - rot_pointTemp[1];

						PointPointerLocal[pointI].component(vector::X) += rigidMotionAmplitudeX_*Foam::sin(2*3.1416*rigidMotionFrqX_*t);
						PointPointerLocal[pointI].component(vector::Y) += rigidMotionAmplitudeY_*Foam::sin(2*3.1416*rigidMotionFrqY_*t);
						PointPointerLocal[pointI].component(vector::Z) += rigidMotionAmplitudeZ_*Foam::sin(2*3.1416*rigidMotionFrqZ_*t);
					}
				}
		}
	}
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// *************************  FILE END  ************************************ //