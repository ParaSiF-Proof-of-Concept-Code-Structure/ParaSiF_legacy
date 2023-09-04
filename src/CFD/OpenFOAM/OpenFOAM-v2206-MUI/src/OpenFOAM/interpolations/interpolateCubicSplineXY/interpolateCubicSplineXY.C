/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011 OpenFOAM Foundation
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

    You should have received a_1 copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "interpolateCubicSplineXY.H"
#include "primitiveFields.H"
#include <iomanip>
#include <fstream>
#include "cartesianCS.H"
#include "addToRunTimeSelectionTable.H"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


Foam::scalarField  Foam::thomasAlgorithm
(
     scalarField a_1,
     scalarField a_2,
     scalarField a_3,
     scalarField b
)
{
    label n = b.size();
    scalarField x(n);

    for(int i = 1; i < n; i++) {
        scalar m = a_1[i-1]/a_2[i-1];
        a_2[i] = a_2[i] - m * a_3[i - 1];
        b[i] = b[i] - m * b[i - 1];
    }

    x[n - 1] = b[n - 1]/a_2[n - 1];

    for(int i = n - 2; i >= 0; i--) {
        x[i] = (b[i] - a_3[i] * x[i + 1]) / a_2[i];
    }
    // return x;
    return x;
}
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
template<class Type>
Foam::Field<Type>  Foam::thomasAlgorithm_Field
(
     scalarField& a_1,
     scalarField& a_2,
     scalarField& a_3,
     Field<Type>& b
)
{
    label n_1 = b.size();
    label n_2 = b[0].size();
    label n_3 = b[0][0].size();
    scalarField bb(n_1),B(n_1);
    Field<Type> D(b.size());

    for (int j=0; j<n_2; j++)
    {
        for (int k=0; k<n_3; k++)
        {
            for (int i=0; i<n_1; i++)
            {
                bb[i]=b[i][j][k];
            }

            B=thomasAlgorithm(a_1,a_2,a_3,bb);
            for (int i=0; i<n_1; i++)
            {
            D[i][j][k] = B[i];
            }
        }
    }
    



    return D;
}
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //



template<class Type>
Foam::Field<Type> Foam::interpolateCubicSplineXY
(
    const scalarField& xNew,
    const scalarField& xOld,
    const Field<Type>& yOld
)
{
    Field<Type> yNew(xNew.size());

    forAll(xNew, i)
    {
        yNew[i] = interpolateCubicSplineXY(xNew[i], xOld, yOld);
    }

    return yNew;
}


template<class Type>
Type Foam::interpolateCubicSplineXY
(
    const scalar x,
    const scalarField& xOld,
    const Field<Type>& yOld
)
{
    Info << "Using interpolate Cubic-Spline interpolation for the tabulated data" <<endl;
    label n = xOld.size();
    Vector2D<Vector<double>> zeroVec(Vector<double>(0, 0, 0),Vector<double>(0, 0, 0));
    scalarField h(n), a_1(n), a_2(n) , a_3(n);
    Field<Type>  A(yOld.size(),zeroVec),B(yOld.size(),zeroVec),C(yOld.size(),zeroVec);
    Field<Type> z(yOld.size(),zeroVec), b(yOld.size(),zeroVec),D(yOld.size(),zeroVec);
    for (int i =0 ; i<n; i++)
    {
        h[i]=0.0;
        a_1[i]=0.0;  a_2[i]=0.0;  a_3[i]=0.0;
    }


    // early exit if out of bounds or only one value
    if (n == 1 || x < xOld[0])
    {
        return yOld[0];
    }
    if (x > xOld[n - 1])
    {
        return yOld[n - 1];
    }


    // linear interpolation if only two values
    if (n == 2)
    {
        return (x - xOld[0])/(xOld[1] - xOld[0])*(yOld[1] - yOld[0]) + yOld[0];
    }

    for (int i =0 ; i<n-1; i++)
    {
        h[i]=xOld[i+1]-xOld[i];
        z[i]=yOld[i+1]-yOld[i];
    }

    
    for (int i =1 ; i<n-1; i++)
    {
        a_1[i-1] =  h[i-1];
        a_2[i] = 2*(h[i-1]+h[i]);
        a_3[i] = h[i];
        b[i] = 3*(z[i]/h[i]-z[i-1]/h[i-1]);
    }
    a_2[0] = 1.0 ; a_2[n-1]=1.0; // b[0] = 0.0 ;b[n-1] = 0.0; as per the intialisation

    D=yOld;    //  XXXXXXXXXXXXXXXXx

    B= thomasAlgorithm_Field(a_1,a_2,a_3,b);

    for (int i =0 ; i<n-1; i++)
    {
        A[i]=(B[i+1]-B[i])/3/h[i];
        C[i]=(z[i]-(A[i]*h[i]+B[i])*h[i]*h[i])/h[i];
    }



    // find bounding knots
    label hi = 0;
    while (hi < n && xOld[hi] < x)
    {
        hi++;
    }

    label lo = hi - 1;



    const Type& y1 = yOld[lo];
    const Type& y2 = yOld[hi];

    Type y0;
    if (lo == 0)
    {
        y0 = 2*y1 - y2;
    }
    else
    {
        y0 = yOld[lo - 1];
    }

    Type y3;
    if (hi + 1 == n)
    {
        y3 = 2*y2 - y1;
    }
    else
    {
        y3 = yOld[hi + 1];
    }

    // weighting
    scalar mu = (x - xOld[lo]);
    auto yNew=A[lo]*pow3(mu)+B[lo]*sqr(mu)+C[lo]*mu+D[lo];

    return yNew;
}

// ************************************************************************* //
