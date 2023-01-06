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
 * @file aitken_inl.H
 * @author W. Liu
 * @date 24 February 2021
 * @brief Aitken Coupling Method (muiCouplingAitken class) of FSI Coupling utility
 */

#ifndef MUICOUPLINGAITKEN_H
#define MUICOUPLINGAITKEN_H

#include "mui.h"

#include <string>
#include <vector>
#include <map>
#include <assert.h>
#include <iostream>
#include <math.h>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <stdlib.h>
#include <mpi.h>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
namespace muiCoupling
{

/*---------------------------------------------------------------------------*\
                 Class muiCouplingAitken Declaration
\*---------------------------------------------------------------------------*/

    class muiCouplingAitken 
    {
        
    private:

        // Private data

            //-MPI Common World
            MPI_Comm world_;

            //-MPI Common Rank      
            int CppRank_;

            //-MPI Common Size      
            int CppSize_;

            //- Under relaxation factor of the coupling at present iteration
            double undRelxCpl_;

            //- Maximum under relaxation factor of the coupling
            double undRelxCplMax_;

            //- Under relaxation factor of the coupling at previous iteration
            mutable double undRelxCplPrev_;

            const double initundRelxCpl_;

            int pointSize_ = -999;

            std::vector<double> residualCoupling_;

            std::vector<double> residualDelta_;

            std::vector<double> residualCouplingPrev_;
            
            std::vector<double> deltaDisp_;
        
            std::vector<double> residualMagSq_;
        
            double residualMagSqSum_;
        
            double residualL2Norm_;
        
            double residualL2NormMax_;
            
            mutable std::map <int, int> pointMap_ ;

            std::map <int, int> :: iterator it_;

        // Private Member Functions

          // Math tools
            //- Calculation of point ID
            inline int calcpointN(int pointV)
            {
                it_ = pointMap_.find(pointV);
                if(it_ != pointMap_.end())
                {
                    return (( *it_ ).second);
                }
                else
                {
                    pointMap_.insert(std::pair<int, int>(pointV, (pointMap_.size()+1)));
                    return (pointMap_.size());
                }
            }
            //- Calculation of point ID
            int sign(double value)
            {
                return (value < 0) ? -1 : ((value > 0) ? 1 : 0);
            }

            //- Calculation of residual at present iteration                           
            inline void calcResidual(double fetchMUIx, 
                                    double fetchMUIy, 
                                    double fetchMUIz,
                                    double disPreX,
                                    double disPreY,
                                    double disPreZ,
                                    int pointN)
            {
                residualCoupling_[(3*(pointN-1))+0] = fetchMUIx - disPreX;
                residualCoupling_[(3*(pointN-1))+1] = fetchMUIy - disPreY;
                residualCoupling_[(3*(pointN-1))+2] = fetchMUIz - disPreZ;
            }                    

            //- Calculation of under relaxation factor at previous iteration
            inline void setUndRelxPrev()
            {
#ifdef USE_MUI
                if(Pstream::master())
                {
                    undRelxCplPrev_ = undRelxCpl_;
                }
                Foam::Pstream::scatter(undRelxCplPrev_);
#else
                if (CppRank_ == 0)
                {
                    undRelxCplPrev_ = undRelxCpl_;
                }
                MPI_Bcast(&undRelxCplPrev_, 1, MPI_DOUBLE, 0, world_);
#endif
            }

