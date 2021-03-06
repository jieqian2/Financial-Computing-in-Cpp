//
//  main.cpp
//  prog10-euro options pricing LPs
//
//  Created by Jay Chen on 11/19/19.
//  Copyright © 2019 Jay Chen. All rights reserved.
//
// inspired by  European Option Pricing via LPs.cpp on Compass

#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <cstdlib>
//#include "normdist.h"          // this defines the normal distribution from Odegaard's files
#include "lp_lib.h"

using namespace std;

// Declaring some global variables to make my life easy

double up_factor, risk_free_rate, strike_price, R;
double initial_stock_price, expiration_time, volatility;
int **call_memoization;
int **put_memoization;
int no_of_divisions;
lprec *lp_call_memo, *lp_put_memo;
lprec *lp_call, *lp_put;
double max(double a, double b) {
    return (b < a )? a:b;
}

// Odegaard's "Curve Fitting" of a curve into the unit normal CDF
double N(const double& z) {
    if (z > 6.0) { return 1.0; }; // this guards against overflow
    if (z < -6.0) { return 0.0; };
    double b1 = 0.31938153;
    double b2 = -0.356563782;
    double b3 = 1.781477937;
    double b4 = -1.821255978;
    double b5 = 1.330274429;
    double p = 0.2316419;
    double c2 = 0.3989423;
    double a=fabs(z);
    double t = 1.0/(1.0+a*p);
    double b = c2*exp((-z)*(z/2.0));
    double n = ((((b5*t+b4)*t+b3)*t+b2)*t+b1)*t;
    n = 1.0-b*n;
    if ( z < 0.0 ) n = 1.0 - n;
    return n;
};

// Odegaard's Black-Scholes Put C++ Code
double option_price_put_black_scholes(const double& S,      // spot price
                                      const double& K,      // Strike (exercise) price,
                                      const double& r,      // interest rate
                                      const double& sigma,  // volatility
                                      const double& time){
    double time_sqrt = sqrt(time);
    double d1 = (log(S/K)+r*time)/(sigma*time_sqrt) + 0.5*sigma*time_sqrt;
    double d2 = d1-(sigma*time_sqrt);
    return K*exp(-r*time)*N(-d2) - S*N(-d1);
};

// Odegaard's Black-Scholes Call C++ Code
double option_price_call_black_scholes(const double& S,       // spot (underlying) price
                                       const double& K,       // strike (exercise) price,
                                       const double& r,       // interest rate
                                       const double& sigma,   // volatility
                                       const double& time) {  // time to maturity
    double time_sqrt = sqrt(time);
    double d1 = (log(S/K)+r*time)/(sigma*time_sqrt)+0.5*sigma*time_sqrt;
    double d2 = d1-(sigma*time_sqrt);
    return S*N(d1) - K*exp(-r*time)*N(d2);
};



