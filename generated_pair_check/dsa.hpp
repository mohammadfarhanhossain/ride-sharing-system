#ifndef DSA_HPP
#define DSA_HPP

#include <iostream>
#include <string>
#include <limits>
#include <functional>
#include <windows.h>

namespace dsa
{
    inline char toLower(char c)
    {
        return (c >= 'A' && c <= 'Z') ? char(c + ('a' - 'A')) : c;
    }

    inline double roundNumber(double value)
    {
        return value >= 0 ? static_cast<long long>(value + 0.5) : static_cast<long long>(value - 0.5);
    }

    inline double ceilNumber(double value)
    {
        long long whole = static_cast<long long>(value);
        return value > whole ? whole + 1.0 : static_cast<double>(whole);
    }

    inline bool isFinite(double value)
    {
        return value != std::numeric_limits<double>::infinity() &&
               value != -std::numeric_limits<double>::infinity();
    }

    inline double sqrtNumber(double value)
    {
        if (value <= 0) return 0;
        double guess = value > 1 ? value : 1;
        for (int i = 0; i < 20; ++i) guess = (guess + value / guess) / 2.0;
        return guess;
    }

    template <typename T> const T &minValue(const T &a, const T &b) { return a < b ? a : b; }
    template <typename T> const T &maxValue(const T &a, const T &b) { return a > b ? a : b; }

    // ==========================================
    // 1. TEMPLATE DYNAMIC ARRAY (Vector)
    // ==========================================
    template <typename T>
    class Vector
    {
    private:
        T *data;
        size_t cap;
        size_t sz;

        void reallocate(size_t newCap)
        {
            T *newBuffer = new T[newCap];
            for (size_t i = 0; i < sz; ++i)
                newBuffer[i] = data[i];
            delete[] data;
            data = newBuffer;
            cap = newCap;
        }

    public:
        Vector() : data(nullptr), cap(0), sz(0) {}

        explicit Vector(size_t count, const T &val = T()) : data(nullptr), cap(count), sz(count)
        {
            if (count > 0)
            {
                data = new T[count];
                for (size_t i = 0; i < count; ++i)
                    data[i] = val;
            }
        }

        Vector(const Vector &other) : data(nullptr), cap(other.cap), sz(other.sz)
        {
            if (cap > 0)
            {
                data = new T[cap];
                for (size_t i = 0; i < sz; ++i)
                    data[i] = other.data[i];
            }
        }

        Vector &operator=(const Vector &other)
        {
            if (this != &other)
            {
                delete[] data;
                cap = other.cap;
                sz = other.sz;
                data = cap > 0 ? new T[cap] : nullptr;
                for (size_t i = 0; i < sz; ++i)
                    data[i] = other.data[i];
            }
            return *this;
        }

        ~Vector() { delete[] data; }

        void push_back(const T &val)
        {
            if (sz == cap)
                reallocate(cap == 0 ? 4 : cap * 2);
            data[sz++] = val;
        }

        void pop_back()
        {
            if (sz > 0)
                --sz;
        }

        void clear()
        {
            sz = 0;
        }

        void assign(size_t count, const T &val)
        {
            clear();
            if (count > cap)
                reallocate(count);
            for (size_t i = 0; i < count; ++i)
                data[i] = val;
            sz = count;
        }

        size_t size() const { return sz; }
        bool empty() const { return sz == 0; }

        T &back() { return data[sz - 1]; }
        const T &back() const { return data[sz - 1]; }

        T &operator[](size_t idx) { return data[idx]; }
        const T &operator[](size_t idx) const { return data[idx]; }

        T *begin() { return data; }
        const T *begin() const { return data; }
        T *end() { return data + sz; }
        const T *end() const { return data + sz; }

        void erase(size_t idx)
        {
            if (idx >= sz) return;
            for (size_t i = idx; i + 1 < sz; ++i)
                data[i] = data[i + 1];
            --sz;
        }
    };

    template <typename T, typename Predicate>
    size_t countIf(const T *begin, const T *end, Predicate condition)
    {
        size_t count = 0;
        for (const T *it = begin; it != end; ++it)
            if (condition(*it)) ++count;
        return count;
    }