            //- Calculation of under relaxation factor at present iteration
            inline void calcUndRelx(int iterN, int curIterN)
            {
                assert(iterN >= 0);
                assert(curIterN >= 0);
                if (curIterN < 2)
                {
                    undRelxCpl_ = initundRelxCpl_;
                }
                else
                {
                    calResidualDelta();
                    double localNominator    = 0.0; 
                    double localDenominator  = 0.0;
                    double nominator    = 0.0; 
                    double denominator  = 0.0;
                    localNominator    = std::inner_product(begin(residualCouplingPrev_), 
                                                            end(residualCouplingPrev_), 
                                                            begin(residualDelta_), 0.0); 
                    localDenominator  = std::inner_product(begin(residualDelta_), 
                                                            end(residualDelta_), 
                                                            begin(residualDelta_), 0.0); 
#ifdef USE_MUI
                    nominator = localNominator;
                    denominator = localDenominator;
                    Foam::reduce(nominator, sumOp<scalar>());
                    Foam::reduce(denominator, sumOp<scalar>());
                    Foam::Pstream::scatter(nominator);
                    Foam::Pstream::scatter(denominator);
                    std::cout  << "iteration: " << iterN << "Current iteration: " << curIterN << " Rank: " << Pstream::myProcNo() 
                        << " nominator: " << nominator << " denominator: " << denominator 
                        << " undRelxCplPrev_: " << undRelxCplPrev_ << std::endl;
#else
                    MPI_Allreduce(&localNominator, &nominator, 1, MPI_DOUBLE, MPI_SUM, world_);
                    MPI_Allreduce(&localDenominator, &denominator, 1, MPI_DOUBLE, MPI_SUM, world_);
                    std::cout  << "iteration: " << iterN << "Current iteration: " << curIterN << " Rank: " << CppRank_ << " nominator: " 
                        << nominator << " denominator: " << denominator << " undRelxCplPrev_: " 
                        << undRelxCplPrev_ << std::endl;
#endif

#ifdef USE_MUI
                    if(Pstream::master())
                    {
                        if (denominator != 0.0 )
                        {
                            if (undRelxCplPrev_ != 0.0)
                            {
                                undRelxCpl_ = -undRelxCplPrev_ * (nominator/denominator);
                            }
                            else
                            {
                                undRelxCpl_ = -initundRelxCpl_ * (nominator/denominator);
                            }
                            // Constraint of the Aitken under relaxation factor
                            undRelxCpl_ = sign(undRelxCpl_) * std::min(std::abs(undRelxCpl_),undRelxCplMax_);

                            if (undRelxCpl_ < 0.0)
                            {
                                undRelxCpl_ = -undRelxCpl_;
                            }
                        }
                        else
                        {
                            assert(nominator == 0);
                            undRelxCpl_ = 0.0;
                        }
                        assert(!isnan( undRelxCpl_));
                        std::cout << "Aitken under relaxation factor is: " << undRelxCpl_ << std::endl;
                    }
                    Foam::Pstream::scatter(undRelxCpl_); 
                    std::cout << "undRelxCpl_: " << undRelxCpl_ << " rank: " << Pstream::myProcNo() << std::endl;
#else
                    if (CppRank_ == 0)
                    {
                        if (denominator != 0.0 )
                        {
                            if (undRelxCplPrev_ != 0.0)
                            {
                                undRelxCpl_ = -undRelxCplPrev_ * (nominator/denominator);
                            }
                            else
                            {
                                undRelxCpl_ = -initundRelxCpl_ * (nominator/denominator);
                            }
                            // Constraint of the Aitken under relaxation factor
                            undRelxCpl_ = sign(undRelxCpl_) * std::min(std::abs(undRelxCpl_),undRelxCplMax_);

                            if (undRelxCpl_ < 0.0)
                            {
                                undRelxCpl_ = -undRelxCpl_;
                            }
                        }
                        else
                        {
                            assert(nominator == 0);
                            undRelxCpl_ = 0.0;
                        }
                        assert(!isnan( undRelxCpl_));
                        std::cout << "Aitken under relaxation factor is: " << undRelxCpl_ << " Rank: " << CppRank_<< std::endl;
                    }
                    MPI_Bcast(&undRelxCpl_, 1, MPI_DOUBLE, 0, world_);
                    std::cout << "undRelxCpl_: " << undRelxCpl_ << std::endl;
#endif
                }
            }

            //- Calculation of delta displacement        
            inline void calcDeltaDisp()
            {
                for (int pointN=0; pointN<=(3*pointSize_-1); ++pointN)
                {    
                    deltaDisp_[pointN] = undRelxCpl_ * residualCoupling_[pointN];
                }
            }

            //- Calculation of delta residual at this iteration
            inline void calResidualDelta()
            {
                for (int pointN=0; pointN<=(3*pointSize_-1); ++pointN)
                {
                    residualDelta_[pointN] = residualCoupling_[pointN] - residualCouplingPrev_[pointN];
                }
            }

            //- Store residual at this iteration
            inline void setPrevResidual()
            {
                for (int pointN=0; pointN<=(3*pointSize_-1); ++pointN)
                {
                    residualCouplingPrev_[pointN] = residualCoupling_[pointN];
                }
            }

            //- Reset of residualMagSqSum_
            inline void resetResidualMagSqSum()
            {
                residualMagSqSum_ = 0.0;
            }

