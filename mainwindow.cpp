#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "node.h"
#include "edge.h"
#include "network.h"
#include "packet.h"
#include "dijkstra.h"

#include <QGraphicsScene>
#include <QSet>
#include <QDateTime>
#include <QMessageBox>
#include <algorithm>
#include <cstdlib>
#include <QTimer>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupTable();

    QGraphicsScene *scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    scene->setSceneRect(-500, -500, 1000, 1000);

    Node *router1 = new Node(1);
    router1->setPos(-100, 0);
    scene->addItem(router1);

    Node *router2 = new Node(2);
    router2->setPos(100, 0);
    scene->addItem(router2);

    Edge *edge = new Edge(router1, router2, 10, Duplex);
    scene->addItem(edge);
    router1->addEdge(edge);
    router2->addEdge(edge);

    dataTimer = new QTimer(this);
    connect(dataTimer, &QTimer::timeout, this, &MainWindow::sendNextDataPacket);

    ui->graphicsView->setDragMode(QGraphicsView::RubberBandDrag);

    connect(ui->rbAlgoHops, &QRadioButton::toggled, this, [=](bool checked)
            {
                Dijkstra::useMinHops = checked;
            });
    Dijkstra::useMinHops = ui->rbAlgoHops->isChecked();

    connect(ui->btnGenerate, &QPushButton::clicked, this, [=]()
            {
                Network::generate(ui->graphicsView->scene());
            });

    connect(ui->btnAddNode, &QPushButton::clicked, this, [=]()
            {
                int maxId = 0;
                foreach(QGraphicsItem *item, scene->items())
                {
                    Node *node = dynamic_cast<Node*>(item);
                    if (node)
                    {
                        if (node->getId() > maxId) maxId = node->getId();
                    }
                }
                Node *newNode = new Node(maxId + 1);
                newNode->setPos(0, 0);
                scene->addItem(newNode);
            });

    connect(ui->btnAddEdge, &QPushButton::clicked, this, [=]()
            {
                QList<QGraphicsItem *> selectedItems = scene->selectedItems();
                if (selectedItems.size() != 2) return;

                Node *nodeA = dynamic_cast<Node*>(selectedItems[0]);
                Node *nodeB = dynamic_cast<Node*>(selectedItems[1]);

                if (nodeA && nodeB)
                {
                    Edge *edge = new Edge(nodeA, nodeB, 10, Duplex);
                    scene->addItem(edge);
                    nodeA->addEdge(edge);
                    nodeB->addEdge(edge);
                }
            });

    connect(ui->btnAddHalfEdge, &QPushButton::clicked, this, [=]()
            {
                QList<QGraphicsItem *> selectedItems = scene->selectedItems();
                if (selectedItems.size() != 2) return;

                Node *nodeA = dynamic_cast<Node*>(selectedItems[0]);
                Node *nodeB = dynamic_cast<Node*>(selectedItems[1]);

                if (nodeA && nodeB)
                {
                    Edge *edge = new Edge(nodeA, nodeB, 15, HalfDuplex);
                    scene->addItem(edge);
                    nodeA->addEdge(edge);
                    nodeB->addEdge(edge);
                }
            });

    connect(ui->btnDelete, &QPushButton::clicked, this, [=]()
            {
                QSet<QGraphicsItem *> itemsToDelete;
                foreach (QGraphicsItem *item, scene->selectedItems())
                {
                    itemsToDelete.insert(item);
                    Node *node = dynamic_cast<Node*>(item);
                    if (node)
                    {
                        foreach(Edge *edge, node->edges())
                        itemsToDelete.insert(edge);
                    }
                }
                foreach (QGraphicsItem *item, itemsToDelete)
                {
                    scene->removeItem(item);
                    delete item;
                }
            });

    connect(ui->btnStartSimulation, &QPushButton::clicked, this, &MainWindow::startSimulation);

    connect(ui->btnChartService, &QPushButton::clicked, this, &MainWindow::showChartServiceTraffic);
    connect(ui->btnChartPackets, &QPushButton::clicked, this, &MainWindow::showChartPacketsCount);
    connect(ui->btnChartError, &QPushButton::clicked, this, &MainWindow::showChartErrorDependence);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupTable()
{
    QStringList headers;
    headers << "From" << "To" << "Type" << "Time (ms)" << "Service (B)" << "Packets" << "Msg Size" << "Path" << "Delivered";
    ui->tableResults->setColumnCount(headers.size());
    ui->tableResults->setHorizontalHeaderLabels(headers);
    ui->tableResults->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableResults->horizontalHeader()->setStretchLastSection(true);
}

