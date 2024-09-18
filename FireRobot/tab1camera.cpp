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
    qDeleteAll(clients);
    clients.clear();

    server->close();
    delete server;
    delete ui;
}

void Tab1Camera::slotNewConnection() {
    QTcpSocket *newClient = server->nextPendingConnection();

    newClient->setSocketOption(QAbstractSocket::LowDelayOption, 1);  // 지연 최소화
    newClient->setSocketOption(QAbstractSocket::KeepAliveOption, 1); // 연결 유효 확인

    connect(newClient, SIGNAL(readyRead()), this, SLOT(slotReadData()));
    connect(newClient, SIGNAL(disconnected()), this, SLOT(slotClientDisconnected()));

    // 클라이언트가 없을 때만 추가
    if (clients.size() < 2) {
        clients.append(newClient);  // 클라이언트 목록에 추가

        // 첫 번째 클라이언트는 첫 번째 라벨에 매핑, 두 번째 클라이언트는 두 번째 라벨에 매핑
        if (clients.size() == 1) {
            clientLabelMap[newClient] = ui->pTLcamView1;
        } else if (clients.size() == 2) {
            clientLabelMap[newClient] = ui->pTLcamView2;
        }

        QByteArray response = "서버와 연결되었습니다.";
        newClient->write(response);
        newClient->flush();
    }
    else {
        // 세 번째 클라이언트는 받지 않음
        newClient->disconnect();
        newClient->deleteLater();
    }
}

void Tab1Camera::slotReadData() {
    QTcpSocket *senderClient = qobject_cast<QTcpSocket*>(sender());

    if (!senderClient)
        return;

    static QByteArray buffer;
    buffer.append(senderClient->readAll());

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
            processFrame(senderClient, img);  // 수신한 프레임을 처리
        }
    }
}

void Tab1Camera::processFrame(QTcpSocket *client, cv::Mat& frame) {
    // OpenCV로 영상 처리: 나중에 화재 탐지 모델 적용 가능
    // 현재는 수신된 프레임을 그대로 화면에 출력
    // 화재 탐지 모델 예시
    // cv::Mat processedFrame = fireDetectionModel.detect(frame);

    if (frame.empty())
        return;

    // QImage로 변환
    QImage qimg(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_BGR888);

    // 고정된 클라이언트-라벨 매핑에 따라 출력
    if (clientLabelMap.contains(client) && !qimg.isNull()) {
        QLabel *label = clientLabelMap[client];
        label->setPixmap(QPixmap::fromImage(qimg));
    }
}

void Tab1Camera::slotClientDisconnected() {
    QTcpSocket *senderClient = qobject_cast<QTcpSocket*>(sender());

    if (senderClient) {
        clients.removeOne(senderClient);  // 클라이언트 목록에서 제거
        clientLabelMap.remove(senderClient);  // 클라이언트와 라벨 매핑 제거
        senderClient->deleteLater();

        if (senderClient == clients[0])
            qDebug() << "CCTV1과 연결이 끊어졌습니다.";
        else if (senderClient == clients[1])
            qDebug() << "CCTV2와 연결이 끊어졌습니다.";
    }
}
