#include "../include/fire_robot/tab3mapping.h"
#include "ui_tab3mapping.h"

#include <QDebug>

Tab3Mapping::Tab3Mapping(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Tab3Mapping)
{
    ui->setupUi(this);

    // 터틀봇1 좌표
    turtle1_points.push_back(cv::Point2f(-0.3, -2.33));
    turtle1_points.push_back(cv::Point2f(-2.14, -0.094));
    turtle1_points.push_back(cv::Point2f(1.8, 2.1));
    turtle1_points.push_back(cv::Point2f(3.6, -0.058));

    // TODO: cam_image에는 선택한 카메라의 이미지(1장)가 표시되어야 함
    cam_image = cv::imread("/home/ubuntu/catkin_ws/src/fire_robot/room.png");
    map_image = cv::imread("/home/ubuntu/catkin_ws/src/fire_robot/turtle1_map.png");
    drawCorners();

    ui->labelCamView->installEventFilter(this);
    ui->labelMapView->installEventFilter(this);
    ui->labelInfoText->setText("준비");

    connect(ui->btnCamMapConnect, SIGNAL(clicked(bool)), this, SLOT(onBtnCamMapConnectClicked(bool)));
    connect(ui->btnMapAreaSet, SIGNAL(clicked(bool)), this, SLOT(onBtnMapAreaSetClicked(bool)));
    connect(ui->btnCam1Select, SIGNAL(clicked()), this, SLOT(onBtnCam1SelectClicked()));
    connect(ui->btnCam2Select, SIGNAL(clicked()), this, SLOT(onBtnCam2SelectClicked()));
}

Tab3Mapping::~Tab3Mapping()
{
    delete ui;
}

/* 이벤트 처리 */
bool Tab3Mapping::eventFilter(QObject *watched, QEvent *event)
{
    if (cam_map_connect_event_flag)
    {
        return camMapConnectEvent(watched, event);
    }
    else if (map_area_set_event_flag)
    {
        return mapAreaSetEvent(watched, event);
    }
    else
    {
        // 마우스 우클릭 (For Test)
        if (event->type() == QMouseEvent::MouseButtonPress){
            QMouseEvent *m_event = (QMouseEvent*)event;
            if (m_event->button() == Qt::RightButton)
            {
                if (watched == ui->labelCamView)
                {
                    test_point = cv::Point2f(m_event->pos().x(), m_event->pos().y());
                    drawCorners();
                    qDebug() << convertPointToTurtle(test_point, current_cam);
                }
            }
        }

        return false;
    }
}

/* 카메라-지도 사이의 좌표 변환을 위한 영역을 설정 */
bool Tab3Mapping::camMapConnectEvent(QObject *watched, QEvent *event)
{
    // 마우스 클릭 이벤트
    if (event->type() == QMouseEvent::MouseButtonPress)
    {
        QMouseEvent *m_event = (QMouseEvent*)event;

        // 마우스 좌클릭
        if (m_event->button() == Qt::LeftButton)
        {
            // 1. 카메라 좌표 선택
            if (active_cam_click && watched == ui->labelCamView)
            {
                std::vector<cv::Point2f> *temp_points;
                if (current_cam == 1)
                    temp_points = &cam1_points;
                else
                    temp_points = &cam2_points;

                // 4개가 추가될 때까지 반복
                temp_points->push_back(cv::Point2f(m_event->pos().x(), m_event->pos().y()));
                if (temp_points->size() == 4)
                {
                    active_cam_click = false;
                    active_map_click = true;
                    ui->labelInfoText->setText("카메라 영역에 대응하는 지도 영역 선택");
                }
            }
            // 2. 지도 좌표 선택
            else if (active_map_click && watched == ui->labelMapView)
            {
                std::vector<cv::Point2f> *temp_points;
                if (current_cam == 1)
                    temp_points = &map1_points;
                else
                    temp_points = &map2_points;

                temp_points->push_back(cv::Point2f(m_event->pos().x(), m_event->pos().y()));
                if (temp_points->size() == 4) {
                    if (current_cam == 1)
                        cam_map1_transform_array = cv::getPerspectiveTransform(cam1_points, map1_points);
                    else
                        cam_map2_transform_array = cv::getPerspectiveTransform(cam2_points, map2_points);

                    ui->btnCamMapConnect->click();
                }
            }

            drawCorners();
        }
    }

    return false;
}

