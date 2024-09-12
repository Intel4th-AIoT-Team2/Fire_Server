#ifndef TAB2ROSCONTROL_H
#define TAB2ROSCONTROL_H

#include <QWidget>
#include <QTcpSocket>
#include <QButtonGroup>
#include <QPushButton>
#include <iostream>
#include <array>
#include "rosnode.h"
#include "server.h"

namespace Ui {
class Tab2RosControl;
}

class Tab2RosControl : public QWidget
{
    Q_OBJECT

public:
    explicit Tab2RosControl(int argc, char **argv, QWidget *parent = nullptr);
    ~Tab2RosControl();

private:
    Ui::Tab2RosControl *ui;
    RosNode *prosNode;
    QButtonGroup *bg;
    QWidget *widget;
    Server *server;
    QTcpSocket *clientSocket;

private slots:
    void goal_Pub();
    void goal_PubSlot(double, double, double, double);
    void goal_Pub_Front();
    void goal_Pub_Living();
    void goal_Pub_Study();
    void goal_Pub_Bedroom();
    void set_Pub();
    void slotLdsReceive(float *);
    void sendData(/*QString*/);
    void saveSocket(QTcpSocket*);
};

#endif // TAB2ROSCONTROL_H
