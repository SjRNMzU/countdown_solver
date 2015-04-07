#ifndef __BIN_TREE__
#define __BIN_TREE__

#include <string>
#include <unistd.h>

struct tnode {
    struct tnode *parent = NULL;
    struct tnode *right = NULL;
    struct tnode *left = NULL;
    uint32_t pos = 0;
    int lv = 1;

    tnode(tnode *_parent){ this->parent = _parent; this->lv = _parent->lv + 1; };
    tnode(){};

    //count the number of leaves from this node downwards
    size_t leaves(void) const
    {
        size_t lsum = 0, rsum = 0;
        //left or right is a tnode, or increment count
        if(this->right != NULL)
            rsum = this->right->leaves();
        else
            rsum = 1;

        if(this->left != NULL)
            lsum = this->left->leaves();
        else
            lsum = 1;

        return rsum + lsum;
    };

    //return number of nodes in tree form this node downwards
    size_t nodes(void) const
    {
        size_t lsum = 0, rsum = 0;
        //left or right is a tnode, or increment count
        if(this->right != NULL)
            rsum = this->right->nodes();

        if(this->left != NULL)
            lsum = this->left->nodes();

        return rsum + lsum + 1;
    };

    //return max depth from this node
    size_t depth(void) const
    {
        size_t ld = 0, rd = 0;
        //left or right is a tnode, or increment count
        if(this->right != NULL)
            rd = this->right->depth();

        if(this->left != NULL)
            ld = this->left->depth();

        return 1 + ( (rd > ld) ? rd : ld );
    };



    //return pointer to root of this tree
    struct tnode* root(void) const
    {
        struct tnode *a = this->copy();
        while(a->parent != NULL){
            a = a->parent;
        }
        return a;
    };

    //Get size of tree form this node downwards in bytes
    size_t size(void) const
    {
        return this->nodes() * sizeof(struct tnode);
    };

    //return copy of this node
    struct tnode* copy(void) const
    {
        struct tnode *new_node = (struct tnode *) malloc(sizeof(struct tnode));
        assert(new_node != NULL);
        // memcpy(new_node, &(*this), sizeof(struct tnode));
        memcpy(new_node, &(*this), sizeof(struct tnode));
        return new_node;
    };

    //deep copy of tree structure from this node downwards
    struct tnode* copy_tree(struct tnode *_parent = NULL) const
    {
        //copy node and set parent recursivly for all left and right nodes
        struct tnode *nnode = this->copy();
        nnode->parent = _parent;
        if(this->right != NULL)
            nnode->right = nnode->right->copy_tree(nnode);

        if(this->left != NULL)
            nnode->left = nnode->left->copy_tree(nnode);

        return nnode;
    };

    //transverse to a relative location in the tree
    struct tnode* transverse(uint32_t pos, int lv) const
    {
        // int i = __builtin_clz(pos); //count leading zeros
        int i = 32 - (lv - 1);
        struct tnode *tmp = const_cast<struct tnode *>(this);
        //while not end
        while(i < 32 ){ //loop through levels
            //Get single bit of step i, pushes bit to MSB then MSB to LSB
            uint32_t d = ( pos << i ) >> ( (sizeof(pos)*8)  - 1);

            if(d == 1){
                //go right
                assert(tmp->right != NULL);
                tmp = tmp->right;
            }else{
                //go left
                assert(d == 0);
                assert(tmp->left != NULL);
                tmp = tmp->left;
            }
            i++;
        }
        return tmp;
    };

    //return long long hash of tree structure
    //i.e. uniquly quantify different tree structures
    long long hash(void) const
    {
        std::hash<long long> hash_fn;
        long long lhash = 0, rhash = 0;
        long long i = (0xFAAE * this->lv) + this->pos;
        if(this->right != NULL){
            i *= 0xF;
            rhash = this->right->hash();
        }
        if(this->left != NULL){
            i /= 0xF;
            lhash = this->left->hash();
        }

        return hash_fn( hash_fn(i) + hash_fn(lhash) + hash_fn(rhash) );
    };

    //print tree structure
    const std::string print(void) const
    {
        std::string tree = "";
        //build as a set of square char blocks
        size_t max_depth = this->depth();
        //3 chars for each left/right, *2 for both directions, +1 for terminating
        //char, +1 to make odd (symmetric)
        size_t max_width = (2 * max_depth * 3) + 1 + 1;

        //taking into account lines between levels
        char *lines[max_depth*2];
        for(size_t i=0;i<max_depth*2;i++){
            lines[i] = (char *)malloc(max_width + 1);
            memset(lines[i], 32, max_width+1); // 32 is ascii for space
            lines[i][max_width+1] = '\0';
        }

        size_t center = max_width / 2;
        buildAscii(lines, center, max_depth, this);
        for(char *line: lines){
            tree += std::string(line) + std::string("\n");
            free(line);
        }
        return tree;
    };

    private:
    static void buildAscii( char **lines, size_t column, size_t max_depth, const struct tnode *node )
    {
        size_t lv = node->lv - 1;
        lines[2*lv][column] = 'o';

        //work out new center column
        size_t span = ( (max_depth - lv) * 2 ) + 1;
        span = span / 2;
        size_t center = span / 2;

        if(node->left != NULL){
            lines[(2*lv)+1][column-center] = '/';
            buildAscii( lines, column - span, max_depth, node->left );
        }
        if(node->right != NULL){
            lines[(2*lv)+1][column+center] = '\\';
            buildAscii( lines, column + span, max_depth, node->right );
        }
    };
};
#endif
