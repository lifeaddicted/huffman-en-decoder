#ifndef __BINARY_TREE_H__
#define __BINARY_TREE_H__

template <class T>
struct TNode
{
    T val;
    struct TNode *left{nullptr};
    struct TNode *right{nullptr};
    TNode(){};
};

template <class T>
class Binary_Tree
{
private:
    TNode<T> *root = nullptr;

public:
    Binary_Tree(){};
    ~Binary_Tree() { desconstruct(); };
    void desconstruct();
    void desconstruct2(TNode<T> *root);

    TNode<T> *new_node(T &val);
    void update_root(TNode<T> *root);
    const TNode<T> *get_root() { return root; };
};

/*******************************************************************************
* functionality:
        new a TNode, init it with an existing val
* args:
*       a reference to val type
*******************************************************************************/
template <class T>
TNode<T> *Binary_Tree<T>::new_node(T &val)
{
    TNode<T> *node = new TNode<T>;
    node->val = val;
    node->left = nullptr;
    node->right = nullptr;
    return node;
}

/*******************************************************************************
* functionality:
        update the root node of the tree
* args:
* 		root: new root pointer
*******************************************************************************/
template <class T>
void Binary_Tree<T>::update_root(TNode<T> *root)
{
    this->root = root;
    return;
}

template <class T>
void Binary_Tree<T>::desconstruct()
{
    if (this->root == nullptr)
        return;
    desconstruct2(this->root);
    //    delete this->root;
    return;
}

template <class T>
void Binary_Tree<T>::desconstruct2(TNode<T> *root)
{
    if (root->left == nullptr && root->right == nullptr)
    {
        // printf("delete: %p\n", root);
        delete root;
        return;
    }
    desconstruct2(root->left);
    // root->left = nullptr;
    desconstruct2(root->right);
    // root->right = nullptr;
    // printf("delete: %p\n", root);
    delete root;
    return;
}

#endif