            //- Accumulation of residualMagSqSum_
            inline void accumResidualMagSqSum()
            {
                double localResidualMagSqSum_ = 0.0;
                for (int pointN=0; pointN<=(3*pointSize_-1); ++pointN)
                {
                    localResidualMagSqSum_ += std::pow(residualCoupling_[pointN], 2);
                }
#ifdef USE_MUI
                std::cout << "{MUI_Coupling} From rank " << Pstream::myProcNo() 
                    << "{MUI_Coupling} localResidualMagSqSum_: " << localResidualMagSqSum_ << std::endl;
                residualMagSqSum_ = localResidualMagSqSum_;
                Foam::reduce(residualMagSqSum_, sumOp<scalar>());
                Foam::Pstream::scatter(residualMagSqSum_);
#else
                std::cout << "{MUI_Coupling} From rank " << CppRank_ << " localResidualMagSqSum_: " 
                    << localResidualMagSqSum_ << std::endl;
                MPI_Allreduce(&localResidualMagSqSum_, &residualMagSqSum_, 1, MPI_DOUBLE, MPI_SUM, world_);
#endif
            }

            //- Calculation of residualL2Norm_         
            inline void calResidualL2Norm()
            {
#ifdef USE_MUI
                if(Pstream::master())
                {
                    residualL2Norm_ = std::sqrt(residualMagSqSum_);
                }
                Foam::Pstream::scatter(residualL2Norm_);
                std::cout << "residualMagSqSum_: " << residualMagSqSum_ << " residualL2Norm_: " 
                    << residualL2Norm_ << " rank: " << Pstream::myProcNo() << std::endl;
#else
                if (CppRank_ == 0)
                {
                    residualL2Norm_ = std::sqrt(residualMagSqSum_);
                }
                MPI_Bcast(&residualL2Norm_, 1, MPI_DOUBLE, 0, world_);
                std::cout << "residualMagSqSum_: " << residualMagSqSum_ 
                    << " residualL2Norm_: " << residualL2Norm_ << std::endl;
#endif
            }
            
            //- Calculation of residualL2NormMax_       
            inline void calResidualL2NormMax()
            {
#ifdef USE_MUI
                if(Pstream::master())
                {
                    residualL2NormMax_ = std::max(residualL2Norm_, residualL2NormMax_);
                }       
                Foam::Pstream::scatter(residualL2NormMax_);
                std::cout << "residualL2NormMax_: " << residualL2NormMax_ 
                    << " residualL2Norm_: " << residualL2Norm_  << std::endl;
#else
                if (CppRank_ == 0)
                {
                    residualL2NormMax_ = std::max(residualL2Norm_, residualL2NormMax_);
                }
                MPI_Bcast(&residualL2NormMax_, 1, MPI_DOUBLE, 0, world_);
                std::cout << "residualL2NormMax_: " << residualL2NormMax_ 
                    << " residualL2Norm_: " << residualL2Norm_  << std::endl;
#endif
            }

            //- Correction of residualL2Norm_ based on the maximum residualL2NormMax_        
            inline void setResidualL2Norm()
            {
#ifdef USE_MUI
                if(Pstream::master())
                {
                    if (residualL2NormMax_ <= 0.0 )
                    {
                        residualL2Norm_ = 1.0;
                    }
                    else
                    {
                        residualL2Norm_ /= residualL2NormMax_;
                    }
                }
                Foam::Pstream::scatter(residualL2Norm_);
#else
                if (CppRank_ == 0)
                {
                    if (residualL2NormMax_ <= 0.0 )
                    {
                        residualL2Norm_ = 1.0;
                    }
                    else
                    {
                        residualL2Norm_ /= residualL2NormMax_;
                    }
                }
                MPI_Bcast(&residualL2Norm_, 1, MPI_DOUBLE, 0, world_);
#endif
            }

