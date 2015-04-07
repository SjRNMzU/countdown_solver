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
#include <future>
#include <thread>

#include <unistd.h>
#include "binary_tree.hpp"

/**
 *  Builds tree of basic mathematical operators
 *  Find all permutations of tree (42) for 6 leaves
 *      Find all permutations of leaves (numbers) 5^5 * 6P6
 *  Track minimized error
 */

//Arithmetic operations and their names
#define NUM_OPS 6
enum ARITH_OP { _add = 0, _sub, _mul, _div, _ignore, _pow };
const char *OPS[] = {"ADD", "SUB", "MUL", "DIV", "IGNORE", "POW"};

std::vector<int> numbers;
int target = 0;

//Store a solution to numbers game
typedef struct { 
    struct tnode *tree;
    std::vector<int> numbers;
    std::vector<ARITH_OP> ops;
    long double answer = 0.0;
    long double diff = -1.0;
} solution;

solution *min = new solution();
std::mutex sol_mtx;

//Store map of all possible trees for N leaves
std::unordered_map<long long, struct tnode *> trees;

//Func prototypes
std::string explain_tree(struct tnode *tree, const std::vector<ARITH_OP> &ops,
                const std::vector<int> &numbers, int *num_index, int *op_index);
long double execute_tree(struct tnode *tree, const std::vector<ARITH_OP> &ops,
                const std::vector<int> &numbers, int *num_index, int *op_index);
void permutateTreeOptions(struct tnode *tree, std::vector<int> numbers);
void buildAllTrees(struct tnode *node);


void usage()
{
    std::cerr << "Usage: ./countdown [ input numbers ] [ target ]" << std::endl;
}

//Parse inputs to program
bool parseInputs(int argc, char *argv[])
{
    for(int i=1; i<argc-1;++i){
        int a = atoi(argv[i]);
        numbers.push_back(a);
    }
    target = atoi(argv[argc-1]);
    return(numbers.size() > 1);
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
    //get input numbers and target
    if(!parseInputs(argc, argv)){
        usage();
        exit(EXIT_FAILURE);
    }

    //Build all tree permutations with N-1 leaves
    struct tnode *root_node = new struct tnode;
    buildAllTrees(root_node);

    std::cout << "Tree permutations: " << trees.size() << std::endl;
    std::cout << "Target number: " << target << std::endl;

    //for each tree permutation launch brute force for numbers and arith ops
    std::vector< std::future<void> > promises;
    for(auto tree: trees){
        promises.emplace_back( std::async(std::launch::any, permutateTreeOptions, tree.second, numbers) );
    }
    // for(auto it: promises){
    for(size_t i=0; i<promises.size(); ++i){
        promises[i].get();
    }

    std::cout << "Number of trees: " << trees.size() << std::endl;

    //assert atleast 1 tree perm wa made and min stuct has been set
    assert(trees.size() > 0);
    assert(min->diff >= 0.0);

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
    std::cout << "Explanation:" << std::endl;

    std::string calc_explained = explain_tree(min->tree, min->ops, min->numbers, new int(0), new int(0));
    std::cout << calc_explained << " = " << min->answer << std::endl;

    return EXIT_SUCCESS;
}


void permutateTreeOptions(struct tnode *tree, std::vector<int> numbers)
{
    sort(numbers.begin(), numbers.end());
    //for permutations of tree operations
    size_t tree_nodes = tree->nodes();

    //create vector of operations and fill with first op
    std::vector<ARITH_OP> ops;
    for(size_t i = 0; i < tree_nodes; i++){
        ops.push_back( static_cast<ARITH_OP>(0) );
    }

    //while iterated over all of the tree nodes
    for(size_t j=0; j < pow(NUM_OPS, tree_nodes); j++){

        //set arith ops for incrementing j, should overflow counter to base num_ops
        ops[0] = static_cast<ARITH_OP>(j % NUM_OPS);
        for(size_t i=1; i < tree_nodes; i++){
           int c = ( j / static_cast<size_t>( pow(NUM_OPS, i) )  ) % NUM_OPS;
           ops[i] = static_cast<ARITH_OP>(c);
        }

        do{
            //execute tree + airth options + numbers perm
            long double ret = execute_tree(tree, ops, numbers, new int(0), new int(0));
            if(!std::isfinite(ret))
                continue;

            long double diff = fabs(ret - target);

            //check for overflow
            assert(diff >= 0.0);

            //check if another thread has found the answer
            if(min->diff == 0.0){
                return;
            }
            if(diff < min->diff || min->diff < 0.0){
                std::lock_guard<std::mutex> lock(sol_mtx);
                if(diff < min->diff || min->diff < 0.0){
                    //copy new solution into min solution
                    min = new solution({ .tree = tree, .numbers = numbers, .ops = ops, .answer=ret, .diff=diff });
                }
            }
        }while(next_permutation(numbers.begin(), numbers.end()));
    }
}

long double execute_tree(struct tnode *tree, const std::vector<ARITH_OP> &ops,
                const std::vector<int> &numbers, int *num_index, int *op_index)
{
    //set ops to tree nodes
    long double left=0,right=0,result=0;

    if(tree->left != NULL)
        left = execute_tree(tree->left, ops, numbers, num_index, op_index);
    else{
        left = numbers[*num_index];
        (*num_index)++;
    }
    
    if(tree->right != NULL)
        right = execute_tree(tree->right, ops, numbers, num_index, op_index);
    else{
        right = numbers[*num_index];
        (*num_index)++;
    }

    /* we now have our first non ignore operator    */
    switch(ops[*op_index]){
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
    (*op_index)++;
    return result;
}

std::string explain_tree(struct tnode *tree, const std::vector<ARITH_OP> &ops,
                const std::vector<int> &numbers, int *num_index, int *op_index)
{
    //set ops to tree nodes
    std::string left,right,result;

    if(tree->left != NULL)
        left = explain_tree(tree->left, ops, numbers, num_index, op_index);
    else{
        left = std::to_string( numbers[*num_index] );
        (*num_index)++;
    }
    
    if(tree->right != NULL)
        right = explain_tree(tree->right, ops, numbers, num_index, op_index);
    else{
        right = std::to_string( numbers[*num_index] );
        (*num_index)++;
    }

    /* we now have our first non ignore operator    */
    switch(ops[*op_index]){
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
        result = "( " + left + " ^ " + right + " )";
        break;
    }
    (*op_index)++;
    return result;
}
