Countdown Solver
=================

### Description

Multi-threaded extended countdown number game solver. Based on the countdown number game, generate all binary tree permutations with N-1 leaves (N being the number of cmd line args). Brute force all permutations of mathematical operations for all permutations of numbers for all permutations of binary trees with (N-1) leaves. The countdown numbers game has been extended to include the exponent operation. Input numbers may be used 0 or 1 times and the aim of the game is to combine the inputs to get the target number. The number of tree permutations follow the [Catalan numbers](https://en.wikipedia.org/wiki/Catalan_number).


### Usage
```
./countdown [input numbers] [target]
```

### Examples
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
Number of trees: 5
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
Explanation:
( 2 * ( ( 3 * 7 ) * 3 ) ) = 65536
./countdown 2 3 5 7 65537  0.00s user 0.00s system 131% cpu 0.004 total
```

```
▶ time ./countdown 2 3 5 7 11 13 17 639
Tree permutations: 132
Target number: 639
Number of trees: 132
Best solution: 639
Tree:
                o
              /
           o
         /
       o
      / \
    o     o
     \
      o


Operations:
POW     MUL     SUB     ADD     ADD     ADD
Numbers:
5       11      2       7       3       13      17
Explanation:
( ( ( ( 5 * ( 11 ^ 2 ) ) + ( 7 - 3 ) ) + 13 ) + 17 ) = 639
./countdown 2 3 5 7 11 13 17 639  0.92s user 0.05s system 694% cpu 0.140 total
```

### Compilation

C++14 compatable compiler is required.

`clang++ main.cpp -o countdown --std=c++1y -Ofast -funroll-loops`

##### Debug
`clang++ -std=c++1y main.cpp -o countdown -g -g3 -ggdb -Wall -Wextra -Werror -DDEBUG`