void MainWindow::startSimulation()
{
    ui->textLog->clear();
    ui->textLog->append(QDateTime::currentDateTime().toString("hh:mm:ss") + " [INFO] Старт симуляції...");

    int sourceID = ui->spinSourceID->value();
    int destID = ui->spinTargetID->value();
    currentMsgSize = ui->spinMsgSize->value();
    currentPacketSize = ui->spinPacketSize->value();
    currentErrorRate = ui->spinErrorProb->value();
    isVirtualMode = ui->rbVirtual->isChecked();

    Node *startNode = nullptr;
    QList<Node*> allNodes;
    foreach (QGraphicsItem *item, ui->graphicsView->scene()->items())
    {
        Node *n = dynamic_cast<Node*>(item);
        if (n)
        {
            allNodes.append(n);
            if (n->getId() == sourceID) startNode = n;
        }
    }

    if (!startNode)
    {
        ui->textLog->append("[ERROR] Стартовий вузол не знайдено!");
        return;
    }

    std::vector<RoutingEntry> table;
    if (Dijkstra::useMinHops)
        table = Dijkstra::calculateMinHops(startNode, allNodes);
    else
        table = Dijkstra::calculate(startNode, allNodes);

    currentPath.clear();
    int pathCost = 0;

    for (const auto& entry : table)
    {
        if (entry.destinationID == destID)
        {
            currentPath = entry.fullPath;
            pathCost = entry.totalCost;
            break;
        }
    }

    if (currentPath.size() < 2)
    {
        ui->textLog->append("[ERROR] Шлях не знайдено!");
        return;
    }

    int headerSize = 40;
    if (currentPacketSize <= headerSize)
    {
        QMessageBox::warning(this, "Помилка", "MTU замалий!");
        return;
    }

    int maxPayload = currentPacketSize - headerSize;
    totalPacketsToSend = (currentMsgSize + maxPayload - 1) / maxPayload;

    calculatedServiceTraffic = totalPacketsToSend * headerSize;
    if (isVirtualMode)
        calculatedServiceTraffic += (3 * headerSize);

    int totalTraffic = currentMsgSize + calculatedServiceTraffic;

    int latencyPerHop = 10;
    int totalHops = currentPath.size() - 1;
    double baseTime = (double)totalTraffic / 10.0;
    calculatedTime = baseTime + (totalHops * latencyPerHop);

    if (isVirtualMode) calculatedTime *= 1.2;

    ui->textLog->append("--------------------------------------------------");
    ui->textLog->append("МАРШРУТ: Вартість = " + QString::number(pathCost));

    QString pathStr = "Шлях: ";
    for (size_t i = 0; i < currentPath.size(); ++i)
    {
        pathStr += QString::number(currentPath[i]);
        if (i < currentPath.size() - 1) pathStr += " -> ";
    }
    ui->textLog->append(pathStr);

    ui->textLog->append("--------------------------------------------------");
    ui->textLog->append("ПАРАМЕТРИ ПЕРЕДАЧІ:");
    ui->textLog->append("  MTU (Розмір пакету): " + QString::number(currentPacketSize) + " байт");
    ui->textLog->append("  Корисна ємність (Payload): " + QString::number(maxPayload) + " байт");
    ui->textLog->append("--------------------------------------------------");
    ui->textLog->append("ПРОГНОЗ ТРАФІКУ:");
    ui->textLog->append("  Всього пакетів даних: " + QString::number(totalPacketsToSend));
    ui->textLog->append("  Службовий трафік: " + QString::number(calculatedServiceTraffic) + " байт");
    ui->textLog->append("  Розрахунковий час: " + QString::number(calculatedTime) + " мс");
    ui->textLog->append("--------------------------------------------------");

    packetsSentCount = 0;
    packetsDeliveredCount = 0;

    if (isVirtualMode)
    {
        ui->textLog->append("=== [Фаза 1] Встановлення з'єднання (Handshake) ===");
        stepHandshakeReq();
    }
    else
    {
        ui->textLog->append("=== [Фаза 1] Дейтаграмний режим ===");
        startDataTransmission();
    }
}

