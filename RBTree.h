#include <initializer_list>
#include <algorithm>

template<class T>
class Set {
    struct RBTreeNode;

public:
    class iterator;

    Set() {}
    template<typename InputIt>
    Set(InputIt first, InputIt last) {
        while (first != last) {
            insert(*first);
            ++first;
        }
    }

    Set(std::initializer_list<T> init) {
        *this = Set<T>(init.begin(), init.end());
    }

    Set(const Set<T>& other) {
        if (other.rootNode_)
            rootNode_ = new RBTreeNode(*other.rootNode_);
        sz_ = other.sz_;
    }

    Set<T>& operator=(const Set<T>& other) {
        if (this != &other) {
            Set<T> tmp = Set<T>(other);
            *this = std::move(tmp);
        }
        return *this;
    }

    Set(Set<T>&& other) {
        sz_ = other.sz_;
        std::swap(rootNode_, other.rootNode_);
    }

    Set<T>& operator=(Set<T>&& other) {
        sz_ = other.sz_;
        std::swap(rootNode_, other.rootNode_);
        return *this;
    }

    ~Set() {
        delete rootNode_;
    }

    size_t size() const {
        return sz_;
    }

    bool empty() const {
        return sz_ == 0;
    }

    void insert(T val) {
        if (insert_(rootNode_, val))
            ++sz_;
        while (rootNode_->parent)
            rootNode_ = rootNode_->parent;
    }

    void erase(T val) {
        if (erase_(rootNode_, val))
            --sz_;
        if (sz_ == 0)
            rootNode_ = nullptr;
        else {
            while (rootNode_->parent)
                rootNode_ = rootNode_->parent;
        }
    }

    class iterator {
        const Set<T>* tree;
        const Set<T>::RBTreeNode* node;

    public:
        iterator() :
            tree(nullptr),
            node(nullptr)
        {};

        iterator(const Set<T>* tree_, const Set<T>::RBTreeNode* node_) :
            tree(tree_),
            node(node_)
        {};

        const T& operator*() const {
            return node->val;
        }

        const T* operator->() const {
            return &(node->val);
        }

        bool operator==(const iterator& other) const {
            return tree == other.tree && node == other.node;
        }

        bool operator!=(const iterator& other) const {
            return !operator==(other);
        }

        iterator operator++() {
            if (node->right) {
                node = node->right;
                while (node->left)
                    node = node->left;
                return *this;
            }
            while (node->parent)
                if (node == node->parent->right) {
                    node = node->parent;
                }
                else {
                    node = node->parent;
                    return *this;
                }
            node = nullptr;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = iterator(tree, node);
            operator++();
            return tmp;
        }

        iterator operator--() {
            if (!node) {
                node = tree->rootNode_;
                while (node->right)
                    node = node->right;
                return *this;
            }
            if (node->left) {
                node = node->left;
                while (node->right)
                    node = node->right;
                return *this;
            }
            while (node->parent)
                if (node == node->parent->left) {
                    node = node->parent;
                }
                else {
                    node = node->parent;
                    return *this;
                }
            node = nullptr;
            return *this;
        }

        iterator operator--(int) {
            iterator tmp = iterator(tree, node);
            operator--();
            return tmp;
        }
    };

    iterator begin() const {
        auto node = rootNode_;
        while (node && node->left)
            node = node->left;
        return iterator(this, node);
    }

    iterator end() const {
        return iterator(this, nullptr);
    }

    iterator lower_bound(T val) const {
        return iterator(this, lower_bound_(rootNode_, val));
    }

    iterator find(T val) const {
        const RBTreeNode* ptr = lower_bound_(rootNode_, val);
        if (ptr && (!(ptr->val < val) && !(val < ptr->val)))
            return iterator(this, ptr);
        return end();
    }

private:

    size_t sz_ = 0;
    RBTreeNode* rootNode_ = nullptr;

    struct RBTreeNode {
        // 0 - black
        // 1 - red
        bool color;
        T val;
        RBTreeNode* left = nullptr;
        RBTreeNode* right = nullptr;
        RBTreeNode* parent = nullptr;

        RBTreeNode() {}

        RBTreeNode(T val_) {
            val = val_;
        }

        RBTreeNode(const RBTreeNode& other) {
            color = other.color;
            val = other.val;
            if (other.left) {
                left = new RBTreeNode(*other.left);
                left->parent = this;
            }
            if (other.right) {
                right = new RBTreeNode(*other.right);
                right->parent = this;
            }
        }

        ~RBTreeNode() {
            delete left;
            delete right;
        }
    };

    RBTreeNode* get_grandparent(RBTreeNode* root) {
        if (!root || !root->parent)
            return nullptr;
        return root->parent->parent;
    }

    RBTreeNode* get_uncle(RBTreeNode* root) {
        auto grandparent = get_grandparent(root);
        if (!grandparent)
            return nullptr;
        return (root->parent == grandparent->left) ? (grandparent->right) : (grandparent->left);
    }

    RBTreeNode* get_sibling(RBTreeNode* root, RBTreeNode* parent) {
        return (root == parent->left) ? (parent->right) : (parent->left);
    }

    void rotate(RBTreeNode* pivot) {
        RBTreeNode* old_p = pivot->parent;
        pivot->parent = old_p->parent;
        if (pivot->parent) {
            if (pivot->parent->left == old_p)
                pivot->parent->left = pivot;
            else
                pivot->parent->right = pivot;
        }
        if (pivot == old_p->right) {
            old_p->right = pivot->left;
            if (old_p->right)
                pivot->left->parent = old_p;
            pivot->left = old_p;
        }
        else {
            old_p->left = pivot->right;
            if (old_p->left)
                pivot->right->parent = old_p;
            pivot->right = old_p;
        }
        old_p->parent = pivot;
    }

