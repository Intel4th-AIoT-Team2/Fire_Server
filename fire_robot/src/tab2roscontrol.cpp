#include "../include/fire_robot/tab2roscontrol.h"
#include "ui_tab2roscontrol.h"
//#include "../include/fire_robot/socketclient.h"

Tab2RosControl::Tab2RosControl(int argc, char **argv, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Tab2RosControl)
{
    ui->setupUi(this);
    prosNode = new RosNode(argc, argv);
    prosNode->pLcamView = ui->pLcamView;
    prosNode->start();

    // Server setup
    server = new Server(this);
    if (!server->listen(QHostAddress::Any, 5001)) {
        qCritical() << "Unable to start server:" << server->errorString();
    } else {
        qDebug() << "Server started on port 5000";
    }

    connect(ui->pushButtonGo, SIGNAL(clicked()),this, SLOT(goal_Pub()));
    connect(ui->pPBFront, SIGNAL(clicked()),this, SLOT(goal_Pub_Front()));
    connect(ui->pPBLiving, SIGNAL(clicked()),this, SLOT(goal_Pub_Living()));
    connect(ui->pPBStudy, SIGNAL(clicked()),this, SLOT(goal_Pub_Study()));
    connect(ui->pPBedroom, SIGNAL(clicked()),this, SLOT(goal_Pub_Bedroom()));
    connect(ui->sendBtn, SIGNAL(clicked()),this, SLOT(sendData()));
    connect(prosNode, SIGNAL(sigLdsReceive(float *)),this, SLOT(slotLdsReceive(float *)));
    connect(prosNode, SIGNAL(rosShutdown()),qApp, SLOT(quit()));

    connect(server, SIGNAL(newConnect(QTcpSocket*)), this, SLOT(saveSocket(QTcpSocket*)));
//    connect(server, SIGNAL(newConnect()), this, [this](QTcpSocket *newSocket) {
//        clientSocket = newSocket; // 클라이언트 소켓 저장
//    });
}

Tab2RosControl::~Tab2RosControl()
{
    delete ui;
}

void Tab2RosControl::goal_Pub()
{
    prosNode->go_goal("map",ui->doubleSpinBox1->value(),ui->doubleSpinBox2->value(), ui->doubleSpinBox3->value(),ui->doubleSpinBox4->value());

}
void Tab2RosControl::goal_PubSlot(double x, double y, double z, double w)
{
   prosNode->go_goal("map",x,y,z,w);
}
void Tab2RosControl::goal_Pub_Front()
{
   prosNode->go_goal("map",0.83, 2.92, 0.0, 0.72);
}
void Tab2RosControl::goal_Pub_Living()
{
   prosNode->go_goal("map",0.077, 0.19, 0.0, 0.79);
}
void Tab2RosControl::goal_Pub_Study()
{
   prosNode->go_goal("map",4.96, 6.366, 0.0, 0.62);
}
void Tab2RosControl::goal_Pub_Bedroom()
{
   prosNode->go_goal("map",7.78, 1.05, 0.0, 0.59);
}
void Tab2RosControl::set_Pub()
{
   prosNode->set_goal("map",0.83, 2.92, 0.0, 0.72);
}
void Tab2RosControl::slotLdsReceive(float *pscanData)
{
    ui->lcdNumber1->display(pscanData[0]);
    ui->lcdNumber2->display(pscanData[1]);
    ui->lcdNumber3->display(pscanData[2]);
    ui->lcdNumber4->display(pscanData[3]);
}
void Tab2RosControl::sendData(/*QString data*/)
{
    qDebug() << "BtnClick";
    //data = "10@20.1@30.2";
    //prosNode->sendDataToRos(data.toStdString());
	if (clientSocket && clientSocket->isOpen()) {
        // QString message = "Message from the button click!";
        QString message = "[18]GOAL@2@4@1@1";
        clientSocket->write(message.toUtf8());
        qDebug() << "Message sent to client:" << message;
		// 클라이언트의 IP 주소와 포트 번호를 가져옵니다.
        QString clientInfo = QString("IP: %1, Port: %2")
            .arg(clientSocket->peerAddress().toString())
            .arg(clientSocket->peerPort());

        qDebug() << "Message sent to client:" << message << "->" << clientInfo;
    } else {
        qDebug() << "Message sent to client: failed";
        qCritical() << "No client connected or socket not open!";
    }

}
void Tab2RosControl::saveSocket(QTcpSocket* socket)
{
    clientSocket = socket;
}