void MainWindow::stepHandshakeReq()
{
    sendSinglePacket(0, 0, CONN_REQ, currentPath);
}

void MainWindow::stepHandshakeAck()
{
    std::vector<int> pathBack = currentPath;
    std::reverse(pathBack.begin(), pathBack.end());
    sendSinglePacket(0, 0, CONN_ACK, pathBack);
}

void MainWindow::startDataTransmission()
{
    if (isVirtualMode)
        ui->textLog->append("=== [Фаза 2] Передача даних (Потік) ===");

    dataTimer->start(1000);
    sendNextDataPacket();
}

void MainWindow::sendNextDataPacket()
{
    if (packetsSentCount < totalPacketsToSend)
    {
        int headerSize = 40;
        int maxPayload = currentPacketSize - headerSize;
        int currentPayload = (packetsSentCount == totalPacketsToSend - 1) ? (currentMsgSize - packetsSentCount * maxPayload) : maxPayload;

        sendSinglePacket(packetsSentCount + 1, currentPayload, DATA, currentPath);
        packetsSentCount++;
    }
    else
    {
        dataTimer->stop();
    }
}

void MainWindow::checkCompletion()
{
    if (packetsDeliveredCount >= totalPacketsToSend)
    {
        if (isVirtualMode)
        {
            ui->textLog->append("=== [Фаза 3] Розрив з'єднання ===");
            stepDisconnect();
        }
        else
        {
            ui->textLog->append("--------------------------------------------------");
            ui->textLog->append("[FINISH] Передачу завершено.");
            logToTable(true);
        }
    }
}

void MainWindow::stepDisconnect()
{
    sendSinglePacket(0, 0, DISCONNECT, currentPath);
}

void MainWindow::logToTable(bool success)
{
    int row = ui->tableResults->rowCount();
    ui->tableResults->insertRow(row);

    QString pathStr;
    for (size_t i = 0; i < currentPath.size(); ++i)
    {
        pathStr += QString::number(currentPath[i]);
        if (i < currentPath.size() - 1) pathStr += "->";
    }

    ui->tableResults->setItem(row, 0, new QTableWidgetItem(QString::number(ui->spinSourceID->value())));
    ui->tableResults->setItem(row, 1, new QTableWidgetItem(QString::number(ui->spinTargetID->value())));
    ui->tableResults->setItem(row, 2, new QTableWidgetItem(isVirtualMode ? "Virtual" : "Datagram"));
    ui->tableResults->setItem(row, 3, new QTableWidgetItem(QString::number(calculatedTime, 'f', 2)));
    ui->tableResults->setItem(row, 4, new QTableWidgetItem(QString::number(calculatedServiceTraffic)));
    ui->tableResults->setItem(row, 5, new QTableWidgetItem(QString::number(totalPacketsToSend)));
    ui->tableResults->setItem(row, 6, new QTableWidgetItem(QString::number(currentMsgSize)));
    ui->tableResults->setItem(row, 7, new QTableWidgetItem(pathStr));

    QTableWidgetItem *statusItem = new QTableWidgetItem(success ? "Yes" : "No");
    if (!success) statusItem->setBackground(Qt::red);
    ui->tableResults->setItem(row, 8, statusItem);
}

