#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <vector>
#include <QList>

class Node;

struct RoutingEntry
{
    int destinationID;
    std::vector<int> fullPath;
    int totalCost;
};

class Dijkstra
{
public:
    static bool useMinHops;

    static std::vector<RoutingEntry> calculate(Node* startNode, const QList<Node*>& allNodes);
    static std::vector<RoutingEntry> calculateMinHops(Node* startNode, const QList<Node*>& allNodes);
};

#endif // DIJKSTRA_H
