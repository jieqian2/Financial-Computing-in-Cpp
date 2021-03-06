//
//  main.cpp
//  prog3-lpsolve
//
//  Created by Jay Chen on 9/11/19.
//  Copyright © 2019 Jay Chen. All rights reserved.
//



//
// There is an ILP formulation for any 9 x 9 Sudoku Puzzle
// See the handout for the formulation -- there are 729 variables
// with a whole lot of constraints that is driven by the input
// (which is the incomplete board-layout).  You can write this
// (moderate-size) ILP automatically in C++, which is then solved
// using the API for Lpsolve.

#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <cstdlib>

#include "lp_lib.h"

using namespace std;

// Global variables for the problem 9*9*9 = 729
// See my handout for further explanations
lprec *lp;
double solution[729];

// This sets the generic ILP for the Sudoku Puzzle
// It does not use any of the pre-determined board-positions,
// that part is done after the input file is read.
void set_sudoku_ilp()
{
    // setting the problem up: 9*9*9 = 729 variables
    lp = make_lp(0,729);
    
    // This keeps the message reporting of lp_solve to a minimum
    set_verbose(lp, 3);
    
    {
        for (int i = 1; i <= 9; i++)
        {
            for (int j = 1; j <= 9; j++)
            {
                // constraint that says each (i,j) entry in the 9x9 table
                // should have one number in it
                // creating a row with 1's at the right spot (the first
                // entry has to be a zero; this is idiosynchratic of lpsolve)
                
                double row[730];
                for (int k = 0; k < 730; k++)
                    row[k] = 0;
                
                for (int k = 1; k <= 9; k++)
                    row[(81*(i-1)) + (9*(j-1)) + k] = 1;
                
                // adding the constraint
                add_constraint(lp, row, EQ, 1);
            }
        }
    }
    
    {
        // Using the above code as a guide, put the appropriate lines that
        // articulate the constraint that says each number must appear once
        // in each row in the 9x9 table; create a bunch of rows with 1's at
        // the right spot (the first entry has to be a zero; this is
        // idiosynchratic of lpsolve)
        
        // Just observe the given restriction, we could find that the i,j,k is symmetrical. So in this way, we just need to swith the position of k with j.
        for(int i=1; i<=9; i++)
            for(int k=1; k<=9; k++)
            {
                double row[730];
                for (int k = 0; k < 730; k++)
                    row[k] = 0;
                
                for(int j=1; j<=9; j++)
                    row[(81*(i-1)) + (9*(j-1)) + k] = 1;
                
                add_constraint(lp, row, EQ, 1);
                
            }
    }
    
    {
        // Using the above code as a guide, put the appropriate lines that
        // articulate the constraint that says each number must appear once
        // in each column in the 9x9 table; create a bunch of rows with 1's at
        // the right spot (the first entry has to be a zero; this is
        // idiosynchratic of lpsolve)
        
        //// Just observe the given restriction, we could find that the i,j,k is symmetrical. So in this way, we just need to swith the position of i with k.
        
        for(int j=1; j<=9; j++)
            for(int k=1; k<=9; k++)
            {
                double row[730];
                for (int k = 0; k < 730; k++)
                    row[k] = 0;
                
                for(int i=1; i<=9; i++)
                    row[(81*(i-1)) + (9*(j-1)) + k] = 1;
                
                add_constraint(lp, row, EQ, 1);
            }
    }
    
    // making sure each number occurs once within each block
    // Block 1
    {
        for (int k = 1; k <= 9; k++)
        {
            // Using the above code as a guide, put the appropriate lines that
            // articulate theconstraint that says each number must appear once
            // in each 3 x 3 block; create a bunch of rows with 1's at the right
            // spot (the first entry has to be a zero; this is idiosynchratic of
            // lpsolve)
            
            //the first block is between i=1&j=1 to i=3&j=3 and k from 1 to 9. so we can got the formula.
            //and the rest blocks are basically the same.
            
            double row[730];
            for (int k = 0; k < 730; k++)
                row[k] = 0;
            
            for(int i=1; i<=3; i++)
                for(int j=1; j<=3; j++)
                    row[(81*(i-1)) + (9*(j-1)) + k] = 1;
            
            add_constraint(lp, row, EQ, 1);
        }
    }
    
    // Block 2
    {
        for (int k = 1; k <= 9; k++)
        {
            // Using the above code as a guide, put the appropriate lines that
            // articulate theconstraint that says each number must appear once
            // in each 3 x 3 block; create a bunch of rows with 1's at the right
            // spot (the first entry has to be a zero; this is idiosynchratic of
            // lpsolve)
            double row[730];
            for (int k = 0; k < 730; k++)
                row[k] = 0;
            
            for(int i=4; i<=6; i++)
                for(int j=1; j<=3; j++)
                    row[(81*(i-1)) + (9*(j-1)) + k] = 1;
            
            add_constraint(lp, row, EQ, 1);
        }
    }
    
    // Block 3
    {
        for (int k = 1; k <= 9; k++)
        {
            // Using the above code as a guide, put the appropriate lines that
            // articulate theconstraint that says each number must appear once
            // in each 3 x 3 block; create a bunch of rows with 1's at the right
            // spot (the first entry has to be a zero; this is idiosynchratic of
            // lpsolve)
            double row[730];
            for (int k = 0; k < 730; k++)
                row[k] = 0;
            
            for(int i=7; i<=9; i++)
                for(int j=1; j<=3; j++)
                    row[(81*(i-1)) + (9*(j-1)) + k] = 1;
            
            add_constraint(lp, row, EQ, 1);
        }
    }
    
    // Block 4
    {
        for (int k = 1; k <= 9; k++)
        {
            // Using the above code as a guide, put the appropriate lines that
            // articulate theconstraint that says each number must appear once
            // in each 3 x 3 block; create a bunch of rows with 1's at the right
            // spot (the first entry has to be a zero; this is idiosynchratic of
            // lpsolve)
            double row[730];
            for (int k = 0; k < 730; k++)
                row[k] = 0;
            
            for(int i=1; i<=3; i++)
                for(int j=4; j<=6; j++)
                    row[(81*(i-1)) + (9*(j-1)) + k] = 1;
            
            add_constraint(lp, row, EQ, 1);
        }
    }
    
    // Block 5
    {
        for (int k = 1; k <= 9; k++)
        {
            // Using the above code as a guide, put the appropriate lines that
            // articulate theconstraint that says each number must appear once
            // in each 3 x 3 block; create a bunch of rows with 1's at the right
            // spot (the first entry has to be a zero; this is idiosynchratic of
            // lpsolve)
            double row[730];
            for (int k = 0; k < 730; k++)
                row[k] = 0;
            
            for(int i=4; i<=6; i++)
                for(int j=4; j<=6; j++)
                    row[(81*(i-1)) + (9*(j-1)) + k] = 1;
            
            add_constraint(lp, row, EQ, 1);
        }
    }
    
    // Block 6
    {
        for (int k = 1; k <= 9; k++)
        {
            // Using the above code as a guide, put the appropriate lines that
            // articulate theconstraint that says each number must appear once
            // in each 3 x 3 block; create a bunch of rows with 1's at the right
            // spot (the first entry has to be a zero; this is idiosynchratic of
            // lpsolve)
            double row[730];
            for (int k = 0; k < 730; k++)
                row[k] = 0;
            
            for(int i=7; i<=9; i++)
                for(int j=4; j<=6; j++)
                    row[(81*(i-1)) + (9*(j-1)) + k] = 1;
            
            add_constraint(lp, row, EQ, 1);
        }
    }
    
    // Block 7
    {
        for (int k = 1; k <= 9; k++)
        {
            // Using the above code as a guide, put the appropriate lines that
            // articulate theconstraint that says each number must appear once
            // in each 3 x 3 block; create a bunch of rows with 1's at the right
            // spot (the first entry has to be a zero; this is idiosynchratic of
            // lpsolve)
            double row[730];
            for (int k = 0; k < 730; k++)
                row[k] = 0;
            
            for(int i=1; i<=3; i++)
                for(int j=7; j<=9; j++)
                    row[(81*(i-1)) + (9*(j-1)) + k] = 1;
            
            add_constraint(lp, row, EQ, 1);
        }
    }
    
    // Block 8
    {
        for (int k = 1; k <= 9; k++)
        {
            // Using the above code as a guide, put the appropriate lines that
            // articulate theconstraint that says each number must appear once
            // in each 3 x 3 block; create a bunch of rows with 1's at the right
            // spot (the first entry has to be a zero; this is idiosynchratic of
            // lpsolve)
            double row[730];
            for (int k = 0; k < 730; k++)
                row[k] = 0;
            
            for(int i=4; i<=6; i++)
                for(int j=7; j<=9; j++)
                    row[(81*(i-1)) + (9*(j-1)) + k] = 1;
            
            add_constraint(lp, row, EQ, 1);
        }
    }
    
    // Block 9
    {
        for (int k = 1; k <= 9; k++)
        {
            // Using the above code as a guide, put the appropriate lines that
            // articulate theconstraint that says each number must appear once
            // in each 3 x 3 block; create a bunch of rows with 1's at the right
            // spot (the first entry has to be a zero; this is idiosynchratic of
            // lpsolve)
            double row[730];
            for (int k = 0; k < 730; k++)
                row[k] = 0;
            
            for(int i=7; i<=9; i++)
                for(int j=7; j<=9; j++)
                    row[(81*(i-1)) + (9*(j-1)) + k] = 1;
            
            add_constraint(lp, row, EQ, 1);
        }
    }
    
    // upper-bound each variable by 1
    for (int i = 1; i <= 729; i++)
    {
        double row[730];
        for (int j = 0; j < 730; j++)
            row[j] = 0;
        row[i] = 1;
        add_constraint(lp, row, LE, 1);
    }
    
    // it does not matter what the objective function (why?)
    // I am minimizing the sum of all variables.
    {
        double row[730];
        for (int i = 1; i < 730; i++)
            row[i] = 1;
        row[0] = 0;
        
        set_obj_fn(lp, row);
    }
    
    // set the variables to be integers
    for (int i = 1; i <= 729; i++)
        set_int(lp, i, TRUE);
}