/* 지도-터틀봇 사이의 좌표 변환을 위한 영역을 설정 */
bool Tab3Mapping::mapAreaSetEvent(QObject *watched, QEvent *event)
{
    // 마우스 클릭 이벤트
    if (event->type() == QMouseEvent::MouseButtonPress)
    {
        QMouseEvent *m_event = (QMouseEvent*)event;

        // 마우스 좌클릭
        if (m_event->button() == Qt::LeftButton)
        {
            // 지도 좌표 선택
            if (active_map_click && watched == ui->labelMapView)
            {
                std::vector<cv::Point2f> *temp_points;
                temp_points = &turtle_map_points;

                temp_points->push_back(cv::Point2f(m_event->pos().x(), m_event->pos().y()));
                if (temp_points->size() == 4) {
                    map_turtle_transform_array = cv::getPerspectiveTransform(turtle_map_points, turtle1_points);

                    ui->btnMapAreaSet->click();
                }
            }

            drawCorners();
        }
    }
    return false;
}

/* 이미지 갱신 */
void Tab3Mapping::updateImageView(cv::Mat &image, QLabel *label)
{
    if (image.empty())
    {
        return;
    }

    cv::Mat rgb_image;
    cv::cvtColor(image, rgb_image, cv::COLOR_BGR2RGB);

    QImage q_image(rgb_image.data, rgb_image.cols, rgb_image.rows, rgb_image.step, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(q_image);

    label->setFixedSize(pixmap.size());
    label->setPixmap(pixmap);
}

/* 선택한 영역을 이미지에 표시 */
void Tab3Mapping::drawCorners()
{
    cv::Mat temp_cam_image = cam_image.clone();
    cv::Mat temp_map_image = map_image.clone();
    std::vector<cv::Point2f> temp_cam_points;
    std::vector<cv::Point2f> temp_map_points;
    cv::Mat temp_transform_array;

    // select cam
    if (current_cam == 1)
    {
        temp_cam_points = cam1_points;
        temp_map_points = map1_points;
        temp_transform_array = cam_map1_transform_array;
    }
    else if (current_cam == 2)
    {
        temp_cam_points = cam2_points;
        temp_map_points = map2_points;
        temp_transform_array = cam_map2_transform_array;
    }
    else
    {
        return;
    }

    // draw circle
    for (int i = 0; i < (int)temp_cam_points.size(); ++i)
    {
        cv::circle(temp_cam_image, temp_cam_points[i], 2, cv::Scalar(0, 255, 0), 2);
    }
    for (int i = 0; i < (int)temp_map_points.size(); ++i)
    {
        cv::circle(temp_map_image, temp_map_points[i], 2, cv::Scalar(0, 255, 0), 2);
    }
    for (int i = 0; i < (int)turtle_map_points.size(); ++i)
    {
        cv::circle(temp_map_image, turtle_map_points[i], 2, cv::Scalar(255, 0, 255), 2);
    }

    // draw line
    if (temp_cam_points.size() == 4)
    {
        cv::line(temp_cam_image, temp_cam_points[0], temp_cam_points[1], cv::Scalar(255, 0, 0), 1);
        cv::line(temp_cam_image, temp_cam_points[1], temp_cam_points[2], cv::Scalar(255, 0, 0), 1);
        cv::line(temp_cam_image, temp_cam_points[2], temp_cam_points[3], cv::Scalar(255, 0, 0), 1);
        cv::line(temp_cam_image, temp_cam_points[3], temp_cam_points[0], cv::Scalar(255, 0, 0), 1);
    }
    if (temp_map_points.size() == 4)
    {
        cv::line(temp_map_image, temp_map_points[0], temp_map_points[1], cv::Scalar(255, 0, 0), 1);
        cv::line(temp_map_image, temp_map_points[1], temp_map_points[2], cv::Scalar(255, 0, 0), 1);
        cv::line(temp_map_image, temp_map_points[2], temp_map_points[3], cv::Scalar(255, 0, 0), 1);
        cv::line(temp_map_image, temp_map_points[3], temp_map_points[0], cv::Scalar(255, 0, 0), 1);
    }
    if (turtle_map_points.size() == 4)
    {
        cv::line(temp_map_image, turtle_map_points[0], turtle_map_points[1], cv::Scalar(0, 0, 255), 1);
        cv::line(temp_map_image, turtle_map_points[1], turtle_map_points[2], cv::Scalar(0, 0, 255), 1);
        cv::line(temp_map_image, turtle_map_points[2], turtle_map_points[3], cv::Scalar(0, 0, 255), 1);
        cv::line(temp_map_image, turtle_map_points[3], turtle_map_points[0], cv::Scalar(0, 0, 255), 1);
    }

    // draw target (For Test)
    if (isCamMapReady(current_cam) && turtle_map_points.size() == 4)
    {
        std::vector<cv::Point2f> src, dst, test;
        src.push_back(test_point);

        cv::circle(temp_cam_image, test_point, 2, cv::Scalar(0, 0, 255), 2);
        cv::perspectiveTransform(src, dst ,temp_transform_array);
        cv::circle(temp_map_image, dst[0], 2, cv::Scalar(0, 0, 255), 2);
        cv::perspectiveTransform(dst, test, map_turtle_transform_array);
    }

    // update images
    updateImageView(temp_cam_image, ui->labelCamView);
    updateImageView(temp_map_image, ui->labelMapView);
}

/* 카메라-지도 변환의 모서리가 설정 되었는지 여부 */
bool Tab3Mapping::isCamMapReady(int num)
{
    switch (num)
    {
    case 1:
        return (cam1_points.size() == 4) && (map1_points.size() == 4);
    case 2:
        return (cam2_points.size() == 4) && (map2_points.size() == 4);
    default:
        return false;
    }
}

/* 모든 설정이 완료되어 좌표 전송이 준비된 상태의 여부 */
bool Tab3Mapping::isMappingDone()
{
    // return isCamMapReady(1) && isCamMapReady(2) && isMapAreaReady();
    return isCamMapReady(1) && turtle_map_points.size() == 4;
}

/* "카메라-지도 연결" 버튼 클릭 이벤트 */
void Tab3Mapping::onBtnCamMapConnectClicked(bool checked)
{
    if (checked)
    {
        if (current_cam == 1)
        {
            cam1_points.clear();
            std::vector<cv::Point2f>().swap(cam1_points);
            map1_points.clear();
            std::vector<cv::Point2f>().swap(map1_points);          
        }
        else if (current_cam == 2)
        {
            cam2_points.clear();
            std::vector<cv::Point2f>().swap(cam2_points);
            map2_points.clear();
            std::vector<cv::Point2f>().swap(map2_points);
        }

        cam_map_connect_event_flag = true;
        active_cam_click = true;      // 카메라 좌표부터 설정
        ui->labelInfoText->setText("카메라 영역을 좌상단부터 시계방향으로 지정");
    }
    else {
        cam_map_connect_event_flag = false;
        active_cam_click = false;
        active_map_click = false;

        if ((current_cam == 1) && !isCamMapReady(1)) {
            ui->labelInfoText->setText("취소되었습니다.");
        }
        else if ((current_cam == 2) && !isCamMapReady(2))
        {
            ui->labelInfoText->setText("취소되었습니다.");
        }
        else {
            QString str = QString("카메라%1 맵핑이 완료되었습니다.").arg(current_cam);
            ui->labelInfoText->setText(str);
        }
    }
}

/* "지도 영역 설정" 버튼 클릭 이벤트 */
void Tab3Mapping::onBtnMapAreaSetClicked(bool checked)
{
    if (checked)
    {
        turtle_map_points.clear();
        std::vector<cv::Point2f>().swap(turtle_map_points);

        map_area_set_event_flag = true;
        active_map_click = true;
        ui->labelInfoText->setText("지도 영역을 좌상단부터 시계방향으로 지정");
    }
    else
    {
        map_area_set_event_flag = false;
        active_map_click = false;

        if (turtle_map_points.size() != 4)
        {
            ui->labelInfoText->setText("취소되었습니다.");
        }
        else
        {
            ui->labelInfoText->setText("지도 영역 설정이 완료되었습니다.");
        }
    }
}

/* "카메라1" 버튼 클릭 이벤트 */
void Tab3Mapping::onBtnCam1SelectClicked()
{
    current_cam = 1;
    drawCorners();
    ui->labelInfoText->setText("카메라1이(가) 선택되었습니다.");
}

/* "카메라2" 버튼 클릭 이벤트 */
void Tab3Mapping::onBtnCam2SelectClicked()
{
    current_cam = 2;
    drawCorners();
    ui->labelInfoText->setText("카메라2이(가) 선택되었습니다.");
}

/* 터틀봇의 지도 좌표로 변환 */
QString Tab3Mapping::convertPointToTurtle(cv::Point2f cam_point, int cam_number)
{
    QString str;
    std::vector<cv::Point2f> cam, map, turtle;
    cam.push_back(cam_point);

    if (!isMappingDone())
    {
        str = "ERROR@Mapping not done";
        return str;
    }

    if (cam_number == 1)
    {
        cv::perspectiveTransform(cam, map, cam_map1_transform_array);       // cam1 -> map1
        cv::perspectiveTransform(map, turtle, map_turtle_transform_array);  // map1 -> turtle
    }
    else
    {
        cv::perspectiveTransform(cam, map, cam_map2_transform_array);       // cam2 -> map2
        cv::perspectiveTransform(map, turtle, map_turtle_transform_array);  // map2 -> turtle
    }

    str = QString("TURTLE@%1@%2@%3@%4").arg(turtle[0].x).arg(turtle[0].y).arg(0).arg(0.5);
    return str;
}