// This recursive routine writes the relevant constraints for the LP
// formulation for the Call Option with memoization
void create_LP_for_european_call_option(int k, int i)
{
    if (-1 == call_memoization[k][(i+k)/2])
    {
        if (k == no_of_divisions-1)
        {
            // creating the constraint row & initializing with all zeros
            double row1[no_of_divisions*(no_of_divisions+1)+1], row2[no_of_divisions*(no_of_divisions+1)+1];
            for (int j = 0; j < no_of_divisions*(no_of_divisions+1)+1; j++) {
                row1[j] = 0.0;
                row2[j] = 0.0;
            }
            
            // make relevant non-zero entries on this row
            // the x- and y-value's location on the constraint-row for (k,i) is
            // k^2 + k + (i+k) + 1 and k^2 + k + (i+k) + 2... keep in mind the #entries in
            // each row must be 1 + #variables (as per lp_solve API's requirement)
            row1[(k*k) + k + (i+k) +1] = initial_stock_price*pow(up_factor, ((float) i+1));
            row1[(k*k) + k + (i+k) + 2] = -R;
            row2[(k*k) + k + (i+k) +1] = initial_stock_price*pow(up_factor, ((float) i-1));
            row2[(k*k) + k + (i+k) + 2] = -R;
            
            add_constraint(lp_call_memo, row1, GE, max(0.0, (initial_stock_price*pow(up_factor, ((float) i+1))) - strike_price));
            add_constraint(lp_call_memo, row2, GE, max(0.0, (initial_stock_price*pow(up_factor, ((float) i-1))) - strike_price));
        }
        else {
            // creating two constraint rows & initializing with all zeros
            double row1[no_of_divisions*(no_of_divisions+1)+1], row2[no_of_divisions*(no_of_divisions+1)+1];
            for (int j = 0; j < no_of_divisions*(no_of_divisions+1)+1; j++) {
                row1[j] = 0.0;
                row2[j] = 0.0;
            }
            
            // make relevant non-zero entries on this row
            row1[(k*k) + k + (i+k) +1] = initial_stock_price*pow(up_factor, ((float) i+1));
            row1[(k*k) + k + (i+k) + 2] = -R;
            row2[(k*k) + k + (i+k) +1] = initial_stock_price*pow(up_factor, ((float) i-1));
            row2[(k*k) + k + (i+k) + 2] = -R;
            
            row1[((k+1)*(k+1)) + (k+1) + (i+1+k+1) + 1] = -initial_stock_price*pow(up_factor, ((float) i+1));
            row1[((k+1)*(k+1)) + (k+1) + (i+1+k+1) + 2] = 1;
            row2[((k+1)*(k+1)) + (k+1) + (i-1+k+1) + 1] = -initial_stock_price*pow(up_factor, ((float) i-1));
            row2[((k+1)*(k+1)) + (k+1) + (i-1+k+1) + 2] = 1;
            
            add_constraint(lp_call_memo, row1, GE, 0);
            add_constraint(lp_call_memo, row2, GE, 0);
            
            create_LP_for_european_call_option(k+1,i+1);
            create_LP_for_european_call_option(k+1,i-1);
        }
        call_memoization[k][(i+k)/2]=1;
    }
}


// This recursive routine writes the relevant constraints for the LP
// formulation for the Put Option with memoization
void create_LP_for_european_put_option(int k, int i)
{
    if (-1 == put_memoization[k][(i+k)/2])
    {
        if (k == no_of_divisions-1)
        {
            // creating the constraint row & initializing with all zeros
            double row1[no_of_divisions*(no_of_divisions+1)+1], row2[no_of_divisions*(no_of_divisions+1)+1];
            for (int j = 0; j < no_of_divisions*(no_of_divisions+1)+1; j++) {
                row1[j] = 0.0;
                row2[j] = 0.0;
            }
            
            // make relevant non-zero entries on this row
            // the x- and y-value's location on the constraint-row for (k,i) is
            // k^2 + k + (i+k) + 1 and k^2 + k + (i+k) + 2... keep in mind the #entries in
            // each row must be 1 + #variables (as per lp_solve API's requirement)
            row1[(k*k) + k + (i+k) +1] = -initial_stock_price*pow(up_factor, ((float) i+1));
            row1[(k*k) + k + (i+k) + 2] = R;
            row2[(k*k) + k + (i+k) +1] = -initial_stock_price*pow(up_factor, ((float) i-1));
            row2[(k*k) + k + (i+k) + 2] = R;
            
            add_constraint(lp_put_memo, row1, GE, max(0.0, (strike_price - initial_stock_price*pow(up_factor, ((float) i+1)))));
            add_constraint(lp_put_memo, row2, GE, max(0.0, (strike_price - initial_stock_price*pow(up_factor, ((float) i-1)))));
        }
        else
        {
            // creating two constraint rows & initializing with all zeros
            double row1[no_of_divisions*(no_of_divisions+1)+1], row2[no_of_divisions*(no_of_divisions+1)+1];
            for (int j = 0; j < no_of_divisions*(no_of_divisions+1)+1; j++)
            {
                row1[j] = 0.0;
                row2[j] = 0.0;
            }
            
            // make relevant non-zero entries on this row
            row1[(k*k) + k + (i+k) +1] = -initial_stock_price*pow(up_factor, ((float) i+1));
            row1[(k*k) + k + (i+k) + 2] = R;
            row2[(k*k) + k + (i+k) +1] = -initial_stock_price*pow(up_factor, ((float) i-1));
            row2[(k*k) + k + (i+k) + 2] = R;
            
            row1[((k+1)*(k+1)) + (k+1) + (i+1+k+1) + 1] = initial_stock_price*pow(up_factor, ((float) i+1));
            row1[((k+1)*(k+1)) + (k+1) + (i+1+k+1) + 2] = -1;
            row2[((k+1)*(k+1)) + (k+1) + (i-1+k+1) + 1] = initial_stock_price*pow(up_factor, ((float) i-1));
            row2[((k+1)*(k+1)) + (k+1) + (i-1+k+1) + 2] = -1;
            
            add_constraint(lp_put_memo, row1, GE, 0);
            add_constraint(lp_put_memo, row2, GE, 0);
            
            create_LP_for_european_put_option(k+1,i+1);
            create_LP_for_european_put_option(k+1,i-1);
        }
        put_memoization[k][(i+k)/2]=1;
    }
}