// This subroutine reads the incomplete board-information from the
// input file and sets the appropriate constraints before the ILP
// is solved.
void read_input_data(char* argv[])
{
    // reading the input filename from commandline
    ifstream input_filename(argv[1]);
    
    if (input_filename.is_open()) {
        
        cout << "Input File Name: " << argv[1] << endl;
        cout << endl << "Initial Board Position" << endl;
        for (int i = 1; i <= 9; i++)
        {
            for (int j = 1; j <= 9; j++)
            {
                int value_just_read;
                input_filename >> value_just_read;
                
                // check if we have a legitimate integer between 1 and 9
                if ((value_just_read >= 1) && (value_just_read <= 9))
                {
                    // printing initial value of the puzzle with some formatting
                    cout << value_just_read << " ";
                    
                    // add appropriate constraints that bind the value of the
                    // appropriate variables based on the incomplete information
                    // that was read from the input file
                    
                    //since we have already read the number, the next thing is to put the number in row[] with order.
                    // the order here is to put the fill the corresponding row[i]with 1 and fill others with 0;
                    
                    double row[730];
                    for (int k = 0; k < 730; k++)
                        row[k] = 0;
                    row[(81*(i-1)) + (9*(j-1)) + value_just_read ] = 1;
                    
                    add_constraint(lp, row, EQ, 1);
                    
                }
                else {
                    // printing initial value of the puzzle with some formatting
                    cout << "X ";
                }
                
            }
            cout << endl;
        }
    }
    else {
        cout << "Input file missing" << endl;
        exit(0);
    }
}

