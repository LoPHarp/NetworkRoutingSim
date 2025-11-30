#ifndef EDGE_H
#define EDGE_H

#include <QGraphicsLineItem>
#include <QPainter>

class Node;

enum EdgeType
{
    Duplex,
    HalfDuplex
};

class Edge : public QGraphicsLineItem
{
public:
    Edge(Node *sourceNode, Node *destNode, int weight, EdgeType type = Duplex);
    ~Edge();

    void adjust();

    Node* sourceNode() const;
    Node* destNode() const;
    int getWeight() const;
    EdgeType getType() const;

    QRectF boundingRect() const override;
    QPainterPath shape() const override;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

public:
    Node *source, *dest;
    int weight;
    EdgeType type;
};

#endif // EDGE_H