// This recursive routine writes the relevant constraints for the LP
// formulation for the Call Option without memoization
void create_LP_for_european_call_option_without_memo(int k, int i)
{
    
    {
        if (k == no_of_divisions-1)
        {
            // creating the constraint row & initializing with all zeros
            double row1[no_of_divisions*(no_of_divisions+1)+1], row2[no_of_divisions*(no_of_divisions+1)+1];
            for (int j = 0; j < no_of_divisions*(no_of_divisions+1)+1; j++) {
                row1[j] = 0.0;
                row2[j] = 0.0;
            }
            
            // make relevant non-zero entries on this row
            // the x- and y-value's location on the constraint-row for (k,i) is
            // k^2 + k + (i+k) + 1 and k^2 + k + (i+k) + 2... keep in mind the #entries in
            // each row must be 1 + #variables (as per lp_solve API's requirement)
            row1[(k*k) + k + (i+k) +1] = initial_stock_price*pow(up_factor, ((float) i+1));
            row1[(k*k) + k + (i+k) + 2] = -R;
            row2[(k*k) + k + (i+k) +1] = initial_stock_price*pow(up_factor, ((float) i-1));
            row2[(k*k) + k + (i+k) + 2] = -R;
            
            add_constraint(lp_call, row1, GE, max(0.0, (initial_stock_price*pow(up_factor, ((float) i+1))) - strike_price));
            add_constraint(lp_call, row2, GE, max(0.0, (initial_stock_price*pow(up_factor, ((float) i-1))) - strike_price));
        }
        else {
            // creating two constraint rows & initializing with all zeros
            double row1[no_of_divisions*(no_of_divisions+1)+1], row2[no_of_divisions*(no_of_divisions+1)+1];
            for (int j = 0; j < no_of_divisions*(no_of_divisions+1)+1; j++) {
                row1[j] = 0.0;
                row2[j] = 0.0;
            }
            
            // make relevant non-zero entries on this row
            row1[(k*k) + k + (i+k) +1] = initial_stock_price*pow(up_factor, ((float) i+1));
            row1[(k*k) + k + (i+k) + 2] = -R;
            row2[(k*k) + k + (i+k) +1] = initial_stock_price*pow(up_factor, ((float) i-1));
            row2[(k*k) + k + (i+k) + 2] = -R;
            
            row1[((k+1)*(k+1)) + (k+1) + (i+1+k+1) + 1] = -initial_stock_price*pow(up_factor, ((float) i+1));
            row1[((k+1)*(k+1)) + (k+1) + (i+1+k+1) + 2] = 1;
            row2[((k+1)*(k+1)) + (k+1) + (i-1+k+1) + 1] = -initial_stock_price*pow(up_factor, ((float) i-1));
            row2[((k+1)*(k+1)) + (k+1) + (i-1+k+1) + 2] = 1;
            
            add_constraint(lp_call, row1, GE, 0);
            add_constraint(lp_call, row2, GE, 0);
            
            create_LP_for_european_call_option_without_memo(k+1,i+1);
            create_LP_for_european_call_option_without_memo(k+1,i-1);
        }
    }
}