    template <typename T, typename Compare>
    void mergeSort(T *data, size_t left, size_t right, Compare comp)
    {
        if (right - left <= 1) return;
        size_t mid = left + (right - left) / 2;
        mergeSort(data, left, mid, comp);
        mergeSort(data, mid, right, comp);
        Vector<T> merged;
        size_t i = left, j = mid;
        while (i < mid && j < right)
            if (comp(data[j], data[i])) merged.push_back(data[j++]); else merged.push_back(data[i++]);
        while (i < mid) merged.push_back(data[i++]);
        while (j < right) merged.push_back(data[j++]);
        for (size_t k = 0; k < merged.size(); ++k) data[left + k] = merged[k];
    }

    template <typename T, typename Compare>
    void mergeSort(Vector<T> &items, Compare comp) { mergeSort(items.begin(), 0, items.size(), comp); }

    // ==========================================
    // 2. TEMPLATE NODE & LINKED LIST BASED QUEUE
    // ==========================================
    template <typename T>
    struct Node
    {
        T data;
        Node *next;
        Node(const T &val) : data(val), next(nullptr) {}
    };

    template <typename T>
    class Queue
    {
    private:
        Node<T> *frontNode;
        Node<T> *rearNode;
        size_t count;

    public:
        Queue() : frontNode(nullptr), rearNode(nullptr), count(0) {}

        ~Queue() { clear(); }

        void clear()
        {
            while (!empty())
                pop();
        }

        void push(const T &val)
        {
            Node<T> *newNode = new Node<T>(val);
            if (!rearNode)
            {
                frontNode = rearNode = newNode;
            }
            else
            {
                rearNode->next = newNode;
                rearNode = newNode;
            }
            ++count;
        }

        void pop()
        {
            if (!frontNode) return;
            Node<T> *temp = frontNode;
            frontNode = frontNode->next;
            if (!frontNode) rearNode = nullptr;
            delete temp;
            --count;
        }

        T &front() { return frontNode->data; }
        const T &front() const { return frontNode->data; }

        bool empty() const { return count == 0; }
        size_t size() const { return count; }

    };

    // This project uses the queue for graph locations (BFS).
    using LocationQueue = Queue<size_t>;

    // ==========================================
    // 3. TEMPLATE LINKED LIST BASED STACK
    // ==========================================
    template <typename T>
    class Stack
    {
    private:
        Node<T> *topNode;
        size_t count;

    public:
        Stack() : topNode(nullptr), count(0) {}
        ~Stack() { clear(); }

        void clear()
        {
            while (!empty())
                pop();
        }

        void push(const T &val)
        {
            Node<T> *newNode = new Node<T>(val);
            newNode->next = topNode;
            topNode = newNode;
            ++count;
        }

        void pop()
        {
            if (!topNode) return;
            Node<T> *temp = topNode;
            topNode = topNode->next;
            delete temp;
            --count;
        }

        T &top() { return topNode->data; }
        const T &top() const { return topNode->data; }

        bool empty() const { return count == 0; }
        size_t size() const { return count; }

        Vector<T> newestFirst() const
        {
            Vector<T> result;
            Node<T> *current = topNode;
            while (current) { result.push_back(current->data); current = current->next; }
            return result;
        }
    };

    // ==========================================
    // 4. TEMPLATE PRIORITY QUEUE (Binary Heap)
    // ==========================================
    template <typename T, typename Compare = std::less<T>>
    class PriorityQueue
    {
    private:
        Vector<T> heap;
        Compare comp;

        void percolateUp(size_t idx)
        {
            while (idx > 0)
            {
                size_t parent = (idx - 1) / 2;
                if (comp(heap[parent], heap[idx]))
                {
                    std::swap(heap[parent], heap[idx]);
                    idx = parent;
                }
                else break;
            }
        }

        void percolateDown(size_t idx)
        {
            size_t n = heap.size();
            while (2 * idx + 1 < n)
            {
                size_t left = 2 * idx + 1;
                size_t right = left + 1;
                size_t target = left;
                if (right < n && comp(heap[left], heap[right]))
                    target = right;
                if (comp(heap[idx], heap[target]))
                {
                    std::swap(heap[idx], heap[target]);
                    idx = target;
                }
                else break;
            }
        }

    public:
        PriorityQueue() {}

        void push(const T &val)
        {
            heap.push_back(val);
            percolateUp(heap.size() - 1);
        }

