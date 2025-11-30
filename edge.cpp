#include "edge.h"
#include "node.h"
#include <QPen>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QInputDialog>
#include <QVector>

Edge::Edge(Node *sourceNode, Node *destNode, int weight, EdgeType type)
    : source(sourceNode), dest(destNode), weight(weight), type(type)
{
    setZValue(-1);
    setFlag(ItemIsSelectable);
    adjust();
}

Edge::~Edge()
{
    if (source) source->removeEdge(this);
    if (dest) dest->removeEdge(this);
}

Node* Edge::sourceNode() const
{
    return source;
}

Node* Edge::destNode() const
{
    return dest;
}

int Edge::getWeight() const
{
    return weight;
}

EdgeType Edge::getType() const
{
    return type;
}

void Edge::adjust()
{
    if (!source || !dest) return;

    QLineF line(source->mapToScene(0, 0), dest->mapToScene(0, 0));

    setLine(line);
}

QRectF Edge::boundingRect() const
{
    QRectF rect = QGraphicsLineItem::boundingRect();
    QPointF center = (line().p1() + line().p2()) / 2;
    QRectF textRect(center.x() - 10, center.y() - 10, 20, 20);
    return rect.united(textRect);
}

QPainterPath Edge::shape() const
{
    QPainterPath path;
    path.moveTo(line().p1());
    path.lineTo(line().p2());
    QPainterPathStroker stroker;
    stroker.setWidth(10);
    return stroker.createStroke(path);
}

void Edge::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (!source || !dest) return;

    QColor lineColor = isSelected() ? Qt::red : Qt::black;
    int lineThickness = isSelected() ? 4 : 2;

    QPen pen(lineColor, lineThickness);

    if (type == HalfDuplex)
    {
        QVector<qreal> dashes;
        dashes << 8 << 7;
        pen.setDashPattern(dashes);
    }
    else
    {
        pen.setStyle(Qt::SolidLine);
    }

    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    painter->setPen(pen);
    painter->drawLine(line());

    QPointF center = (line().p1() + line().p2()) / 2;
    QRectF textRect(center.x() - 10, center.y() - 10, 20, 20);

    painter->setBrush(Qt::white);
    painter->setPen(Qt::NoPen);
    painter->drawRect(textRect);

    painter->setPen(Qt::black);
    painter->setFont(QFont("Arial", 10, QFont::Bold));
    painter->drawText(textRect, Qt::AlignCenter, QString::number(weight));
}

void Edge::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    bool ok;
    int newWeight = QInputDialog::getInt(nullptr, "Зміна ваги",
                                         "Введіть нову вагу каналу:",
                                         weight, 1, 100, 1, &ok);

    if (ok)
    {
        weight = newWeight;
        update();
    }

    QGraphicsLineItem::mouseDoubleClickEvent(event);
}