// This recursive routine writes the relevant constraints for the LP
// formulation for the Put Option without memoization
void create_LP_for_european_put_option_without_memo(int k, int i)
{
    
    {
        if (k == no_of_divisions-1)
        {
            // creating the constraint row & initializing with all zeros
            double row1[no_of_divisions*(no_of_divisions+1)+1], row2[no_of_divisions*(no_of_divisions+1)+1];
            for (int j = 0; j < no_of_divisions*(no_of_divisions+1)+1; j++) {
                row1[j] = 0.0;
                row2[j] = 0.0;
            }
            
            // make relevant non-zero entries on this row
            // the x- and y-value's location on the constraint-row for (k,i) is
            // k^2 + k + (i+k) + 1 and k^2 + k + (i+k) + 2... keep in mind the #entries in
            // each row must be 1 + #variables (as per lp_solve API's requirement)
            row1[(k*k) + k + (i+k) +1] = -initial_stock_price*pow(up_factor, ((float) i+1));
            row1[(k*k) + k + (i+k) + 2] = R;
            row2[(k*k) + k + (i+k) +1] = -initial_stock_price*pow(up_factor, ((float) i-1));
            row2[(k*k) + k + (i+k) + 2] = R;
            
            add_constraint(lp_put, row1, GE, max(0.0, (strike_price - initial_stock_price*pow(up_factor, ((float) i+1)))));
            add_constraint(lp_put, row2, GE, max(0.0, (strike_price - initial_stock_price*pow(up_factor, ((float) i-1)))));
        }
        else
        {
            // creating two constraint rows & initializing with all zeros
            double row1[no_of_divisions*(no_of_divisions+1)+1], row2[no_of_divisions*(no_of_divisions+1)+1];
            for (int j = 0; j < no_of_divisions*(no_of_divisions+1)+1; j++)
            {
                row1[j] = 0.0;
                row2[j] = 0.0;
            }
            
            // make relevant non-zero entries on this row
            row1[(k*k) + k + (i+k) +1] = -initial_stock_price*pow(up_factor, ((float) i+1));
            row1[(k*k) + k + (i+k) + 2] = R;
            row2[(k*k) + k + (i+k) +1] = -initial_stock_price*pow(up_factor, ((float) i-1));
            row2[(k*k) + k + (i+k) + 2] = R;
            
            row1[((k+1)*(k+1)) + (k+1) + (i+1+k+1) + 1] = initial_stock_price*pow(up_factor, ((float) i+1));
            row1[((k+1)*(k+1)) + (k+1) + (i+1+k+1) + 2] = -1;
            row2[((k+1)*(k+1)) + (k+1) + (i-1+k+1) + 1] = initial_stock_price*pow(up_factor, ((float) i-1));
            row2[((k+1)*(k+1)) + (k+1) + (i-1+k+1) + 2] = -1;
            
            add_constraint(lp_put, row1, GE, 0);
            add_constraint(lp_put, row2, GE, 0);
            
            create_LP_for_european_put_option_without_memo(k+1,i+1);
            create_LP_for_european_put_option_without_memo(k+1,i-1);
        }
    }
}

void set_up_and_solve_the_LP_for_the_call_option()
{
    // get everything started; the number of variables equals 2 * the number of
    // vertices upto the last-but-one layer before expiry
    lp_call_memo = make_lp(0,no_of_divisions*(no_of_divisions+1));
    
    // This keeps the message reporting of lp_solve to a minimum
    set_verbose(lp_call_memo, 3);
    
    // set the constraints in the LP for a call option
    clock_t start_time1 = clock();
    create_LP_for_european_call_option(0,0);
    clock_t end_time1 = clock();
    cout<<"Running time for adding equations to the LPsolve model (with memoization): "<< ((float)end_time1 - (float) start_time1)/CLOCKS_PER_SEC<<" seconds"<<endl;
    
    
    // set the objective function
    {
        double row[no_of_divisions*(no_of_divisions+1)+1];
        for (int i = 0; i < no_of_divisions*(no_of_divisions+1)+1; i++)
            row[i] = 0.0;
        row[1] = initial_stock_price;
        row[2] = -1;
        set_obj_fn(lp_call_memo, row);
    }
    
    // solve the LP
    {
        clock_t start_time = clock();
        int error_code = solve(lp_call_memo);
        clock_t end_time = clock();
        cout<<"Running time for solve the model (with memoization): "<< ((float)end_time - (float) start_time)/CLOCKS_PER_SEC<<" seconds"<<endl;
        
        if (0 == error_code)
            cout << "Call Price according the LP formulation (with memoization) = " << get_objective(lp_call_memo) << endl;
        else {
            cout << "LP solve ran into problems; Error Code = " << error_code << endl;
            cout << "Look up http://lpsolve.sourceforge.net/5.1/solve.htm for error code details" << endl;
        }
    }
    
    // delete the lp to release memory
    delete_lp(lp_call_memo);
}

