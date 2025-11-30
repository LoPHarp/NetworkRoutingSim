#include "dijkstra.h"
#include "node.h"
#include "edge.h"
#include <map>
#include <queue>
#include <limits>
#include <algorithm>

using namespace std;

bool Dijkstra::useMinHops = false;

const int INF = std::numeric_limits<int>::max();

vector<RoutingEntry> Dijkstra::calculate(Node* startNode, const QList<Node*>& allNodes)
{
    vector<RoutingEntry> table;

    if (!startNode || allNodes.isEmpty()) return table;

    map<int, int> dist;
    map<int, Node*> parent;
    map<int, bool> visited;

    for (Node* node : allNodes)
    {
        int id = node->getId();
        dist[id] = INF;
        visited[id] = false;
        parent[id] = nullptr;
    }

    dist[startNode->getId()] = 0;

    for (int i = 0; i < allNodes.size(); ++i)
    {
        Node* current = nullptr;
        int minVal = INF;

        for (Node* node : allNodes)
        {
            int id = node->getId();
            if (!visited[id] && dist[id] < minVal)
            {
                minVal = dist[id];
                current = node;
            }
        }

        if (current == nullptr || dist[current->getId()] == INF) break;

        visited[current->getId()] = true;

        for (Edge* edge : current->edges())
        {
            Node* neighbor = (edge->sourceNode() == current) ? edge->destNode() : edge->sourceNode();

            if (!visited[neighbor->getId()])
            {
                int weight = edge->getWeight();

                if (dist[current->getId()] + weight < dist[neighbor->getId()])
                {
                    dist[neighbor->getId()] = dist[current->getId()] + weight;
                    parent[neighbor->getId()] = current;
                }
            }
        }
    }

    for (Node* target : allNodes)
    {
        if (target == startNode) continue;
        if (dist[target->getId()] == INF) continue;

        vector<int> path;
        Node* curr = target;

        while (curr != nullptr)
        {
            path.insert(path.begin(), curr->getId());
            if (curr == startNode) break;
            curr = parent[curr->getId()];
        }

        RoutingEntry entry;
        entry.destinationID = target->getId();
        entry.fullPath = path;
        entry.totalCost = dist[target->getId()];

        table.push_back(entry);
    }

    return table;
}

vector<RoutingEntry> Dijkstra::calculateMinHops(Node* startNode, const QList<Node*>& allNodes)
{
    vector<RoutingEntry> table;
    if (!startNode || allNodes.isEmpty()) return table;

    map<int, int> dist;
    map<int, Node*> parent;
    map<int, bool> visited;

    for (Node* node : allNodes)
    {
        int id = node->getId();
        dist[id] = INF;
        visited[id] = false;
        parent[id] = nullptr;
    }

    queue<Node*> q;

    dist[startNode->getId()] = 0;
    visited[startNode->getId()] = true;
    q.push(startNode);

    while (!q.empty())
    {
        Node* current = q.front();
        q.pop();

        for (Edge* edge : current->edges())
        {
            Node* neighbor = (edge->sourceNode() == current) ? edge->destNode() : edge->sourceNode();

            if (!visited[neighbor->getId()])
            {
                visited[neighbor->getId()] = true;
                dist[neighbor->getId()] = dist[current->getId()] + 1;
                parent[neighbor->getId()] = current;
                q.push(neighbor);
            }
        }
    }

    for (Node* target : allNodes)
    {
        if (target == startNode) continue;
        if (dist[target->getId()] == INF) continue;

        vector<int> path;
        Node* curr = target;
        int realCost = 0;

        while (curr != nullptr)
        {
            path.insert(path.begin(), curr->getId());

            Node* par = parent[curr->getId()];
            if (par != nullptr)
            {
                for (Edge* e : par->edges())
                {
                    if ((e->sourceNode() == par && e->destNode() == curr) || (e->sourceNode() == curr && e->destNode() == par))
                    {
                        realCost += e->getWeight();
                        break;
                    }
                }
            }

            if (curr == startNode) break;
            curr = par;
        }

        RoutingEntry entry;
        entry.destinationID = target->getId();
        entry.fullPath = path;
        entry.totalCost = realCost;

        table.push_back(entry);
    }

    return table;
}
