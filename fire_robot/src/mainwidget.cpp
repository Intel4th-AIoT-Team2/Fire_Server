#include "../include/fire_robot/mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(int argc, char** argv, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    //Tab1
    pTab1Camera = new Tab1Camera(ui->pTab1);
    ui->pTab1->setLayout(pTab1Camera->layout());

    pTab2RosControl = new Tab2RosControl(argc, argv, ui->pTab2);
    ui->pTab2->setLayout(pTab2RosControl->layout());

    pTab3Mapping = new Tab3Mapping(ui->pTab3);
    ui->pTab3->setLayout(pTab3Mapping->layout());

    ui->pTabWidget->setCurrentIndex(0);
}

MainWidget::~MainWidget()
{
    delete ui;
}

