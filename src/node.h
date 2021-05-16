#ifndef NODE_H
#define NODE_H

#include <list>

class Node {
public:
    Node();

    Node(int x, int y);

    // Node(Node &source) = delete;

    // Node(Node &&source) = delete;

    // Node &operator=(Node &source) = delete;

    // Node &operator=(Node &&source) = delete;

    ~Node() = default;

private:
    int x, y, dist{255};
    std::list<Node> neighbours{};
};

#endif