            //- Logging        
            inline void logging()
            {
#ifdef USE_MUI
                if(Pstream::master())
                {
                    std::cout << std::scientific;
                    std::cout << std::endl;
                    std::cout << "{MUI_Coupling} No. of points is: " << pointSize() << std::endl;
                    std::cout << "{MUI_Coupling} Under-relaxation factor is: " << undRelxCpl() << std::endl;
                    std::cout << "{MUI_Coupling} Size of residualCoupling_ is: " << residualCoupling_.size() << std::endl;
                    std::cout << "{MUI_Coupling} Size of residualCouplingPrev_ is: " << residualCouplingPrev_.size() << std::endl;
                    std::cout << "{MUI_Coupling} Size of residualMagSq_ is: " << residualMagSq_.size() << std::endl;
                    std::cout << std::endl;
                    std::cout << "{MUI_Coupling} Residual of MUI Coupling is: " << residualL2Norm_ << std::endl;    
                    std::cout << "{MUI_Coupling} residualL2NormMax of MUI Coupling is: " << residualL2NormMax_ << std::endl;    
                    std::cout << "{MUI_Coupling} residualMagSqSum of MUI Coupling is: " << residualMagSqSum_ << std::endl;    
                }
#else
                if (CppRank_ == 0)
                {
                    std::cout << std::scientific;
                    std::cout << std::endl;
                    std::cout << "{MUI_Coupling} No. of points is: " << pointSize() << std::endl;
                    std::cout << "{MUI_Coupling} Under-relaxation factor is: " << undRelxCpl() << std::endl;
                    std::cout << "{MUI_Coupling} Size of residualCoupling_ is: " << residualCoupling_.size() << std::endl;
                    std::cout << "{MUI_Coupling} Size of residualCouplingPrev_ is: " << residualCouplingPrev_.size() << std::endl;
                    std::cout << "{MUI_Coupling} Size of residualMagSq_ is: " << residualMagSq_.size() << std::endl;
                    std::cout << std::endl;
                    std::cout << "{MUI_Coupling} Residual of MUI Coupling is: " << residualL2Norm_ << std::endl;    
                    std::cout << "{MUI_Coupling} residualL2NormMax of MUI Coupling is: " << residualL2NormMax_ << std::endl;    
                    std::cout << "{MUI_Coupling} residualMagSqSum of MUI Coupling is: " << residualMagSqSum_ << std::endl;    
                }
#endif
            }

    protected:

        // Protected member functions

    public:
        // Static data members
     
        // Constructors

            //- Constructor - default
            inline muiCouplingAitken
            (
                double initUndRelxCpl
            ):
            initundRelxCpl_(initUndRelxCpl)
            {
                std::cout << "{MUI_Coupling} C++ Aitken default constructor" 
                    << "initial under-relaxation factor: " << initundRelxCpl_ << std::endl;
            }

            //- Construct from components
            inline muiCouplingAitken
            (
                int pointSize,
                double initUndRelxCpl
            ): 
            pointSize_(pointSize),
            initundRelxCpl_(initUndRelxCpl)
            {
                std::cout << "{MUI_Coupling} C++ Aitken constructor: " 
                    <<" number of points: " << pointSize_ 
                    << " initial under-relaxation factor: " << initundRelxCpl_ <<std::endl;
            }

            //- Construct from components - overload
            inline muiCouplingAitken
            (
                int pointSize,
                double initUndRelxCpl,
                MPI_Comm *Cppworld
            ): 
            pointSize_(pointSize),
            initundRelxCpl_(initUndRelxCpl),
            world_(*Cppworld)
            {
                std::cout << "{MUI_Coupling} C++ Aitken constructor: " 
                    <<" number of points: " << pointSize_ 
                    << " initial under-relaxation factor: " << initundRelxCpl_ << std::endl;
                MPI_Comm_rank( world_, &CppRank_ );
                MPI_Comm_size( world_, &CppSize_ );
                std::cout << "{MUI_Coupling} Hello world from rank: " << CppRank_ 
                    << " out of " << CppSize_ << " processors. " <<std::endl;
            }


        // Selectors

        // Destructor - default

            inline ~muiCouplingAitken
            (
        
            )
            {
                std::cout << "{MUI_Coupling} C++ Aitken destructor" << std::endl;
            }


        // Member Functions
            // Access

                //- Return under relaxation factor of the coupling
                inline double undRelxCpl()
                {
                    return undRelxCpl_;
                }

                //- Return No. of points
                inline int pointSize() const
                {
                    return pointSize_;
                }

                //- Return square sum of the residual
                inline double residualMagSqSum() const
                {
                    return residualMagSqSum_;
                }

                //- Return maximum value of the residual L-2 Norm
                inline double residualL2NormMax() const
                {
                    return residualL2NormMax_;
                }

                //- Return the value of the residual L-2 Norm
                inline double residualL2Norm() const
                {
                    return residualL2Norm_;
                }

