#pragma once
#include <memory>
#include <utility>

namespace NAvlTree {

namespace NAvlTreeInternal {

template<typename K>
struct TNode {
    TNode()
        : H(1)
    {
    }
    virtual K& Key() = 0;
    virtual ~TNode() {
    }
public:
    size_t H;
    std::shared_ptr<TNode<K> > Left;
    std::shared_ptr<TNode<K> > Right;
    std::shared_ptr<TNode<K> > Parent;
};

template<typename K>
struct TSimpleNode: public TNode<K> {
    TSimpleNode(const K& key)
        : StoredKey(key)
    {
    }
    virtual ~TSimpleNode() {
    }
    virtual K& Key() {
        return StoredKey;
    }
public:
    K StoredKey;
};

template<typename K, typename V>
struct TValuedNode: public TNode<K> {
    TValuedNode(const std::pair<K, V>& element)
        : Element(element)
    {
    }
    virtual ~TValuedNode() {
    }
    virtual K& Key() {
        return Element.first;
    }
public:
    std::pair<K, V> Element;
};

template<typename N, bool IsMulti>
class TAvlTree {
public:
    bool Add(N newNode) {
        if (!Root) {
            Root = newNode;
            return true;
        }
        return Insert(Root, newNode);
    }
    bool Insert(N node, N newNode) {
        bool result = false;
        if (!IsMulti && node->Key() == newNode->Key()) {
            return result;
        }
        if (newNode->Key() > node->Key()) {
            if (!node->Right) {
                node->Right = newNode;
                newNode->Parent = node;
                UpdateHeight(node);
                result = true;
            } else {
                result = Insert(node->Right, newNode);
            }
        } else {
            if (!node->Left) {
                node->Left = newNode;
                newNode->Parent = node;
                UpdateHeight(node);
                result = true;
            } else {
                result = Insert(node->Left, newNode);
            }
        }
        if (node->Parent) {
            Balance(node->Parent);
        }
        return result;
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
        if (!node || node->Key() == value) {
            return node;
        }
        if (value > node->Key()) {
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
    typedef NAvlTreeInternal::TNode<K> TNode;
    typedef NAvlTreeInternal::TSimpleNode<K> TSetNode;
    typedef std::shared_ptr<TNode> TSetNodeRef;
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
            return Node->Key();
        }
        const K* operator ->() const {
            return &Node->Key();
        }
    private:
        TSetNodeRef Node;
        TTree* Tree;
    };
public:
    std::pair<iterator, bool> insert(const K& element) {
        TSetNodeRef node = std::make_shared<TSetNode>(element);
        if (Tree.Add(node)) {
            return std::pair<iterator, bool>(iterator(&Tree, node), true);
        }
        return std::pair<iterator, bool>(iterator(), false);
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

// todo: think about merging same code from set and map

// map

template <typename K, typename V>
class map {
    typedef NAvlTreeInternal::TNode<K> TNode;
    typedef NAvlTreeInternal::TValuedNode<K, V> TMapNode;
    typedef std::shared_ptr<TNode> TMapNodeRef;
    typedef NAvlTreeInternal::TAvlTree<TMapNodeRef, false> TTree;
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
        iterator(TTree* tree, TMapNodeRef node)
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
        std::pair<K, V>& operator *() {
            return MapNode()->Element;
        }
        std::pair<K, V>* operator ->() {
            return &MapNode()->Element;
        }
    private:
        TMapNode* MapNode() {
            return static_cast<TMapNode*>(Node.get());
        }
    private:
        TMapNodeRef Node;
        TTree* Tree;
    };
public:
    std::pair<iterator, bool> insert(const std::pair<K, V>& element) {
        TMapNodeRef node = std::make_shared<TMapNode>(element);
        if (Tree.Add(node)) {
            return std::pair<iterator, bool>(iterator(&Tree, node), true);
        }
        return std::pair<iterator, bool>(iterator(), false);
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
    V& operator [] (const K& key) {
        iterator it = find(key);
        if (it == end()) {
            std::pair<iterator, bool> inserted = insert(std::pair<K, V>(key, V()));
            it = inserted.first;
        }
        return it->second;
    }
private:
    TTree Tree;
};

} // NAvlTree
