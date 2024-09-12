#ifndef TAB3MAPPING_H
#define TAB3MAPPING_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <QImage>
#include <QPixmap>
#include <opencv2/opencv.hpp>
#include <vector>

namespace Ui {
class Tab3Mapping;
}

class Tab3Mapping : public QWidget
{
    Q_OBJECT

public:
    explicit Tab3Mapping(QWidget *parent = nullptr);
    ~Tab3Mapping();

private:
    Ui::Tab3Mapping *ui;

    cv::Mat cam_image;                      // 카메라 이미지
    cv::Mat map_image;                      // 지도 이미지

    /* 상태 변수 */
    int current_cam = 1;                    // 현재 선택된 카메라
    bool active_cam_click = false;          // 카메라 좌표 선택 모드
    bool active_map_click = false;          // 지도 좌표 선택 모드

    /* 이벤트 플래그 */
    bool cam_map_connect_event_flag = false;
    bool map_area_set_event_flag = false;

    /* 카메라-지도 변환 */
    std::vector<cv::Point2f> cam1_points;
    std::vector<cv::Point2f> map1_points;
    std::vector<cv::Point2f> cam2_points;
    std::vector<cv::Point2f> map2_points;
    cv::Mat cam_map1_transform_array;       // 카메라-지도 좌표 변환 배열
    cv::Mat cam_map2_transform_array;

    /* 지도-터틀봇 변환 */
    std::vector<cv::Point2f> turtle_map_points;
    std::vector<cv::Point2f> turtle1_points;
    cv::Mat map_turtle_transform_array;     // 지도-터틀봇 좌표 변환 배열

    cv::Point2f test_point = cv::Point2f(0.0, 0.0);

public:
    bool isCamMapReady(int);                // 카메라-지도 변환의 모서리가 설정 되었는지 여부
    bool isMappingDone();                   // 모든 설정이 완료되어 좌표 전송이 준비된 상태의 여부

private:
    bool eventFilter(QObject*, QEvent*);
    bool camMapConnectEvent(QObject*, QEvent*);
    bool mapAreaSetEvent(QObject*, QEvent*);

    void updateImageView(cv::Mat&, QLabel*);
    void drawCorners();

private slots:
    void onBtnCam1SelectClicked();
    void onBtnCam2SelectClicked();
    void onBtnCamMapConnectClicked(bool);
    void onBtnMapAreaSetClicked(bool);

public slots:
    QString convertPointToTurtle(cv::Point2f, int);
};

#endif // TAB3MAPPING_H