void set_up_and_solve_the_LP_for_the_put_option()
{
    // get everything started; the number of variables equals 2 * the number of
    // vertices upto the last-but-one layer before expiry
    lp_put_memo = make_lp(0,no_of_divisions*(no_of_divisions+1));
    
    // This keeps the message reporting of lp_solve to a minimum
    set_verbose(lp_put_memo, 3);
    
    // set the constraints in the LP for a call option
    
    clock_t start_time1 = clock();
    create_LP_for_european_put_option(0,0);
    clock_t end_time1 = clock();
    cout<<"Running time for adding equations to the LPsolve model (with memoization): "<< ((float)end_time1 - (float) start_time1)/CLOCKS_PER_SEC<<" seconds"<<endl;
    
    // set the objective function
    {
        double row[no_of_divisions*(no_of_divisions+1)+1];
        for (int i = 0; i < no_of_divisions*(no_of_divisions+1)+1; i++)
            row[i] = 0.0;
        row[1] = -initial_stock_price;
        row[2] = 1;
        set_obj_fn(lp_put_memo, row);
    }
    
    // solve the LP
    {
        clock_t start_time = clock();
        int error_code = solve(lp_put_memo);
        clock_t end_time = clock();
        cout<<"Running time for solve the model (with memoization): "<< ((float)end_time - (float) start_time)/CLOCKS_PER_SEC<<" seconds"<<endl;
        
        if (0 == error_code)
            cout << "Put Price according the LP formulation (with memoization) = " << get_objective(lp_put_memo) << endl;
        else {
            cout << "LP solve ran into problems; Error Code = " << error_code << endl;
            cout << "Look up http://lpsolve.sourceforge.net/5.1/solve.htm for error code details" << endl;
        }
    }
    
    // delete the lp to release memory
    delete_lp(lp_put_memo);
}

void set_up_and_solve_the_LP_for_the_call_option_without_memo()
{
    // get everything started; the number of variables equals 2 * the number of
    // vertices upto the last-but-one layer before expiry
    lp_call = make_lp(0,no_of_divisions*(no_of_divisions+1));
    
    // This keeps the message reporting of lp_solve to a minimum
    set_verbose(lp_call, 3);
    
    clock_t start_time1 = clock();
    create_LP_for_european_call_option_without_memo(0,0);
    clock_t end_time1 = clock();
    cout<<"Running time for adding equations to the LPsolve model (without memoization): "<< ((float)end_time1 - (float) start_time1)/CLOCKS_PER_SEC<<" seconds"<<endl;
    // set the constraints in the LP for a call option
    
    
    // set the objective function
    {
        double row[no_of_divisions*(no_of_divisions+1)+1];
        for (int i = 0; i < no_of_divisions*(no_of_divisions+1)+1; i++)
            row[i] = 0.0;
        row[1] = initial_stock_price;
        row[2] = -1;
        set_obj_fn(lp_call, row);
    }
    
    // solve the LP
    {
        clock_t start_time = clock();
        int error_code = solve(lp_call);
        clock_t end_time = clock();
        cout<<"Running time for solve the model (without memoization): "<< ((float)end_time - (float) start_time)/CLOCKS_PER_SEC<<" seconds"<<endl;
        
        if (0 == error_code)
            cout << "Call Price according the LP formulation (without memoization) = " << get_objective(lp_call) << endl;
        else {
            cout << "LP solve ran into problems; Error Code = " << error_code << endl;
            cout << "Look up http://lpsolve.sourceforge.net/5.1/solve.htm for error code details" << endl;
        }
    }
    
    // delete the lp to release memory
    delete_lp(lp_call);
}

void set_up_and_solve_the_LP_for_the_put_option_without_memo()
{
    // get everything started; the number of variables equals 2 * the number of
    // vertices upto the last-but-one layer before expiry
    lp_put = make_lp(0,no_of_divisions*(no_of_divisions+1));
    
    // This keeps the message reporting of lp_solve to a minimum
    set_verbose(lp_put, 3);
    
    clock_t start_time1 = clock();
    create_LP_for_european_put_option_without_memo(0,0);
    clock_t end_time1 = clock();
    cout<<"Running time for adding equations to the LPsolve model (without memoization): "<< ((float)end_time1 - (float) start_time1)/CLOCKS_PER_SEC<<" seconds"<<endl;
    // set the constraints in the LP for a call option
    
    
    // set the objective function
    {
        double row[no_of_divisions*(no_of_divisions+1)+1];
        for (int i = 0; i < no_of_divisions*(no_of_divisions+1)+1; i++)
            row[i] = 0.0;
        row[1] = -initial_stock_price;
        row[2] = 1;
        set_obj_fn(lp_put, row);
    }
    
    // solve the LP
    {
        
        
        clock_t start_time = clock();
        int error_code = solve(lp_put);
        clock_t end_time = clock();
        cout<<"Running time for solve the model (without memoization): "<< ((float)end_time - (float) start_time)/CLOCKS_PER_SEC<<" seconds"<<endl;
        
        if (0 == error_code)
            cout << "Put Price according the LP formulation (without memoization) = " << get_objective(lp_put) << endl;
        else {
            cout << "LP solve ran into problems; Error Code = " << error_code << endl;
            cout << "Look up http://lpsolve.sourceforge.net/5.1/solve.htm for error code details" << endl;
        }
    }
    
    // delete the lp to release memory
    delete_lp(lp_put);
}

