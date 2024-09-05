#include "segmentationdockwidget.h"
#include <qfiledialog.h>                   //QT文件目录库，用来打开文件选择窗口
#include "./UI/ui_segmentationdockwidget.h"

SegmentationDockWidget::SegmentationDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::SegmentationDockWidget)
{
    ui->setupUi(this);
    setWindowTitle("Segmentation");

}

SegmentationDockWidget::~SegmentationDockWidget()
{
    delete ui;
}

void SegmentationDockWidget::on_pushButton_selectUpl_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"));
    if (fileName.isNull()) //如果文件未选择则返回
    {
        return;
    }
    ui->lineEdit_Path->setText(fileName);
}


void SegmentationDockWidget::on_pushButton_selectSvg_clicked()
{
    QString fileName = QFileDialog::getExistingDirectory(this, tr("Open file"));
    if (fileName.isNull()) //如果文件未选择则返回
    {
        return;
    }
    ui->lineEdit_svgPath->setText(fileName);
}

void SegmentationDockWidget::on_pushButton_start_clicked()
{

}

