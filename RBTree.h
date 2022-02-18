#include <initializer_list>
#include <algorithm>

using namespace std;

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

    Set(initializer_list<T> init) {
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
		swap(rootNode_, other.rootNode_);
	}

	Set<T>& operator=(Set<T>&& other) {
		sz_ = other.sz_;
		swap(rootNode_, other.rootNode_);
		return *this;
	}

	~Set() {
		delete rootNode_;
	}

	int size() const {
		return sz_;
	}

	bool empty() const {
		return sz_ == 0;
	}

	void insert(T val) {
		if (insert_(rootNode_, val))
			++sz_;
		while (rootNode_->p)
			rootNode_ = rootNode_->p;
	}

	void erase(T val) {
		if (erase_(rootNode_, val))
			--sz_;
		if (sz_ == 0)
			rootNode_ = nullptr;
		else {
			while (rootNode_->p)
				rootNode_ = rootNode_->p;
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
			if (node->r) {
				node = node->r;
				while (node->l)
					node = node->l;
				return *this;
			}
			while (node->p)
				if (node == node->p->r) {
					node = node->p;
				}
				else {
					node = node->p;
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
				while (node->r)
					node = node->r;
				return *this;
			}
			if (node->l) {
				node = node->l;
				while (node->r)
					node = node->r;
				return *this;
			}
			while (node->p)
				if (node == node->p->l) {
					node = node->p;
				}
				else {
					node = node->p;
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
		while (node && node->l)
			node = node->l;
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

	int sz_ = 0;
	RBTreeNode* rootNode_ = nullptr;

	struct RBTreeNode {
		// 0 - black
		// 1 - red
		bool color;
		T val;
		RBTreeNode* l = nullptr;
		RBTreeNode* r = nullptr;
		RBTreeNode* p = nullptr;

		RBTreeNode() {}

		RBTreeNode(T val_) {
			val = val_;
		}

		RBTreeNode(const RBTreeNode& other) {
			color = other.color;
			val = other.val;
			if (other.l) {
				l = new RBTreeNode(*other.l);
				l->p = this;
			}
			if (other.r) {
				r = new RBTreeNode(*other.r);
				r->p = this;
			}
		}

		~RBTreeNode() {
			delete l;
			delete r;
		}
	};

	RBTreeNode* get_grandparent(RBTreeNode* root) {
		if (!root || !root->p)
			return nullptr;
		return root->p->p;
	}

	RBTreeNode* get_uncle(RBTreeNode* root) {
		auto g = get_grandparent(root);
		if (!g)
			return nullptr;
		return (root->p == g->l) ? (g->r) : (g->l);
	}

	RBTreeNode* get_sibling(RBTreeNode* root, RBTreeNode* p) {
		return (root == p->l) ? (p->r) : (p->l);
	}

	void rotate(RBTreeNode* pivot) {
		RBTreeNode* old_p = pivot->p;
		pivot->p = old_p->p;
		if (pivot->p) {
			if (pivot->p->l == old_p)
				pivot->p->l = pivot;
			else
				pivot->p->r = pivot;
		}
		if (pivot == old_p->r) {
			old_p->r = pivot->l;
			if (old_p->r)
				pivot->l->p = old_p;
			pivot->l = old_p;
		}
		else {
			old_p->l = pivot->r;
			if (old_p->l)
				pivot->r->p = old_p;
			pivot->r = old_p;
		}
		old_p->p = pivot;
	}

	void rotate_left(RBTreeNode* root) {
		rotate(root->r);
		return;
	}

	void rotate_right(RBTreeNode* root) {
		rotate(root->l);
		return;
	}

	void insert_fix(RBTreeNode* root) {
		if (!root->p)
			root->color = false;
		else if (root->p->color == false)
			return;
		else {
			RBTreeNode* u = get_uncle(root);
			RBTreeNode* g = get_grandparent(root);

			if (u && u->color) {
				root->p->color = false;
				u->color = false;
				g->color = true;
				insert_fix(g);
			}
			else {
				if ((root == root->p->r) && (root->p == g->l)) {
					rotate(root);
					root = root->l;
				}
				else if ((root == root->p->l) && (root->p == g->r)) {
					rotate(root);
					root = root->r;
				}
				root->p->color = false;
				root->p->p->color = true;
				rotate(root->p);
			}
		}
	}

	bool insert_(RBTreeNode*& root, T val, RBTreeNode* p = nullptr) {
		if (!root) {
			root = new RBTreeNode(val);
			root->color = true;
			root->p = p;
			insert_fix(root);
			return true;
		}
		if (val < root->val)
			return insert_(root->l, val, root);
		if (root->val < val)
			return insert_(root->r, val, root);
		return false;
	};

	void replace_node(RBTreeNode* old, RBTreeNode* next) {
		if (next)
			next->p = old->p;
		if (old->p) {
			if (old == old->p->l)
				old->p->l = next;
			else
				old->p->r = next;
		}
	}

	void delete_fix(RBTreeNode* root, RBTreeNode* p) {
		if (p) {
			RBTreeNode* s = get_sibling(root, p);
			if (get_color(s)) {
				p->color = true;
				s->color = false;
				rotate(s);
			}

			s = get_sibling(root, p);

			if ((!p->color) &&
				(!s->color) &&
				(!get_color(s->l)) &&
				(!get_color(s->r))) {
				s->color = true;
				delete_fix(p, p->p);
			}
			else {
				s = get_sibling(root, p);

				if ((p->color) &&
					(!s->color) &&
					(!get_color(s->l)) &&
					(!get_color(s->r))) {
					s->color = true;
					p->color = false;
				}
				else {
					s = get_sibling(root, p);

					if (!s->color) {
						if ((root == p->l) &&
							(!get_color(s->r)) &&
							(get_color(s->l))) {
							s->color = true;
							s->l->color = false;
							rotate_right(s);
						}
						else if ((root == p->r) &&
							(!get_color(s->l)) &&
							(get_color(s->r))) {
							s->color = true;
							s->r->color = false;
							rotate_left(s);
						}
					}
					s = get_sibling(root, p);

					s->color = p->color;
					p->color = false;

					if (root == p->l) {
						s->r->color = false;
						rotate_left(p);
					}
					else {
						s->l->color = false;
						rotate_right(p);
					}
				}
			}
		}
	}

	bool get_color(RBTreeNode* root) {
		return root ? root->color : false;
	}

	void delete_with_one_child(RBTreeNode* root) {
		RBTreeNode* child = root->r ? root->r : root->l;

		replace_node(root, child);
		if (root->color == false) {
			if (get_color(child))
				child->color = false;
			else
				delete_fix(child, root->p);
		}
		root->l = nullptr;
		root->r = nullptr;
		delete root;
	}

	bool erase_(RBTreeNode* root, T val) {
		while (root) {
			if (val < root->val)
				root = root->l;
			else if (root->val < val)
				root = root->r;
			else
				break;
		}
		if (!root)
			return false;
		if (root->l) {
			RBTreeNode* replace = root->l;
			while (replace->r)
				replace = replace->r;
			root->val = replace->val;
			delete_with_one_child(replace);
		}
		else if (root->r) {
			RBTreeNode* replace = root->r;
			while (replace->l)
				replace = replace->l;
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
			return lower_bound_(root->r, val);
		auto left = lower_bound_(root->l, val);
		if (left && left->val < root->val)
			return left;
		return root;
	}
};
