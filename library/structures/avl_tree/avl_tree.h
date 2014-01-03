#pragma once
#include <memory>

namespace NAvlTree {

namespace NAvlTreeInternal {

template<typename K>
struct TNode {
    TNode(const K& key) // todo: move semantics here
        : Key(key)
        , H(1)
    {
    }
public:
    K Key;
    size_t H;
    std::shared_ptr<TNode<K> > Left;
    std::shared_ptr<TNode<K> > Right;
    std::shared_ptr<TNode<K> > Parent;
};

template<typename K, typename V>
struct TValuedNode: public TNode<K> {
    TValuedNode(const K& key, const V& value)
        : TNode<K>(key)
        , Value(value)
    {
    }
public:
    V Value;
};

template<typename N, bool IsMulti>
class TAvlTree {
public:
    void Add(N newNode) {
        if (!Root) {
            Root = newNode;
        } else {
            Insert(Root, newNode);
        }
    }
    void Insert(N node, N newNode) {
        if (!IsMulti && node->Key == newNode->Key) {
            return;
        }
        if (newNode->Key > node->Key) {
            if (!node->Right) {
                node->Right = newNode;
                newNode->Parent = node;
                UpdateHeight(node);
            } else {
                Insert(node->Right, newNode);
            }
        } else {
            if (!node->Left) {
                node->Left = newNode;
                newNode->Parent = node;
                UpdateHeight(node);
            } else {
                Insert(node->Left, newNode);
            }
        }
        if (node->Parent) {
            Balance(node->Parent);
        }
    }
    void Balance(N node) {
        int leftH = node->Left ? node->Left->H : 0;
        int rightH = node->Right ? node->Right->H : 0;
        if (leftH - rightH == 2) {
            RotateRight(node);
        } else if (rightH - leftH == 2) {
            RotateLeft(node);
        }
    }
    void ReplaceChild(N oldChild, N newChild) {
        N parent = oldChild->Parent;
        if (!parent) {
            Root = newChild;
        } else {
            if (parent->Left == oldChild) {
                parent->Left = newChild;
            } else {
                parent->Right = newChild;
            }
        }
        oldChild->Parent = newChild;
        newChild->Parent = parent;
    }
    void UpdateHeight(N x) {
        if (!x) {
            return;
        }
        if (x->Left && x->Right) {
            x->H = std::max(x->Left->H, x->Right->H) + 1;
        } else if (x->Left) {
            x->H = x->Left->H + 1;
        } else if (x->Right) {
            x->H = x->Right->H + 1;
        } else {
            x->H = 1;
        }
        UpdateHeight(x->Parent);
    }
    void RotateRight(N x) {
        N y = x->Left;
        N beta = y->Right;
        ReplaceChild(x, y);
        x->Left = beta;
        y->Right = x;
        if (x->Left) {
            x->Left->Parent = x;
        }
        UpdateHeight(x);
        UpdateHeight(y);
        UpdateHeight(y->Parent);
    }
    void RotateLeft(N y) {
        N x = y->Right;
        N beta = x->Left;
        ReplaceChild(y, x);
        x->Left = y;
        y->Right = beta;
        if (y->Right) {
            y->Right->Parent = y;
        }
        UpdateHeight(y);
        UpdateHeight(x);
        UpdateHeight(x->Parent);
    }
    template<typename K>
    N Find(const K& value) {
        return FindNode(Root, value);
    }
    template<typename K>
    N FindNode(N node, const K& value) {
        if (!node || node->Key == value) {
            return node;
        }
        if (value > node->Key) {
            return FindNode(node->Right, value);
        }
        return FindNode(node->Left, value);
    }
    N FindMin() {
        N node = Root;
        N prev = node;
        while (node) {
            prev = node;
            node = node->Left;
        }
        return prev;
    }
    N Leftest(N node) {
        if (node->Left) {
            return Leftest(node->Left);
        }
        return node;
    }
    N Rightest(N node) {
        if (node->Right) {
            return Rightest(node->Right);
        }
        return node;
    }
    N UpLeft(N node) {
        if (!node->Parent || node->Parent->Left == node) {
            return node->Parent;
        }
        return UpLeft(node->Parent);
    }
    N UpRight(N node) {
        if (!node->Parent || node->Parent->Right == node) {
            return node->Parent;
        }
        return UpRight(node->Parent);
    }
    N Next(N node) {
        if (node->Right) {
            return Leftest(node->Right);
        }
        return UpLeft(node);
    }
    N Previous(N node) {
        if (node->Left) {
            return Rightest(node->Left);
        }
        return UpRight(node);
    }

private:
    N Root;
};

} // NAvlTreeIntenal



// set

template <typename K>
class set {
    typedef NAvlTreeInternal::TNode<K> TSetNode;
    typedef std::shared_ptr<TSetNode> TSetNodeRef;
    typedef NAvlTreeInternal::TAvlTree<TSetNodeRef, false> TTree;
public:
    class iterator {
    public:
        iterator()
            : Tree(nullptr)
        {
        }
        iterator(TTree* tree)
            : Tree(tree)
        {
        }
        iterator(TTree* tree, TSetNodeRef node)
            : Node(node)
            , Tree(tree)
        {
        }
        iterator(const iterator& it)
            : Node(it.Node)
            , Tree(it.Tree)
        {
        }
        bool operator ==(const iterator& second) const {
            return Node == second.Node;
        }
        bool operator !=(const iterator& second) const {
            return Node != second.Node;
        }
        iterator& operator ++() {
            Node = Tree->Next(Node);
            return *this;
        }
        iterator& operator --() {
            Node = Tree->Previous(Node);
            return *this;
        }
        const K& operator *() const {
            return Node->Key;
        }
    private:
        TSetNodeRef Node;
        TTree* Tree;
    };
public:
    void insert(const K& element) {
        Tree.Add(std::make_shared<TSetNode>(element));
    }
    iterator find(const K& element) {
        return iterator(&Tree, Tree.Find(element));
    }
    iterator begin() {
        return iterator(&Tree, Tree.FindMin());
    }
    iterator end() {
        return iterator();
    }
private:
    TTree Tree;
};

} // NAvlTree
