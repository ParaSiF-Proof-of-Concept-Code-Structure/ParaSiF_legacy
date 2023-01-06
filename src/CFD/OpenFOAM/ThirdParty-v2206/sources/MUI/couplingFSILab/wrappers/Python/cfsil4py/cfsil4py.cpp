/*****************************************************************************
* Multiscale Universal Interface Code Coupling Library                       *
*                                                                            *
* Copyright (C) 2019 Y. H. Tang, S. Kudo, X. Bian, Z. Li, G. E. Karniadakis  *
*                                                                            *
* This software is jointly licensed under the Apache License, Version 2.0    *
* and the GNU General Public License version 3, you may use it according     *
* to either.                                                                 *
*                                                                            *
* ** Apache License, version 2.0 **                                          *
*                                                                            *
* Licensed under the Apache License, Version 2.0 (the "License");            *
* you may not use this file except in compliance with the License.           *
* You may obtain a copy of the License at                                    *
*                                                                            *
* http://www.apache.org/licenses/LICENSE-2.0                                 *
*                                                                            *
* Unless required by applicable law or agreed to in writing, software        *
* distributed under the License is distributed on an "AS IS" BASIS,          *
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
* See the License for the specific language governing permissions and        *
* limitations under the License.                                             *
*                                                                            *
* ** GNU General Public License, version 3 **                                *
*                                                                            *
* This program is free software: you can redistribute it and/or modify       *
* it under the terms of the GNU General Public License as published by       *
* the Free Software Foundation, either version 3 of the License, or          *
* (at your option) any later version.                                        *
*                                                                            *
* This program is distributed in the hope that it will be useful,            *
* but WITHOUT ANY WARRANTY; without even the implied warranty of             *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
* GNU General Public License for more details.                               *
*                                                                            *
* You should have received a copy of the GNU General Public License          *
* along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
*****************************************************************************/

/**
 * @file cfsil4py.cpp
 * @author W. Liu
 * @date 24 February 2021
 * @brief Python wrapper (by using pybind11) on Interface Quasi-Newton with 
 * Inverse Jacobian from Least Squares model (IQN-ILS) Coupling Method 
 * (muiCouplingIQNILS class) of FSI Coupling utility
 */

#include <pybind11/pybind11.h>
#include <mpi.h>
#include <mpi4py/mpi4py.h>
#include "../../../iqnils_inl.H"

namespace py = pybind11;

PYBIND11_MODULE(cfsil4py_mod, m) {
    // optional module docstring
    m.doc() = "python bind on iqnils FSI coupling utility on MUI coupling library";

    // define Global Functions

    // bindings to class
    py::class_<muiCoupling::muiCouplingIQNILS>(m, "muiCouplingIQNILS")
        .def(py::init<double>())
         .def(py::init(
			[](int pointSize, double initUndRelxCpl, double undRelxCplMax, int aitkenIterationN, bool globalAlphaInput)
			{return new muiCoupling::muiCouplingIQNILS(pointSize, initUndRelxCpl, undRelxCplMax, aitkenIterationN, globalAlphaInput);}))
         .def(py::init(
			[](int pointSize, double initUndRelxCpl, pybind11::handle const& pyHdl, double undRelxCplMax, int aitkenIterationN, bool globalAlphaInput){
			PyObject *py_src = pyHdl.ptr();
			MPI_Comm *comm_p = PyMPIComm_Get(py_src);
			auto ric_mpiComm = reinterpret_cast<MPI_Comm>(comm_p);			
			return new muiCoupling::muiCouplingIQNILS(pointSize, initUndRelxCpl, &ric_mpiComm, undRelxCplMax, aitkenIterationN, globalAlphaInput);}))
         .def(py::init(
			[](int pointSize, double initUndRelxCpl, std::vector<int> &localRankVec, int sizeAfterSplit, double undRelxCplMax, int aitkenIterationN, bool globalAlphaInput)
			{return new muiCoupling::muiCouplingIQNILS(pointSize, initUndRelxCpl, localRankVec, sizeAfterSplit, undRelxCplMax, aitkenIterationN, globalAlphaInput);}))
        .def("undRelxCpl", &muiCoupling::muiCouplingIQNILS::undRelxCpl)
        .def("pointSize", &muiCoupling::muiCouplingIQNILS::pointSize)
        .def("residualMagSqSum", &muiCoupling::muiCouplingIQNILS::residualMagSqSum)
        .def("residualL2NormMax", &muiCoupling::muiCouplingIQNILS::residualL2NormMax)
        .def("residualL2Norm", &muiCoupling::muiCouplingIQNILS::residualL2Norm)
        .def("getXDeltaDisp", &muiCoupling::muiCouplingIQNILS::getXDeltaDisp)
        .def("getYDeltaDisp", &muiCoupling::muiCouplingIQNILS::getYDeltaDisp)
        .def("getZDeltaDisp", &muiCoupling::muiCouplingIQNILS::getZDeltaDisp)
        .def("initialize", [](muiCoupling::muiCouplingIQNILS &initialize_obj){return initialize_obj.initialize();})
        .def("initialize",
			[](muiCoupling::muiCouplingIQNILS &initialize_obj, int pointSize, double undRelxCplMax, int aitkenIterationN, bool globalAlphaInput)
			{ return initialize_obj.initialize(pointSize, undRelxCplMax, aitkenIterationN, globalAlphaInput);})
        .def("initialize",
			[](muiCoupling::muiCouplingIQNILS &initialize_obj, int pointSize, py::handle pyHdl, double undRelxCplMax, int aitkenIterationN, bool globalAlphaInput){
			PyObject *py_src = pyHdl.ptr();
			MPI_Comm *comm_p = PyMPIComm_Get(py_src);
			auto ric_mpiComm = reinterpret_cast<MPI_Comm>(comm_p);		
			return initialize_obj.initialize(pointSize, &ric_mpiComm, undRelxCplMax, aitkenIterationN, globalAlphaInput);})
		.def("collectResidual", &muiCoupling::muiCouplingIQNILS::collectResidual)
		.def("process", &muiCoupling::muiCouplingIQNILS::process);
}