#include <iostream>
#include <fstream>
#include <cmath>
#include "mui.h"

using namespace std;
typedef std::unique_ptr<mui::uniface1d> MuiUnifacePtr1d;
MuiUnifacePtr1d interface;


double k = 20;  // Spring constant
double c = 1.0;  // Damping coefficient
double m = 0.0750;  // Mass

// Differential equation for the spring damping system
double acc(double t, double x, double v, double f) {
    return (f-k*x - c*v)/m;
}

// Fourth-order Runge-Kutta method for solving differential equations
void runge_kutta(double &t, double &x, double &v, double &f, double dt) {
    double k1x = v;
    double k1v = acc(t, x, v,f);
    double k2x = v + 0.5*dt*k1v;
    double k2v = acc(t + 0.5*dt, x + 0.5*dt*k1x, v + 0.5*dt*k1v,f);
    double k3x = v + 0.5*dt*k2v;
    double k3v = acc(t + 0.5*dt, x + 0.5*dt*k2x, v + 0.5*dt*k2v,f);
    double k4x = v + dt*k3v;
    double k4v = acc(t + dt, x + dt*k3x, v + dt*k3v,f);
    x += (dt/6.0)*(k1x + 2.0*k2x + 2.0*k3x + k4x);
    v += (dt/6.0)*(k1v + 2.0*k2v + 2.0*k3v + k4v);
    t += dt;
}

int main(int argc , char *argv[]) {

    int rank, size, i;
    int sum = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm  world = mui::mpi_split_by_app();
    MPI_Comm_rank(world, &rank);
    MPI_Comm_size(world, &size);
    interface.reset(new mui::uniface1d("mpi://ping/ifs" ));
    mui::sampler_exact1d<double> spatial_sampler;
    mui::temporal_sampler_exact1d chrono_sampler;
    mui::point1d push_point;
    mui::point1d fetch_point;


    double x0 = -0.00001;  // Initial displacement
    double xf = 0;
    double v0 = 0.0;  // Initial velocity
    double t = 0.0;   // Initial time
    double dt = 0.0001;  // Time step
    double f = -0;
    ofstream outFile("simulation_results.txt");

    cout << "Time\tDisplacement\tVelocity" << endl;

    double x = x0;
    double v = v0;
    while (t <= 2.0) {  // Solve for 10 seconds
        // cout << t << "\t" << x << "\t" << v << endl;
        outFile << t << "\t" << x << "\t" << v << endl;


        double TotalForce= interface->fetch( "TotalForce_y", 0, t, spatial_sampler, chrono_sampler );

        cout << " =============== springDamping get at time " << t << "  Force = " << TotalForce <<endl;


        runge_kutta(t, x, v,TotalForce, dt);
        interface->push( "disp_x", push_point, 0.0 );
        interface->push( "disp_y", push_point, x );
        interface->push( "disp_z", push_point, 0.0 );

        interface->push( "angle_x", push_point, 0.0 );
        interface->push( "angle_y", push_point, 0.0 );
        interface->push( "angle_z", push_point, 0.0 );
        cout << " =============== springDamping sent at time " << t << "  Diss = " << x <<endl;
        interface->commit( t );
    }

    return 0;
}
