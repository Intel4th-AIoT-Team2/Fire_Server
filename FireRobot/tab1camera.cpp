#include "tab1camera.h"
#include "ui_tab1camera.h"

Tab1Camera::Tab1Camera(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Tab1Camera)
{
    ui->setupUi(this);

    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
    server->listen(QHostAddress::Any, 5000);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(slotProcessImage()));
    timer->start(30);
}

Tab1Camera::~Tab1Camera()
{
    if (client) {
        client->disconnect();
        client->close();
        delete client;
    }
    server->close();
    delete server;
    delete ui;
}

void Tab1Camera::slotNewConnection() {
    if (client) {
        client->deleteLater();
    }
    client = server->nextPendingConnection();

    // 클라이언트 연결에 타임아웃 설정
    client->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    client->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

    connect(client, SIGNAL(readyRead()), this, SLOT(slotReadData()));
    connect(client, SIGNAL(disconnect()), this, SLOT(slotClientDisconnected()));

    QByteArray response = "서버와 연결되었습니다.";
    client->write(response);
    client->flush();
}

void Tab1Camera::slotReadData() {
    if (!client)
        return;

    static QByteArray buffer;
    buffer.append(client->readAll());

    while (buffer.size() >= 16) {
        bool ok;
        int length = buffer.left(16).toInt(&ok);
        if (!ok) {
            buffer.clear();  // 유효하지 않은 데이터
            return;
        }

        if (buffer.size() < length + 16) {
            return; // 아직 전체 데이터를 받지 않았음
        }

        QByteArray imageData = buffer.mid(16, length);
        buffer.remove(0, 16 + length);  // 사용한 데이터 제거

        QImage image;
        image.loadFromData(imageData);
        if (!image.isNull()) {
            ui->pTLcamView->setPixmap(QPixmap::fromImage(image));
        }
    }
}

void Tab1Camera::slotClientDisconnected() {
    client->deleteLater();
    client = nullptr;
    qDebug() << "클라이언트가 연결을 끊었습니다.";
}

void Tab1Camera::slotProcessImage() {
    // 추가 처리 기능
}