        void pop()
        {
            if (heap.empty()) return;
            heap[0] = heap[heap.size() - 1];
            heap.pop_back();
            if (!heap.empty())
                percolateDown(0);
        }

        const T &top() const { return heap[0]; }
        bool empty() const { return heap.empty(); }
        size_t size() const { return heap.size(); }
    };

    // ==========================================
    // 5. TEMPLATE BINARY SEARCH TREE (BST)
    // ==========================================
    template <typename Key, typename Value>
    class BST
    {
    private:
        struct BSTNode
        {
            Key key;
            Value val;
            BSTNode *left;
            BSTNode *right;
            BSTNode(const Key &k, const Value &v) : key(k), val(v), left(nullptr), right(nullptr) {}
        };

        BSTNode *root;
        size_t count;

        void clearHelper(BSTNode *node)
        {
            if (!node) return;
            clearHelper(node->left);
            clearHelper(node->right);
            delete node;
        }

        BSTNode *insertHelper(BSTNode *node, const Key &k, const Value &v)
        {
            if (!node)
            {
                ++count;
                return new BSTNode(k, v);
            }
            if (k < node->key)
                node->left = insertHelper(node->left, k, v);
            else if (k > node->key)
                node->right = insertHelper(node->right, k, v);
            else
                node->val = v;
            return node;
        }

        BSTNode *findHelper(BSTNode *node, const Key &k) const
        {
            if (!node) return nullptr;
            if (k < node->key) return findHelper(node->left, k);
            if (k > node->key) return findHelper(node->right, k);
            return node;
        }

    public:
        BST() : root(nullptr), count(0) {}
        ~BST() { clear(); }

        void clear()
        {
            clearHelper(root);
            root = nullptr;
            count = 0;
        }

        void insert(const Key &k, const Value &v)
        {
            root = insertHelper(root, k, v);
        }

        Value *find(const Key &k)
        {
            BSTNode *res = findHelper(root, k);
            return res ? &(res->val) : nullptr;
        }

        const Value *find(const Key &k) const
        {
            BSTNode *res = findHelper(root, k);
            return res ? &(res->val) : nullptr;
        }

        size_t size() const { return count; }
        bool empty() const { return count == 0; }
    };

    // ==========================================
    // 6. GRAPH ALGORITHMS (Dijkstra, BFS, DFS)
    // ==========================================
    template <typename GraphType>
    Vector<size_t> dijkstraPath(const GraphType &graph, size_t numNodes, size_t start, size_t goal, double trafficMul, double weatherMul, double &distanceKm)
    {
        distanceKm = 0.0;
        Vector<double> dist(numNodes, std::numeric_limits<double>::infinity());
        Vector<int> parent(numNodes, -1);

        struct PQNode
        {
            double dist;
            size_t node;
            bool operator>(const PQNode &other) const { return dist > other.dist; }
        };

        PriorityQueue<PQNode, std::greater<PQNode>> pq;
        dist[start] = 0.0;
        pq.push({0.0, start});

        while (!pq.empty())
        {
            PQNode current = pq.top();
            pq.pop();
            if (current.dist > dist[current.node]) continue;
            if (current.node == goal) break;
            if (current.node >= graph.size()) continue;

            for (size_t i = 0; i < graph[current.node].size(); ++i)
            {
                auto edge = graph[current.node][i];
                size_t next = edge.first;
                double dynamicEdgeWeight = edge.second * trafficMul * weatherMul;
                double candidate = current.dist + dynamicEdgeWeight;
                if (candidate < dist[next])
                {
                    dist[next] = candidate;
                    parent[next] = static_cast<int>(current.node);
                    pq.push({candidate, next});
                }
            }
        }

        if (!isFinite(dist[goal])) return Vector<size_t>();
        Vector<size_t> path;
        for (int cur = static_cast<int>(goal); cur != -1; cur = parent[static_cast<size_t>(cur)])
            path.push_back(static_cast<size_t>(cur));

        // Reverse path
        for (size_t i = 0; i < path.size() / 2; ++i)
            std::swap(path[i], path[path.size() - 1 - i]);

        distanceKm = 0.0;
        for (size_t i = 1; i < path.size(); ++i)
            for (size_t j = 0; j < graph[path[i - 1]].size(); ++j)
                if (graph[path[i - 1]][j].first == path[i])
                {
                    distanceKm += graph[path[i - 1]][j].second;
                    break;
                }

        return path;
    }