void MainWindow::sendSinglePacket(int id, int size, PacketType type, std::vector<int> path, bool isRetransmission)
{
    if (isRetransmission)
    {
        ui->textLog->append("!! [RETRY] Повторна відправка пакету #" + QString::number(id));
    }

    Node *startNode = nullptr;
    foreach(QGraphicsItem *item, ui->graphicsView->scene()->items())
    {
        Node *n = dynamic_cast<Node*>(item);
        if (n && n->getId() == path[0]) startNode = n;
    }
    if (!startNode) return;

    Packet *pkt = new Packet(id, size, type);
    ui->graphicsView->scene()->addItem(pkt);
    pkt->setPos(startNode->pos());
    pkt->setVisible(true);
    pkt->setProperty("isLost", false);

    QAbstractAnimation *anim = createPacketAnim(pkt, path, currentErrorRate);

    connect(anim, &QAbstractAnimation::finished, this, [=]()
            {
                bool lost = pkt->property("isLost").toBool();
                int lostNode = pkt->property("lostNode").toInt();

                ui->graphicsView->scene()->removeItem(pkt);
                delete pkt;

                anim->deleteLater();

                if (lost)
                {
                    ui->textLog->append("xx [LOSS] Пакет #" + QString::number(id) + " втрачено на шляху до вузла " + QString::number(lostNode));

                    if (isVirtualMode)
                    {
                        QTimer::singleShot(1500, this, [=]() {
                            sendSinglePacket(id, size, type, path, true);
                        });
                    }
                }
                else
                {
                    onPacketDelivered(id, size, type);
                }
            });

    anim->start();
}

void MainWindow::onPacketDelivered(int id, int size, PacketType type)
{
    QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss");

    if (isVirtualMode)
    {
        if (type == CONN_REQ)
        {
            ui->textLog->append(timeStr + " >> [REQ] Запит доставлено.");
            QTimer::singleShot(500, this, &MainWindow::stepHandshakeAck);
        }
        else if (type == CONN_ACK)
        {
            ui->textLog->append(timeStr + " >> [ACK] З'єднання встановлено!");
            QTimer::singleShot(500, this, &MainWindow::startDataTransmission);
        }
        else if (type == DATA)
        {
            ui->textLog->append(timeStr + " >> [DATA] Пакет #" + QString::number(id) + " доставлено.");

            std::vector<int> pathBack = currentPath;
            std::reverse(pathBack.begin(), pathBack.end());

            ui->textLog->append("<< [ACK] Підтвердження для пакету #" + QString::number(id) + " відправлено.");

            packetsDeliveredCount++;
            checkCompletion();
        }
        else if (type == DISCONNECT)
        {
            ui->textLog->append(timeStr + " >> [FIN] З'єднання розірвано.");
            ui->textLog->append("--------------------------------------------------");
            ui->textLog->append("[FINISH] Симуляцію завершено успішно.");
            logToTable(true);
        }
    }
    else
    {
        if (type == DATA)
        {
            ui->textLog->append(timeStr + " >> [DATA] Пакет #" + QString::number(id) + " доставлено.");
            packetsDeliveredCount++;
            checkCompletion();
        }
    }
}