    void rotate_left(RBTreeNode* root) {
        rotate(root->right);
    }

    void rotate_right(RBTreeNode* root) {
        rotate(root->left);
    }

    void insert_fix(RBTreeNode* root) {
        if (!root->parent) {
            root->color = false;
            return;
        }
        if (root->parent->color == false)
            return;

        RBTreeNode* uncle = get_uncle(root);
        RBTreeNode* grandparent = get_grandparent(root);

        if (uncle && uncle->color) {
            root->parent->color = false;
            uncle->color = false;
            grandparent->color = true;
            insert_fix(grandparent);
        }
        else {
            if ((root == root->parent->right) && (root->parent == grandparent->left)) {
                rotate(root);
                root = root->left;
            }
            else if ((root == root->parent->left) && (root->parent == grandparent->right)) {
                rotate(root);
                root = root->right;
            }
            root->parent->color = false;
            root->parent->parent->color = true;
            rotate(root->parent);
        }
    }

    bool insert_(RBTreeNode*& root, T val, RBTreeNode* parent = nullptr) {
        if (!root) {
            root = new RBTreeNode(val);
            root->color = true;
            root->parent = parent;
            insert_fix(root);
            return true;
        }
        if (val < root->val)
            return insert_(root->left, val, root);
        if (root->val < val)
            return insert_(root->right, val, root);
        return false;
    };

    void replace_node(RBTreeNode* old, RBTreeNode* next) {
        if (next)
            next->parent = old->parent;
        if (old->parent) {
            if (old == old->parent->left)
                old->parent->left = next;
            else
                old->parent->right = next;
        }
    }

    void delete_fix(RBTreeNode* root, RBTreeNode* parent) {
        if (!parent)
            return;
        delete_fix_case1(root, parent);
    }

    void delete_fix_case1(RBTreeNode* root, RBTreeNode* parent) {
        RBTreeNode* sibling = get_sibling(root, parent);
        if (get_color(sibling)) {
            parent->color = true;
            sibling->color = false;
            rotate(sibling);
        }
        delete_fix_case2(root, parent);
    }

    void delete_fix_case2(RBTreeNode* root, RBTreeNode* parent) {
        RBTreeNode* sibling = get_sibling(root, parent);
        if ((!parent->color) &&
            (!sibling->color) &&
            (!get_color(sibling->left)) &&
            (!get_color(sibling->right)))
        {
            sibling->color = true;
            delete_fix(parent, parent->parent);
        }
        else {
            delete_fix_case3(root, parent);
        }
    }

    void delete_fix_case3(RBTreeNode* root, RBTreeNode* parent) {
        RBTreeNode* sibling = get_sibling(root, parent);
        if ((parent->color) &&
            (!sibling->color) &&
            (!get_color(sibling->left)) &&
            (!get_color(sibling->right)))
        {
            sibling->color = true;
            parent->color = false;
        }
        else {
            delete_fix_case4(root, parent);
        }
    }

    void delete_fix_case4(RBTreeNode* root, RBTreeNode* parent) {
        RBTreeNode* sibling = get_sibling(root, parent);

        if (!sibling->color) {
            if ((root == parent->left) &&
                (!get_color(sibling->right)) &&
                (get_color(sibling->left))) {
                sibling->color = true;
                sibling->left->color = false;
                rotate_right(sibling);
            }
            else if ((root == parent->right) &&
                (!get_color(sibling->left)) &&
                (get_color(sibling->right)))
            {
                sibling->color = true;
                sibling->right->color = false;
                rotate_left(sibling);
            }
        }
        delete_fix_case5(root, parent);
    }

    void delete_fix_case5(RBTreeNode* root, RBTreeNode* parent) {
        RBTreeNode* sibling = get_sibling(root, parent);

        sibling->color = parent->color;
        parent->color = false;

        if (root == parent->left) {
            sibling->right->color = false;
            rotate_left(parent);
        }
        else {
            sibling->left->color = false;
            rotate_right(parent);
        }
    }

    bool get_color(RBTreeNode* root) {
        return root ? root->color : false;
    }

    void delete_with_one_child(RBTreeNode* root) {
        RBTreeNode* child = root->right ? root->right : root->left;

        replace_node(root, child);
        if (root->color == false) {
            if (get_color(child))
                child->color = false;
            else
                delete_fix(child, root->parent);
        }
        root->left = nullptr;
        root->right = nullptr;
        delete root;
    }

    bool erase_(RBTreeNode* root, T val) {
        while (root) {
            if (val < root->val)
                root = root->left;
            else if (root->val < val)
                root = root->right;
            else
                break;
        }
        if (!root)
            return false;
        if (root->left) {
            RBTreeNode* replace = root->left;
            while (replace->right)
                replace = replace->right;
            root->val = replace->val;
            delete_with_one_child(replace);
        }
        else if (root->right) {
            RBTreeNode* replace = root->right;
            while (replace->left)
                replace = replace->left;
            root->val = replace->val;
            delete_with_one_child(replace);
        }
        else {
            delete_with_one_child(root);
        }
        return true;
    };

    const RBTreeNode* lower_bound_(const RBTreeNode* root, T val) const {
        if (!root)
            return nullptr;
        if (root->val < val)
            return lower_bound_(root->right, val);
        auto left = lower_bound_(root->left, val);
        if (left && left->val < root->val)
            return left;
        return root;
    }
};