    template <typename GraphType>
    Vector<size_t> bfsPath(const GraphType &graph, size_t numNodes, size_t start, size_t goal)
    {
        Vector<int> parent(numNodes, -1);
        Vector<bool> visited(numNodes, false);
        LocationQueue pending;
        visited[start] = true;
        pending.push(start);

        while (!pending.empty())
        {
            size_t node = pending.front();
            pending.pop();
            if (node == goal) break;
            if (node >= graph.size()) continue;

            for (size_t i = 0; i < graph[node].size(); ++i)
            {
                size_t next = graph[node][i].first;
                if (!visited[next])
                {
                    visited[next] = true;
                    parent[next] = static_cast<int>(node);
                    pending.push(next);
                }
            }
        }

        if (!visited[goal]) return Vector<size_t>();

        Vector<size_t> path;
        for (int cur = static_cast<int>(goal); cur != -1; cur = parent[static_cast<size_t>(cur)])
            path.push_back(static_cast<size_t>(cur));

        for (size_t i = 0; i < path.size() / 2; ++i)
            std::swap(path[i], path[path.size() - 1 - i]);

        return path;
    }

    template <typename GraphType>
    bool dfsReachableHelper(const GraphType &graph, size_t current, size_t goal, Vector<bool> &visited)
    {
        if (current == goal) return true;
        visited[current] = true;
        if (current >= graph.size()) return false;

        for (size_t i = 0; i < graph[current].size(); ++i)
        {
            size_t next = graph[current][i].first;
            if (!visited[next] && dfsReachableHelper(graph, next, goal, visited))
                return true;
        }
        return false;
    }

    template <typename GraphType>
    bool dfsReachable(const GraphType &graph, size_t numNodes, size_t start, size_t goal)
    {
        Vector<bool> visited(numNodes, false);
        return dfsReachableHelper(graph, start, goal, visited);
    }

    template <typename GraphType>
    bool dfsPathHelper(const GraphType &graph, size_t current, size_t goal, Vector<bool> &visited, Vector<size_t> &path)
    {
        visited[current] = true;
        path.push_back(current);
        if (current == goal) return true;
        for (const auto &edge : graph[current])
            if (!visited[edge.first] && dfsPathHelper(graph, edge.first, goal, visited, path)) return true;
        path.pop_back();
        return false;
    }

    template <typename GraphType>
    Vector<size_t> dfsPath(const GraphType &graph, size_t numNodes, size_t start, size_t goal)
    {
        Vector<bool> visited(numNodes, false);
        Vector<size_t> path;
        if (start < numNodes && goal < numNodes) dfsPathHelper(graph, start, goal, visited, path);
        return path;
    }

    template <typename Key, typename Value>
    class HashMap
    {
        struct Entry { Key key; Value value; Entry *next; Entry(const Key &k, const Value &v, Entry *n) : key(k), value(v), next(n) {} };
        Vector<Entry *> buckets;
        size_t count;
        size_t bucketIndex(const Key &key) const { return std::hash<Key>{}(key) % buckets.size(); }
    public:
        explicit HashMap(size_t bucketCount = 101) : buckets(bucketCount, nullptr), count(0) {}
        ~HashMap() { clear(); }
        void clear() { for (size_t i=0;i<buckets.size();++i){ Entry *p=buckets[i]; while(p){Entry *n=p->next; delete p; p=n;} buckets[i]=nullptr;} count=0; }
        void insert(const Key &key, const Value &value) { size_t i=bucketIndex(key); for(Entry *p=buckets[i];p;p=p->next) if(p->key==key){p->value=value;return;} buckets[i]=new Entry(key,value,buckets[i]); ++count; }
        Value &operator[](const Key &key) { Value *value=find(key); if(value) return *value; insert(key, Value()); return *find(key); }
        Value *find(const Key &key) { size_t i=bucketIndex(key); for(Entry *p=buckets[i];p;p=p->next) if(p->key==key) return &p->value; return nullptr; }
        const Value *find(const Key &key) const { size_t i=bucketIndex(key); for(Entry *p=buckets[i];p;p=p->next) if(p->key==key) return &p->value; return nullptr; }
        bool contains(const Key &key) const { return find(key)!=nullptr; }
        size_t size() const { return count; }
    };
}

#endif // DSA_HPP
