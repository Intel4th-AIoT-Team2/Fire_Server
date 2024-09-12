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
        client->disconnect();
        client->deleteLater();
    }
    client = server->nextPendingConnection();

    // 클라이언트 연결에 타임아웃 설정
    client->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    client->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

    connect(client, SIGNAL(readyRead()), this, SLOT(slotReadData()));
    connect(client, SIGNAL(disconnected()), this, SLOT(slotClientDisconnected()));

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
        bool ok = false;
        int length = buffer.left(16).toInt(&ok);
        if (!ok || length <= 0) {
            buffer.clear();
            return;
        }

        if (buffer.size() < length + 16) {
            return; // 아직 전체 데이터를 받지 않았음
        }

        QByteArray imageData = buffer.mid(16, length);
        buffer.remove(0, 16 + length);  // 사용한 데이터 제거

        // OpenCV로 수신된 데이터 처리
        cv::Mat img = cv::imdecode(std::vector<uchar>(imageData.begin(), imageData.end()), cv::IMREAD_COLOR);
        if (!img.empty()) {
            processFrame(img);  // 수신한 프레임을 처리
        }
    }
}

void Tab1Camera::processFrame(cv::Mat& frame) {
    // OpenCV로 영상 처리: 나중에 화재 탐지 모델 적용 가능
    // 현재는 수신된 프레임을 그대로 화면에 출력
    // 화재 탐지 모델 예시
    // cv::Mat processedFrame = fireDetectionModel.detect(frame);

    if (frame.empty())
        return;

    // QImage로 변환
    QImage qimg(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_BGR888);

    // 영상 출력
    if (!qimg.isNull()) {
        ui->pTLcamView1->setPixmap(QPixmap::fromImage(qimg));
    }
}

void Tab1Camera::slotClientDisconnected() {
    client->deleteLater();
    client = nullptr;
    qDebug() << "클라이언트가 연결을 끊었습니다.";
}
