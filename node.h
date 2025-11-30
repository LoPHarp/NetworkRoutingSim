#ifndef NODE_H
#define NODE_H

#include <QGraphicsItem>
#include <QPainter>
#include <QPixmap>

class Edge;

class Node : public QGraphicsItem
{
public:
    Node(int id);

    void addEdge(Edge *edge);
    void removeEdge(Edge *edge);

    QList<Edge *> edges() const;

    QRectF boundingRect() const override;  //хітбокс
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    int getId() const { return id; }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    int id;
    QPixmap sprite;

    QList<Edge *> edgeList;
};

#endif // NODE_H
