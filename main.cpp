#include <cstdlib>
#include <assert.h>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <exception>
#include <cmath>
#include <string>
#include <vector>
#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>

#include <unistd.h>
#include "binary_tree.hpp"

/**
 *  Builds tree of basic mathematical operators
 *  Find all permutations of tree (42) for 6 leaves
 *      Find all permutations of leaves (numbers) 5^5 * 6P6
 *  Track minimized error
 */


//shared vars
std::vector<int> numbers;
int target = 0;

//Arithmetic operations and their names
enum ARITH_OP { _add = 0, _sub, _mul, _div, _ignore, _pow };
const char *OPS[] = {"ADD", "SUB", "MUL", "DIV", "IGNORE", "POW"};
#define NUM_OPS 6

//Store a solution to numbers game
typedef struct { 
    std::mutex mtx;
    struct tnode *tree;
    std::vector<int> numbers;
    std::vector<ARITH_OP> ops;
    long double answer = 0.0;
    long double diff = -1.0;
} solution;

solution *min = new solution;

//Store map of all possible trees for N leaves
std::unordered_map<long long, struct tnode *> trees;

//Func prototypes
std::string explain_tree(struct tnode *tree, const std::vector<ARITH_OP> &ops,
                const std::vector<int> &numbers, int &num_index, int &op_index);
long double execute_tree(struct tnode *tree, const std::vector<ARITH_OP> &ops,
                const std::vector<int> &numbers, int &num_index, int &op_index);
void permutateTreeOptions(struct tnode *tree, std::vector<int> numbers);
void buildAllTrees(struct tnode *node);


//Parse inputs to program
void parseInputs(int argc, char *argv[])
{
    for(int i=1; i<argc-1;++i){
        int a = atoi(argv[i]);
        numbers.push_back(a);
    }
    target = atoi(argv[argc-1]);
}

//Recursilvly transverse all paths of root_node tree expanding all untaken paths
void navigateTree(const struct tnode *root_node, struct tnode* node)
{
    //execute both left and right building all trees
    if(node->right == NULL){
        struct tnode *new_tree = root_node->copy_tree();
        new_tree = new_tree->transverse(node->pos, node->lv);
        new_tree->right = new struct tnode(new_tree);
        new_tree->right->pos = new_tree->pos << 1;
        new_tree->right->pos += 1;
        buildAllTrees(new_tree->right);
    }else{
        navigateTree(root_node, node->right);
    }

    //execute both left and right building all trees
    if(node->left == NULL){
        struct tnode *new_tree = root_node->copy_tree();
        new_tree = new_tree->transverse(node->pos, node->lv);
        new_tree->left = new struct tnode(new_tree);
        new_tree->left->pos = new_tree->pos << 1;
        buildAllTrees(new_tree->left);
    }else{
        navigateTree(root_node, node->left);
    }
}

