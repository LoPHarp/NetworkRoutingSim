#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <vector>
#include <QTimer>
#include "packet.h"
#include "chartwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTimer *dataTimer;

    std::vector<int> currentPath;
    int currentMsgSize;
    int currentPacketSize;
    int currentErrorRate;

    int packetsSentCount;
    int packetsDeliveredCount;
    int totalPacketsToSend;
    bool isVirtualMode;

    double calculatedTime;
    int calculatedServiceTraffic;

    void startSimulation();
    void startDataTransmission();
    void sendNextDataPacket();

    void stepHandshakeReq();
    void stepHandshakeAck();
    void stepDisconnect();

    void sendSinglePacket(int id, int size, PacketType type, std::vector<int> path, bool isRetransmission = false);

    void onPacketDelivered(int id, int size, PacketType type);
    void checkCompletion();

    void logToTable(bool success);
    void setupTable();

    void showChartServiceTraffic();
    void showChartPacketsCount();
    void showChartErrorDependence();

    QAbstractAnimation* createPacketAnim(Packet* pkt, std::vector<int> path, int errorRate);
};
#endif // MAINWINDOW_H
