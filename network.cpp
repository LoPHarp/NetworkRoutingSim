#include "network.h"
#include "node.h"
#include "edge.h"

#include <cstdlib>
#include <vector>
#include <ctime>

int getRandomWeight()
{
    int weights[] = {3, 5, 6, 7, 8, 10, 11, 15, 18, 21};
    int index = rand() % 10;
    return weights[index];
}

void connectNodes(Node* n1, Node* n2, QGraphicsScene* scene)
{
    if (!n1 || !n2) return;

    int weight = getRandomWeight();

    EdgeType type = (rand() % 100 < 30) ? HalfDuplex : Duplex;

    if (type == HalfDuplex)
    {
        weight = weight * 1.5;
    }

    Edge *edge = new Edge(n1, n2, weight, type);
    scene->addItem(edge);

    n1->addEdge(edge);
    n2->addEdge(edge);
}

void Network::generate(QGraphicsScene *scene)
{
    scene->clear();

    int regions = 3;
    int nodesPerRegion = 9;
    int currentId = 1;

    std::vector<Node*> regionGateways;

    for (int r = 0; r < regions; ++r)
    {
        int centerX = 0;
        int centerY = 0;

        if (r == 0)
        {
            centerX = -400;
            centerY = -200;
        }
        else if (r == 1)
        {
            centerX = 0;
            centerY = 250;
        }
        else if (r == 2)
        {
            centerX = 400;
            centerY = -200;
        }

        std::vector<Node*> currentRegionNodes;

        for (int i = 0; i < nodesPerRegion; ++i)
        {
            Node *node = new Node(currentId++);

            int x = centerX + (rand() % 400 - 200);
            int y = centerY + (rand() % 400 - 200);

            node->setPos(x, y);
            scene->addItem(node);

            currentRegionNodes.push_back(node);
        }

        for (int i = 0; i < nodesPerRegion; ++i)
        {
            Node* n1 = currentRegionNodes[i];
            Node* n2 = currentRegionNodes[(i + 1) % nodesPerRegion];
            connectNodes(n1, n2, scene);
        }

        for (int k = 0; k < 4; ++k)
        {
            int idx1 = rand() % nodesPerRegion;
            int idx2 = rand() % nodesPerRegion;

            if (idx1 != idx2)
            {
                connectNodes(currentRegionNodes[idx1], currentRegionNodes[idx2], scene);
            }
        }

        regionGateways.push_back(currentRegionNodes[0]);
    }

    if (regionGateways.size() >= 3)
    {
        connectNodes(regionGateways[0], regionGateways[1], scene);
        connectNodes(regionGateways[1], regionGateways[2], scene);
        connectNodes(regionGateways[2], regionGateways[0], scene);
    }
}