//Recursive flood down function that builds all possible tree structures
void buildAllTrees(struct tnode *node)
{
    //find size of this tree from root
    struct tnode *root_node = node->root();
    size_t leaves = root_node->leaves();
    size_t depth = root_node->depth();
#ifdef DEBUG
    std::cout << "Depth: " << depth << ", Leaves: " << leaves << std::endl << root_node->print() << std::endl;
#endif

    if(leaves < numbers.size()){
        /**
         *  Tree finding algo
         *  Expand this tree:
         *      For each unused l/r node, expand root tree and take it passing
         *      it back to rebuild
         */
        navigateTree(root_node, root_node);
    }else{
        if(leaves == numbers.size()){
            long long key = root_node->hash();
            auto it = trees.find(key);
            if(it == trees.end()){
                trees.insert(std::make_pair(key, root_node));
                // trees.insert(key, root_node);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    parseInputs(argc, argv);
    struct tnode *root_node = new struct tnode;
    buildAllTrees(root_node);
    std::cout << "Tree permutations: " << trees.size() << std::endl;
    std::cout << "Target number: " << target << std::endl;

    //New thread per tree structure
    std::vector<std::thread *> threads;

    //open a thread per tree, try all permutations of operations and number on the tree
    for(auto it = trees.begin(); it != trees.end(); ++it){
        threads.push_back( new std::thread( permutateTreeOptions, it->second, numbers ) );
    }

    for(std::thread* t: threads){
        t->join();
    }

    std::cout << "Best solution: " << min->answer << std::endl;
    std::cout << "Tree: " << std::endl << min->tree->print() << std::endl;

    std::cout << "Operations:" << std::endl;
    for(ARITH_OP a: min->ops){
        std::cout << OPS[static_cast<int>(a)] << "\t";
    }
    std::cout << std::endl;

    std::cout << "Numbers:" << std::endl;
    for(int a: min->numbers){
        std::cout << a << "\t";
    }
    std::cout << std::endl;

    std::cout << "Explaination:" << std::endl;
    int a=0,b=0;
    std::string calc_explained = explain_tree(min->tree, min->ops, min->numbers, a, b);
    std::cout << calc_explained << " = " << min->answer << std::endl;

    return 0;
}


void permutateTreeOptions(struct tnode *tree, std::vector<int> numbers)
{
    sort(numbers.begin(), numbers.end());
    //for permutations of tree operations
    size_t tree_nodes = tree->leaves();

    std::vector<ARITH_OP> ops;
    for(size_t i = 1; i < tree_nodes; i++){
        ops.push_back( static_cast<ARITH_OP>(0) );
    }

    //while iterated over all of the tree nodes
    for(size_t j=1; j < pow(NUM_OPS, tree_nodes) ;j++){

        ops[0] = static_cast<ARITH_OP>(j % NUM_OPS);
        for(size_t i=1; i < tree_nodes-1; i++){
           int c = ( j / static_cast<size_t>( pow(NUM_OPS, i) )  ) % NUM_OPS;
           ops[i] = static_cast<ARITH_OP>(c);
        }

        if(static_cast<size_t>(ops[tree_nodes-1]) > NUM_OPS-1)
            break;

        do{
            //execute tree + airth options + numbers perm
            int a=0,b=0;
            long double ret = execute_tree(tree, ops, numbers, a, b);
            if(!std::isfinite(ret))
                continue;

            //abs eturning less than 0?
            // long double diff = abs(ret - target);
            long double diff = ret - target;
            if(diff < 0.0)
                diff = -diff;

            assert(diff >= 0);

            //check if another thread has found the answer
            if(min->diff == 0.0){
                return;
            }
            if(diff < min->diff || min->diff < 0){
                min->mtx.lock();
                if(diff < min->diff || min->diff < 0){
                    min->diff = diff;
                    min->tree = tree;
                    min->numbers = numbers;
                    min->answer = ret;
                    min->ops = ops;
                }
                min->mtx.unlock();
            }
        }while(next_permutation(numbers.begin(), numbers.end()));
    }
}

long double execute_tree(struct tnode *tree, const std::vector<ARITH_OP> &ops,
                const std::vector<int> &numbers, int &num_index, int &op_index)
{
    //set ops to tree nodes
    long double left=0,right=0,result=0;

    if(tree->left != NULL)
        left = execute_tree(tree->left, ops, numbers, num_index, op_index);
    else{
        left = numbers[num_index];
        num_index++;
    }
    
    if(tree->right != NULL)
        right = execute_tree(tree->right, ops, numbers, num_index, op_index);
    else{
        right = numbers[num_index];
        num_index++;
    }

    /* we now have our first non ignore operator    */
    switch(ops[op_index]){
    case ARITH_OP::_add:
        result = left + right;
        break;
    case ARITH_OP::_sub:
        result = left - right;
        break;
    case ARITH_OP::_mul:
        result = left * right;
        break;
    case ARITH_OP::_div:
        result = left / right;
        break;
    case ARITH_OP::_ignore:
        result = 0;
        break;
    case ARITH_OP::_pow:
        result = pow( left , right );
        break;
    }
    op_index++;
    return result;
}

std::string explain_tree(struct tnode *tree, const std::vector<ARITH_OP> &ops,
                const std::vector<int> &numbers, int &num_index, int &op_index)
{
    //set ops to tree nodes
    std::string left,right,result;

    if(tree->left != NULL)
        left = explain_tree(tree->left, ops, numbers, num_index, op_index);
    else{
        left = std::to_string( numbers[num_index] );
        num_index++;
    }
    
    if(tree->right != NULL)
        right = explain_tree(tree->right, ops, numbers, num_index, op_index);
    else{
        right = std::to_string( numbers[num_index] );
        num_index++;
    }

    /* we now have our first non ignore operator    */
    switch(ops[op_index]){
    case ARITH_OP::_add:
        result = "( " + left + " + " + right + " )";
        break;
    case ARITH_OP::_sub:
        result = "( " + left + " - " + right + " )";
        break;
    case ARITH_OP::_mul:
        result = "( " + left + " * " + right + " )";
        break;
    case ARITH_OP::_div:
        result = "( " + left + " / " + right + " )";
        break;
    case ARITH_OP::_ignore:
        result = "0";
        break;
    case ARITH_OP::_pow:
        result = "( " + left + " ^( " + right + " ) )";
        break;
    }
    op_index++;
    return result;
}
