#include "node.h"
#include "edge.h"
#include "dijkstra.h"

#include <QGraphicsScene>
#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>

Node::Node(int id) : id(id)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setFlag(ItemIsSelectable);
    sprite.load(":/image/router.png");
}

void Node::addEdge(Edge *edge)
{
    edgeList << edge;
    edge->adjust();
}

void Node::removeEdge(Edge *edge)
{
    edgeList.removeAll(edge);
}

QList<Edge *> Node::edges() const
{
    return edgeList;
}

QRectF Node::boundingRect() const
{
    return QRectF(-35, -45, 70, 70);
}

void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->drawPixmap(-30, -20, 60, 40, sprite);

    QRectF textRect(-15, -42, 30, 20);

    painter->setBrush(QColor(0, 100, 255));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(textRect, 5, 5);

    painter->setPen(Qt::white);
    painter->setFont(QFont("Arial", 9, QFont::Bold));
    painter->drawText(textRect, Qt::AlignCenter, QString::number(id));

    if (isSelected())
    {
        painter->setPen(QPen(Qt::yellow, 2, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}

QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange)
        foreach (Edge *edge, edgeList)
            edge->adjust();
    return QGraphicsItem::itemChange(change, value);
}

void Node::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QList<Node*> allNodes;
    foreach (QGraphicsItem *item, scene()->items())
    {
        Node *n = dynamic_cast<Node*>(item);
        if (n) allNodes.append(n);
    }

    std::vector<RoutingEntry> tableData;

    if (Dijkstra::useMinHops)
    {
        tableData = Dijkstra::calculateMinHops(this, allNodes);
    }
    else
    {
        tableData = Dijkstra::calculate(this, allNodes);
    }

    QDialog *tableWindow = new QDialog();
    QString algoName = Dijkstra::useMinHops ? " (Min Hops)" : " (Weight)";
    tableWindow->setWindowTitle("Таблиця маршрутизації роутера #" + QString::number(id) + algoName);
    tableWindow->resize(700, 450);

    QTableWidget *table = new QTableWidget();

    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"Direction", "Next Hop", "Full Path", "Metric"});

    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);

    table->setRowCount(tableData.size());

    std::sort(tableData.begin(), tableData.end(), [](const RoutingEntry& a, const RoutingEntry& b) {
        return a.destinationID < b.destinationID;
    });

    for (int i = 0; i < tableData.size(); ++i)
    {
        RoutingEntry entry = tableData[i];

        QString dirStr = QString::number(id) + " -> " + QString::number(entry.destinationID);

        QString nextHopStr = (entry.fullPath.size() > 1) ? QString::number(entry.fullPath[1]) : "-";

        QString pathStr = "";
        for (int k = 0; k < entry.fullPath.size(); ++k)
        {
            pathStr += QString::number(entry.fullPath[k]);
            if (k < entry.fullPath.size() - 1) pathStr += " -> ";
        }

        QString costStr = QString::number(entry.totalCost);

        QTableWidgetItem *itemDir = new QTableWidgetItem(dirStr);
        QTableWidgetItem *itemNext = new QTableWidgetItem(nextHopStr);
        QTableWidgetItem *itemPath = new QTableWidgetItem(pathStr);
        QTableWidgetItem *itemCost = new QTableWidgetItem(costStr);

        itemDir->setTextAlignment(Qt::AlignCenter);
        itemNext->setTextAlignment(Qt::AlignCenter);
        itemPath->setTextAlignment(Qt::AlignCenter);
        itemCost->setTextAlignment(Qt::AlignCenter);

        table->setItem(i, 0, itemDir);
        table->setItem(i, 1, itemNext);
        table->setItem(i, 2, itemPath);
        table->setItem(i, 3, itemCost);
    }

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(table);
    tableWindow->setLayout(layout);

    tableWindow->show();

    QGraphicsItem::mouseDoubleClickEvent(event);
}