// The ILP formulation is solved using the API for Lpsolve
// Pay attention to how the solution is interpretted...
void solve_the_puzzle()
{
    int ret;
    // solve the lp
    ret = solve(lp);
    
    // Check if you had a solution
    // (see online guide for the codes at http://lpsolve.sourceforge.net/5.0/ )
    if (ret == 0)
    {
        
        // get the optimal assignment
        get_variables(lp, solution);
        
        // print the solution
        cout << endl << "Final Solution" << endl;
        {
            // Figure out a way to look at the 729-long 0/1 vector solution
            // to the ILP and print the appropriate integer value to be
            // inserted into each square of the solution to the Sudoku puzzle
            
            // the count is used to count the numbers we've printed, every time we printed 9 numbers, we just cout<<endl to continue print the next new line.
            
            int count=0;
            
            // in this part, we have the transform the binary into the 0~9 numbers in sudoku number.
            // given solution[i], to finish this, we just need to cout the (i+1)%9, which means the number of k.
            // but we get 2 exceptional case.
            //1. when i<=9, (i+1)%9=0 so we need to direct cout the number i+1
            //2. when (i+1)%9=0, which means k = 9, we just need to direct cout 9.
            for(int i=0; i<730; i++)
            {
                if( solution[i]==1 )
                {
                    if(i<=9)
                    {
                        cout<< i+1 << " ";
                        count++;
                    }
                    else
                    {
                        if( (i+1)%9 != 0 )
                        {
                            cout<< (i+1)%9 <<" ";
                            count++;
                        }
                        else
                        {
                            cout<<"9 ";
                            count++;
                        }
                    }
                    // this just mean to print the puzzle,
                    if(count%9==0)
                        cout<<endl;
                }
            }
        }
    }
    else {
        cout << "Check the input file... looks like there is no solution" << endl;
    }
    
    
    delete_lp(lp);
}

int main (int argc, char* argv[])
{
    // formulate the non-input related part of the puzzle
    set_sudoku_ilp();
    
    // read the incomplete input information, and set appropriate constraints
    read_input_data(argv);
    
    // solve the puzzle and print the solution
    solve_the_puzzle();
    
    return(0);
}
