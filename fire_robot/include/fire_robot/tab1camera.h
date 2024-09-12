#ifndef TAB1CAMERA_H
#define TAB1CAMERA_H

#include <QWidget>
#include <QTimer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QImage>
#include <QPixmap>
#include <QDebug>
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
    QTcpSocket *client;
    QTimer *timer;

    QImage cam1_image;

public slots:
    void slotCopyCam1Image(cv::Mat&);

private slots:
    void slotNewConnection();
    void slotReadData();
    void slotClientDisconnected();
    void slotProcessImage();
};

#endif // TAB2CAMERA_H