QAbstractAnimation* MainWindow::createPacketAnim(Packet* pkt, std::vector<int> path, int errorRate)
{
    QSequentialAnimationGroup *seq = new QSequentialAnimationGroup;

    for (size_t k = 0; k < path.size() - 1; ++k)
    {
        int id1 = path[k];
        int id2 = path[k+1];
        QPointF p1, p2;

        QList<QGraphicsItem*> items = ui->graphicsView->scene()->items();
        for(auto *item : items)
        {
            Node *n = dynamic_cast<Node*>(item);
            if (n)
            {
                if (n->getId() == id1) p1 = n->pos();
                if (n->getId() == id2) p2 = n->pos();
            }
        }

        QPropertyAnimation *moveAnim = new QPropertyAnimation(pkt, "pos");
        moveAnim->setDuration(1000);
        moveAnim->setStartValue(p1);
        moveAnim->setEndValue(p2);
        moveAnim->setEasingCurve(QEasingCurve::InOutQuad);

        int randomVal = rand() % 100;
        if (randomVal < errorRate)
        {
            QParallelAnimationGroup *lossGroup = new QParallelAnimationGroup;
            lossGroup->addAnimation(moveAnim);

            QPropertyAnimation *fadeAnim = new QPropertyAnimation(pkt, "opacity");
            fadeAnim->setDuration(1000);
            fadeAnim->setStartValue(1.0);
            fadeAnim->setEndValue(0.0);
            lossGroup->addAnimation(fadeAnim);

            seq->addAnimation(lossGroup);

            pkt->setProperty("isLost", true);
            pkt->setProperty("lostNode", id2);
            break;
        }
        else
        {
            seq->addAnimation(moveAnim);
        }
    }
    return seq;
}

void MainWindow::showChartServiceTraffic()
{
    int msgSize = ui->spinMsgSize->value();
    bool isVirtual = ui->rbVirtual->isChecked();
    int headerSize = 40;

    std::vector<std::pair<double, double>> data;

    for (int mtu = 50; mtu <= 1500; mtu += 10)
    {
        int maxPayload = mtu - headerSize;
        if (maxPayload <= 0) continue;

        int packets = (msgSize + maxPayload - 1) / maxPayload;

        int serviceTraffic = packets * headerSize;
        if (isVirtual) serviceTraffic += (3 * headerSize);

        data.push_back({(double)mtu, (double)serviceTraffic});
    }

    ChartWindow *w = new ChartWindow("Залежність службового трафіку від MTU", "Розмір пакету (MTU), байт", "Службовий трафік, байт", this);
    w->setData(data);
    w->show();
}

void MainWindow::showChartPacketsCount()
{
    int msgSize = ui->spinMsgSize->value();
    int headerSize = 40;

    std::vector<std::pair<double, double>> data;

    for (int mtu = 50; mtu <= 1500; mtu += 10)
    {
        int maxPayload = mtu - headerSize;
        if (maxPayload <= 0) continue;

        int packets = (msgSize + maxPayload - 1) / maxPayload;

        data.push_back({(double)mtu, (double)packets});
    }

    ChartWindow *w = new ChartWindow("Залежність кількості пакетів від MTU", "Розмір пакету (MTU), байт", "Кількість пакетів, шт", this);
    w->setData(data);
    w->show();
}

void MainWindow::showChartErrorDependence()
{
    int msgSize = ui->spinMsgSize->value();
    int mtu = ui->spinPacketSize->value();
    bool isVirtual = ui->rbVirtual->isChecked();
    int headerSize = 40;

    if (mtu <= headerSize) return;

    int maxPayload = mtu - headerSize;
    int packets = (msgSize + maxPayload - 1) / maxPayload;

    int baseTraffic = msgSize + (packets * headerSize);
    if (isVirtual) baseTraffic += (3 * headerSize);

    std::vector<std::pair<double, double>> data;

    for (int error = 0; error <= 80; error += 2)
    {
        double prob = (double)error / 100.0;

        double totalTraffic = (double)baseTraffic / (1.0 - prob);

        data.push_back({(double)error, totalTraffic});
    }

    ChartWindow *w = new ChartWindow("Залежність трафіку від ймовірності помилок", "Ймовірність помилки, %", "Загальний трафік (прогноз), байт", this);
    w->setData(data);
    w->show();
}
