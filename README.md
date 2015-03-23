Countdown Solver
=================

### Description

Multi-threaded extended countdown number game solver. Based on the countdown number game, this will generate all binary tree permutations with N-1 leaves (N being the number of cmd line args). Brute force all permutations of mathematical operations for all permutations of numbers for all permutations of binary trees with (N-1) leaves. The countdown numbers game has been expanded to include the exponent operation. Input numbers may be used 0 or 1 times and the aim of the game is to combine the inputs to get the target number.


### Usage
```
./countdown [input numbers] [target]
```

### Example
```
▶ ./countdown 3 7 15 53 55 89 5243
Tree permutations: 42
Target number: 5243
Best solution: 5243
Tree:
          o
         / \
       o     o
        \   /
         o o


Operations:
MUL     MUL     MUL     ADD     SUB
Numbers:
7       15      53      3       89      55
Explaination:
( ( 7 * ( 15 * 53 ) ) - ( ( 3 * 89 ) + 55 ) ) = 5243

```


Extended version also works with exponential operator.

```
▶ time ./countdown 2 3 5 7 65537
Tree permutations: 5
Target number: 65537
Best solution: 65536
Tree:
          o
           \
             o
            /
           o


Operations:
MUL     SUB     POW
Numbers:
2       3       7       5
Explaination:
( 2 ^( ( ( 3 * 7 ) - 5 ) ) ) = 65536
./countdown 2 3 5 7 65537  0.02s user 0.00s system 257% cpu 0.008 total
```


### Compilation
`clang++ main.cpp -o countdown --std=c++11 -O2`
