#include "segmentation.h"
#include "ui_segmentation.h"

Segmentation::Segmentation(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Segmentation)
{
    ui->setupUi(this);
}

Segmentation::~Segmentation()
{
    delete ui;
}


