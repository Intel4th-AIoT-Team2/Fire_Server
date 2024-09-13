#ifndef TAB1CAMERA_H
#define TAB1CAMERA_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QImage>
#include <QDebug>
#include <QList>
#include <opencv2/opencv.hpp>

namespace Ui {
class Tab1Camera;
}

class Tab1Camera : public QWidget
{
    Q_OBJECT

public:
    explicit Tab1Camera(QWidget *parent = nullptr);
    ~Tab1Camera();

private:
    Ui::Tab1Camera *ui;
    QTcpServer *server;
    QList<QTcpSocket *> clients;
    void processFrame(QTcpSocket*, cv::Mat&);
    void displayImage(QTcpSocket*, const cv::Mat&);

private slots:
    void slotNewConnection();
    void slotReadData();
    void slotClientDisconnected();
};

#endif // TAB2CAMERA_H
