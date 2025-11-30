#ifndef CHARTWINDOW_H
#define CHARTWINDOW_H

#include <QDialog>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QVBoxLayout>

class ChartWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ChartWindow(QString title, QString xLabel, QString yLabel, QWidget *parent = nullptr);
    ~ChartWindow();

    void setData(const std::vector<std::pair<double, double>>& data);

private:
    QChart *chart;
    QLineSeries *series;
    QChartView *chartView;
    QValueAxis *axisX;
    QValueAxis *axisY;
};

#endif // CHARTWINDOW_H
