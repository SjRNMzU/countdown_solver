countdown
=================

Based on the countdown number game, will generate all binary tree permutations with N-1 leaves. 
Brute force all permutations of mathematical operations and numbers and numbers.
The countdown numbers game has been expanded to include the exponent operation.

#compile with clang++
clang++ main.cpp --std=c++11 -o countdown

#first N-1 numbers are used as the input numbers, Nth number is target
#This will attept to form an exquation generating the traget value using 
#the input numbers only once
./countdown 75 50 2 3 8 7 812
