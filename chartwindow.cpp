#include "chartwindow.h"

ChartWindow::ChartWindow(QString title, QString xLabel, QString yLabel, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(title);
    resize(800, 600);

    series = new QLineSeries();

    chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(title);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    axisX = new QValueAxis();
    axisX->setTitleText(xLabel);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    axisY = new QValueAxis();
    axisY->setTitleText(yLabel);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(false);

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(chartView);
    setLayout(layout);
}

ChartWindow::~ChartWindow()
{
}

void ChartWindow::setData(const std::vector<std::pair<double, double>>& data)
{
    series->clear();

    if (data.empty()) return;

    double minX = data[0].first;
    double maxX = data[0].first;
    double minY = data[0].second;
    double maxY = data[0].second;

    for (const auto& point : data)
    {
        series->append(point.first, point.second);

        if (point.first < minX) minX = point.first;
        if (point.first > maxX) maxX = point.first;
        if (point.second < minY) minY = point.second;
        if (point.second > maxY) maxY = point.second;
    }

    axisX->setRange(minX, maxX);
    axisY->setRange(0, maxY * 1.1);
}