                //- Return x axis component of the delta displacement
                inline double getXDeltaDisp(int pointV)
                {
                    it_ = pointMap_.find(pointV);
                    assert (it_ != pointMap_.end());
                    return deltaDisp_[(3*(((*it_).second)-1))+0];
                }
     

                //- Return y axis component of the delta displacement
                inline double getYDeltaDisp(int pointV)
                {
                    it_ = pointMap_.find(pointV);
                    assert (it_ != pointMap_.end());
                    return deltaDisp_[(3*(((*it_).second)-1))+1];
                }

                //- Return z axis component of the delta displacement
                inline double getZDeltaDisp(int pointV)
                {
                    it_ = pointMap_.find(pointV);
                    assert (it_ != pointMap_.end());
                    return deltaDisp_[(3*(((*it_).second)-1))+2];
                }

              // Edit

                //- Initialize coupling method
                inline void initialize(int pointSize)
                {
                    pointSize_ = pointSize;
                    undRelxCplMax_ = 1.0;
                    residualMagSq_.resize(pointSize_, 0.0);
                    residualDelta_.resize((3*pointSize_), 0.0);
                    residualCoupling_.resize((3*pointSize_), 0.0);
                    residualCouplingPrev_.resize((3*pointSize_), 0.0);
                    deltaDisp_.resize((3*pointSize_), 0.0);
                    calcUndRelx(0,0);
                    setUndRelxPrev();
                    std::cout << "{MUI_Coupling} C++ Aitken initialize: " 
                        <<" number of points: " << pointSize_ << std::endl;
                }

                //- Initialize coupling method - overload
                inline void initialize(int pointSize,
                                       MPI_Comm *Cppworld)
                {
                    pointSize_ = pointSize;
                    world_ = *Cppworld;
                    undRelxCplMax_ = 1.0;
                    residualMagSq_.resize(pointSize_, 0.0);
                    residualDelta_.resize((3*pointSize_), 0.0);
                    residualCoupling_.resize((3*pointSize_), 0.0);
                    residualCouplingPrev_.resize((3*pointSize_), 0.0);
                    deltaDisp_.resize((3*pointSize_), 0.0);
                    calcUndRelx(0,0);
                    setUndRelxPrev();
                    std::cout << "{MUI_Coupling} C++ Aitken initialize: " 
                        <<" number of points: " << pointSize_<< std::endl;
                    MPI_Comm_rank( world_, &CppRank_ );
                    MPI_Comm_size( world_, &CppSize_ );
                    std::cout << "{MUI_Coupling} Hello world from rank: " << CppRank_ 
                        << " out of " << CppSize_ << " processors. " <<std::endl;
                }

                //- Initialize coupling method - overload
                inline void initialize()
                {
                    undRelxCplMax_ = 1.0;
                    residualMagSq_.resize(pointSize_, 0.0);
                    residualDelta_.resize((3*pointSize_), 0.0);
                    residualCoupling_.resize((3*pointSize_), 0.0);
                    residualCouplingPrev_.resize((3*pointSize_), 0.0);
                    deltaDisp_.resize((3*pointSize_), 0.0);
                    calcUndRelx(0,0);
                    setUndRelxPrev();
                }

                //- Collection of Residual at this iteration            
                inline void collectResidual(double fetchMUIx, 
                                            double fetchMUIy, 
                                            double fetchMUIz,
                                            double disPreX,
                                            double disPreY,
                                            double disPreZ,
                                            int pointV)
                {
                    //calculate point ID
                    int pointN = -999;
                    pointN = calcpointN(pointV); 
                    assert (pointN >= 0);
                    calcResidual(   fetchMUIx, 
                                    fetchMUIy, 
                                    fetchMUIz,
                                    disPreX,
                                    disPreY,
                                    disPreZ,
                                    pointN);
                }
                                                            
                //- Collection of coupling process at this iteration
                inline void process(int iterN, int curIterN)
                {
                    calcUndRelx(iterN,curIterN);
                    calcDeltaDisp();
                    setPrevResidual();
                    accumResidualMagSqSum();
                    calResidualL2Norm();
                    calResidualL2NormMax();
                    setResidualL2Norm();
                    setUndRelxPrev();
                    logging();
                    resetResidualMagSqSum();
                }
    };

    // Global Functions

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace muiCoupling

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif // End MUICOUPLINGAITKEN_H

// ************************************************************************* //