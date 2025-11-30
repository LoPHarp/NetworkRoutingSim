#ifndef PACKET_H
#define PACKET_H

#include <QObject>
#include <QGraphicsItem>
#include <QPainter>
#include <QPixmap>

enum PacketType
{
    DATA,
    CONN_REQ,
    CONN_ACK,
    DISCONNECT
};

class Packet : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    Packet(int sequenceNumber, int dataSize, PacketType type = DATA);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    int getSequenceNumber() const { return seqNum; }
    int getDataSize() const { return size; }

    void setOpacity(qreal opacity)
    {
        QGraphicsItem::setOpacity(opacity);
    }

    qreal opacity() const
    {
        return QGraphicsItem::opacity();
    }

private:
    int seqNum;
    int size;
    PacketType type;
    QPixmap sprite;
};

#endif // PACKET_H
