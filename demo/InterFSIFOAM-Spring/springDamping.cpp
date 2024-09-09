#include <iostream>
#include <fstream>
#include <cmath>
#include "mui.h"
#include "muiconfig.h"

using namespace std;
// typedef std::unique_ptr<mui::uniface1d> MuiUnifacePtr1d;
// MuiUnifacePtr1d interface;
typedef std::unique_ptr<mui::uniface<mui::mui_config>> MuiUnifacePtr3d;
std::vector<MuiUnifacePtr3d> ifs;


double k = 63000;  // Spring constant
double c = 5.0;  // Damping coefficient
double m = 18.4;  // Mass

// Differential equation for the spring damping system
double acc( double x, double v, double f) {
    return (f-k*x - c*v)/m;
}

// Fourth-order Runge-Kutta method for solving differential equations
void runge_kutta( double &x0, double &v0,double &x, double &v, double &f, double dt) {
    double k1x = v;
    double k1v = acc( x, v,f);
    double k2x = v + 0.5*dt*k1v;
    double k2v = acc( x + 0.5*dt*k1x, v + 0.5*dt*k1v,f);
    double k3x = v + 0.5*dt*k2v;
    double k3v = acc( x + 0.5*dt*k2x, v + 0.5*dt*k2v,f);
    double k4x = v + dt*k3v;
    double k4v = acc( x + dt*k3x, v + dt*k3v,f);
    x = x0+(dt/6.0)*(k1x + 2.0*k2x + 2.0*k3x + k4x);
    v = v0+(dt/6.0)*(k1v + 2.0*k2v + 2.0*k3v + k4v);
    
}

int main(int argc , char *argv[]) {

    int rank, size, i;
    int sum = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm  world = mui::mpi_split_by_app();
    MPI_Comm_rank(world, &rank);
    MPI_Comm_size(world, &size);

    std::vector<std::string> interfaces;
	std::string domainName="PUSHER_FETCHER_1";
	std::string appName="threeDInterface0";
    interfaces.emplace_back(appName);
    ifs=mui::create_uniface<mui::mui_config>( domainName, interfaces );

    // interface.reset(new mui::uniface1d("mpi://ping/threeDInterface0" ));
    mui::sampler_exact<mui::mui_config> spatial_sampler;
    mui::temporal_sampler_exact<mui::mui_config>  chrono_sampler;
    mui::point3d locf( 0.0, 0.0, 0.0 );
    mui::point1d fetch_point;


    double x0 = -0.08;  // Initial displacement
    double xf = 0;
    double v0 = 0.0;  // Initial velocity
    double t = 0.0;   // Initial time
    double dt = 0.001;  // Time step
    int nSubIter=5;
    double f = -0;
    ofstream outFile("simulation_results.txt");

    cout << "Time\tDisplacement\tVelocity" << endl;
    double x00=x0;
    double x = x0;
    double v = v0;
    int iterCounter;
    t= dt;
    while (t <= 10.0) {  // Solve for 10 seconds
    outFile << t << "\t" << x << "\t" << v << endl;
    for ( int iter = 1; iter <= nSubIter; ++iter ) {
    iterCounter++;
        // cout << t << "\t" << x << "\t" << v << endl;
        


        double TotalForce= ifs[0]->fetch( "forceZ", locf,  t,iter , spatial_sampler, chrono_sampler );

        


        runge_kutta( x0, v0,x, v,TotalForce, dt);
        ifs[0]->push( "dispX", locf, 0.0 );
        ifs[0]->push( "dispY", locf, 0.0 );
        ifs[0]->push( "dispZ", locf, x00-x );

        ifs[0]->push( "angleX", locf, 0.0 );
        ifs[0]->push( "angleY", locf, 0.0 );
        ifs[0]->push( "angleZ", locf, 0.0 );
        cout << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"<<endl;
	cout << " =============== springDamping get at time " << t <<" sub iter "<< iter <<"  Force = " << TotalForce <<endl;
        cout << " =============== springDamping sent at time " << t <<" sub iter "<< iter << "  Diss = " << x00-x <<endl;
        cout << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"<<endl;
        ifs[0]->commit( t,iter );
    }
    t += dt;
    x0=x;v0=v;
    }
    MPI_Finalize();

    return 0;
}
