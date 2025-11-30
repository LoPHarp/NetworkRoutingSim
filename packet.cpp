#include "packet.h"

Packet::Packet(int sequenceNumber, int dataSize, PacketType type)
    : seqNum(sequenceNumber), size(dataSize), type(type)
{
    switch (type)
    {
    case DATA:
        sprite.load(":/image/envelope_yellow.png");
        break;
    case CONN_REQ:
        sprite.load(":/image/envelope_blue.png");
        break;
    case CONN_ACK:
        sprite.load(":/image/envelope_green.png");
        break;
    case DISCONNECT:
        sprite.load(":/image/envelope_red.png");
        break;
    default:
        sprite.load(":/image/envelope_yellow.png");
        break;
    }

    setZValue(10);
}

QRectF Packet::boundingRect() const
{
    return QRectF(-25, -20, 50, 50);
}

void Packet::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->drawPixmap(-25, -20, 50, 50, sprite);

    painter->setPen(Qt::black);
    painter->setFont(QFont("Arial", 12, QFont::Bold));
    painter->drawText(boundingRect(), Qt::AlignCenter, QString::number(seqNum));
}
