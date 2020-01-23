//
//  main.cpp
//  prog8-AmericanOptionPricing_Trinomial_DP
//
//  Created by Jay Chen on 11/6/19.
//  Copyright © 2019 Jay Chen. All rights reserved.
//
#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include "newmat.h"
using namespace std;

//inspired by American Option via Dynamic Programming and the Binomial Model.cpp in compas2g

double up_factor, uptick_prob, downtick_prob, notick_prob, risk_free_rate, strike_price;
double initial_stock_price, expiration_time, volatility, R;
int no_of_divisions;

double max(double a, double b)
{
    return (b < a )? a:b;
}

double american_call_option_dyn_prog()
{
    // create the probability matrix
    Matrix transition_probability(2*no_of_divisions+1, 2*no_of_divisions+1);
    
    //probibality matrix is coming from
    // campas2g flipped class video;
    
    for (int i = 1; i <= 2*no_of_divisions+1; i++)
        for (int j = 1; j <= 2*no_of_divisions+1; j++)
            transition_probability(i,j) = 0.0;
    
    // boundary values of the probabilities need to be entered
    transition_probability(1,1) = 1.0 - notick_prob;
    transition_probability(1,2) = notick_prob;
    transition_probability(2*no_of_divisions+1,2*no_of_divisions+1) = notick_prob;
    transition_probability(2*no_of_divisions+1,2*no_of_divisions) = 1 - notick_prob;
    
    
    for (int i = 2; i <= 2*no_of_divisions; i++)
        for (int j = 1; j <= 2*no_of_divisions+1; j++)
        {
            if(j == (i-1))
                transition_probability(i,j) = downtick_prob;
            if(j == i)
                transition_probability(i,j) = notick_prob;
            if(j == (i+1))
                transition_probability(i,j) = uptick_prob;
        }
    
    Matrix V_t(2*no_of_divisions+1,1);
    // value at expiration
    for (int i = 1; i <= 2*no_of_divisions+1; i++)
        V_t(i,1) = max(0.0, (initial_stock_price*pow(up_factor,i-1-no_of_divisions)) - strike_price);
    
    // value at intermediate stages
    Matrix discounted_one_step_forward_value (2*no_of_divisions+1,1);
    Matrix value_if_option_is_exercised_now (2*no_of_divisions+1,1);
    for (int i = no_of_divisions; i > 0; i--) {
        // going backwards from expiration to zero-time
        discounted_one_step_forward_value = (transition_probability * V_t)/R;
        for (int j = 1; j <= 2*no_of_divisions+1; j++)
            value_if_option_is_exercised_now(j,1) = max(0, (initial_stock_price*pow(up_factor, j-1-no_of_divisions) - strike_price));
        for (int j = 1; j <= 2*no_of_divisions+1; j++)
            V_t(j,1) = max(value_if_option_is_exercised_now(j,1), discounted_one_step_forward_value(j,1));
    }
    // At the end of this backward movement, the middle-entry is the value of the american option
    return (V_t(no_of_divisions+1,1));
}

double american_put_option_dyn_prog()
{
    // create the probability matrix
    Matrix transition_probability(2*no_of_divisions+1, 2*no_of_divisions+1);
    
    for (int i = 1; i <= 2*no_of_divisions+1; i++)
        for (int j = 1; j <= 2*no_of_divisions+1; j++)
            transition_probability(i,j) = 0.0;
    
    // boundary values of the probabilities need to be entered
    transition_probability(1,1) = 1.0 - notick_prob;
    transition_probability(1,2) = notick_prob;
    transition_probability(2*no_of_divisions+1,2*no_of_divisions+1) = notick_prob;
    transition_probability(2*no_of_divisions+1,2*no_of_divisions) = 1 - notick_prob;
    
    
    for (int i = 2; i <= 2*no_of_divisions; i++)
        for (int j = 1; j <= 2*no_of_divisions+1; j++)
        {
            if(j == (i-1))
                transition_probability(i,j) = downtick_prob;
            if(j == i)
                transition_probability(i,j) = notick_prob;
            if(j == (i+1))
                transition_probability(i,j) = uptick_prob;
        }
    
    Matrix V_t(2*no_of_divisions+1,1);
    // value at expiration
    for (int i = 1; i <= 2*no_of_divisions+1; i++)
        V_t(i,1) = max(0.0, strike_price - (initial_stock_price*pow(up_factor,i-1-no_of_divisions)));
    // value at intermediate stages
    Matrix discounted_one_step_forward_value (2*no_of_divisions+1,1);
    Matrix value_if_option_is_exercised_now (2*no_of_divisions+1,1);
    //
    for (int i = no_of_divisions; i > 0; i--) {
        // going backwards from expiration to zero-time
        discounted_one_step_forward_value = (transition_probability * V_t)/R;
        for (int j = 1; j <= 2*no_of_divisions+1; j++)
            value_if_option_is_exercised_now(j,1) = max(0, strike_price - (initial_stock_price*pow(up_factor, j-1-no_of_divisions)));
        for (int j = 1; j <= 2*no_of_divisions+1; j++)
            V_t(j,1) = max(value_if_option_is_exercised_now(j,1), discounted_one_step_forward_value(j,1));
    }
    
    // At the end of this backward movement, the middle-entry is the value of the american option
    
    return (V_t(no_of_divisions+1,1));
}

int main (int argc, char* argv[])
{
    
    sscanf (argv[1], "%lf", &expiration_time);
    sscanf (argv[2], "%d", &no_of_divisions);
    sscanf (argv[3], "%lf", &risk_free_rate);
    sscanf (argv[4], "%lf", &volatility);
    sscanf (argv[5], "%lf", &initial_stock_price);
    sscanf (argv[6], "%lf", &strike_price);
    
    
    up_factor = exp(volatility*sqrt((2*expiration_time)/((float) no_of_divisions)));
    R = exp(risk_free_rate*expiration_time/((float) no_of_divisions));
    uptick_prob = pow((sqrt(R) - 1/sqrt(up_factor))/(sqrt(up_factor)-1/sqrt(up_factor)),2);
    downtick_prob = pow((sqrt(up_factor) - sqrt(R))/(sqrt(up_factor)-1/sqrt(up_factor)),2);
    notick_prob = 1 - uptick_prob - downtick_prob;
    
    cout << "American Option Pricing by Binomial-Model-Inspired Dynamic Programming" << endl;
    cout << "Expiration Time (Years) = " << expiration_time << endl;
    cout << "Number of Divisions = " << no_of_divisions << endl;
    cout << "Risk Free Interest Rate = " << risk_free_rate << endl;
    cout << "Volatility (%age of stock value) = " << volatility*100 << endl;
    cout << "Initial Stock Price = " << initial_stock_price << endl;
    cout << "Strike Price = " << strike_price << endl;
    cout << "--------------------------------------" << endl;
    cout << "R = " << R << endl;
    cout << "Up Factor = " << up_factor << endl;
    cout << "Uptick Probability = " << uptick_prob << endl;
    cout << "Downtick Probability = " << downtick_prob << endl;
    cout << "Notick Probability = " << notick_prob << endl;
    cout << "--------------------------------------" << endl;
    cout << "Price of an American Call Option = " << american_call_option_dyn_prog() << endl;
    cout << "Price of an American Put Option = " << american_put_option_dyn_prog() << endl;
    cout << "--------------------------------------" << endl;
}