int main (int argc, char* argv[])
{
    sscanf (argv[1], "%lf", &expiration_time);
    sscanf (argv[2], "%d", &no_of_divisions);
    sscanf (argv[3], "%lf", &risk_free_rate);
    sscanf (argv[4], "%lf", &volatility);
    sscanf (argv[5], "%lf", &initial_stock_price);
    sscanf (argv[6], "%lf", &strike_price);
    
    call_memoization = new int*[no_of_divisions];
    for (int i = 0; i < no_of_divisions; i++)
        call_memoization[i] = new int[no_of_divisions];
    put_memoization = new int*[no_of_divisions];
    for (int i = 0; i < no_of_divisions; i++)
        put_memoization[i] = new int[no_of_divisions];
    for (int i = 0; i < no_of_divisions; i++)
    {
        for (int j = 0; j < no_of_divisions; j++)
        {
            call_memoization[i][j] = -1;
            put_memoization[i][j] = -1;
        }
    }
    
    up_factor = exp(volatility*sqrt(expiration_time/((double) no_of_divisions)));
    R = exp(risk_free_rate*expiration_time/((double) no_of_divisions));
    
    cout << "--------------------------------------" << endl;
    cout << "European Option Pricing via Linear Programming" << endl;
    cout << "Expiration Time (Years) = " << expiration_time << endl;
    cout << "Number of Divisions = " << no_of_divisions << endl;
    cout << "Risk Free Interest Rate = " << risk_free_rate << endl;
    cout << "Volatility (%age of stock value) = " << volatility*100 << endl;
    cout << "Initial Stock Price = " << initial_stock_price << endl;
    cout << "Strike Price = " << strike_price << endl;
    cout << "R = " << R << endl;
    cout << "Up-Factor = " << up_factor << endl;
    cout << "--------------------------------------" << endl;

    
    cout << "Call Price by Black-Scholes = " <<
    option_price_call_black_scholes(initial_stock_price, strike_price, risk_free_rate, volatility, expiration_time) << endl;
    
    clock_t start_time_mem1 = clock();
    set_up_and_solve_the_LP_for_the_call_option();
    clock_t end_time_mem1 = clock();
    cout<<"Running time for whole : "<< ((float)end_time_mem1 - (float) start_time_mem1)/CLOCKS_PER_SEC<<" seconds"<<endl;
    cout << "--------------------------------------" << endl;
    
    clock_t start_time1 = clock();
    set_up_and_solve_the_LP_for_the_call_option_without_memo();
    clock_t end_time1 = clock();
    cout<<"Running time for whole : "<< ((float)end_time1 - (float) start_time1)/CLOCKS_PER_SEC<<" seconds"<<endl;
    
    cout << "--------------------------------------" << endl;
    
    
    cout << "Put Price by Black-Scholes = " <<
    option_price_put_black_scholes(initial_stock_price, strike_price, risk_free_rate, volatility, expiration_time) << endl;
   
    clock_t start_time_mem2 = clock();
    set_up_and_solve_the_LP_for_the_put_option();
    clock_t end_time_mem2 = clock();
    cout<<"Running time for whole : "<< ((float)end_time_mem2 - (float) start_time_mem2)/CLOCKS_PER_SEC<<" seconds"<<endl;
    
    cout << "--------------------------------------" << endl;
    clock_t start_time2 = clock();
    set_up_and_solve_the_LP_for_the_put_option_without_memo();
    clock_t end_time2 = clock();
    cout<<"Running time for whole : "<< ((float)end_time2 - (float) start_time2)/CLOCKS_PER_SEC<<" seconds"<<endl;
    
    cout << "--------------------------------------" << endl;

}

