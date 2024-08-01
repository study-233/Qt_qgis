#include "mainwindows.h"
#include "./ui_mainwindow.h"
#include <QDebug>

//自己引入的库
#include <qmenubar.h>                      //QT菜单栏库，用来存放菜单
#include <qfiledialog.h>                   //QT文件目录库，用来打开文件选择窗口
#include "qmessagebox.h"                   //QT信息盒子，用来显示操作提示

#include "qgslayertreemodel.h"            //使用给定层树构建新的树模型,一般与QgsLayerTreeView 一起使用
#include "qgridlayout.h"                  //栅格布局管理器

MainWindow::~MainWindow()
{
    delete ui;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug()<<Qgis::releaseName();

    this->resize(1200, 800);               //设置MyQGIS01类窗口大小

    //初始化地图画布
    mapCanvas = new QgsMapCanvas();
    //this->setCentralWidget(mapCanvas);                  //将地图画布放置我们创建的主窗口中心
    mapCanvas->setCanvasColor(QColor(255, 255, 255));   //设置地图画布为白色
    mapCanvas->setVisible(true);
    mapCanvas->enableAntiAliasing(true);

    //----------------------

    //初始化图层管理器
    layerTreeView = new QgsLayerTreeView(this);
    initLayerTreeView();

    //----------------------

    //栅格布局
    QWidget* centralWidget = this->centralWidget();
    QGridLayout* centralLayout = new QGridLayout(centralWidget);
    centralLayout->addWidget(layerTreeView, 0, 0, 2, 1);                 //图层管理器位置
    centralLayout->addWidget(mapCanvas, 0, 1, 2, 1);                     //画布位置 后面四个参数：初始行列 占位行类

}



void MainWindow::on_actionOpen_raster_triggered()
{
    //步骤1：打开文件选择对话框
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open raster file"), "", "remote sensing image(*.tif *.tiff);;image(*.jpg *.jpeg *.png *.bmp)");
    if (fileName.isNull()) //如果文件未选择则返回
    {
        return;
    }
    QStringList temp = fileName.split('/');
    QString basename = temp.at(temp.size() - 1);//获取栅格数据名称

    //步骤2：创建QgsRasterLayer类
    QgsRasterLayer* rasterLayer = new QgsRasterLayer(fileName, basename, "gdal");
    //如果不是geotiff文件，则提示错误
    if (!rasterLayer->isValid())
    {
        QMessageBox::critical(this, "error", QString("layer is invalid: \n") + fileName);
        return;
    }

    //步骤3：添加栅格数据
    QgsProject::instance()->addMapLayer(rasterLayer); //注册
    mapCanvas->setExtent(rasterLayer->extent());     //将画布范围设置为栅格图层范围
    layers.append(rasterLayer);                      //将栅格图层追加到链表中
    mapCanvas->setLayers(layers);                    //将图层画到画布上
    mapCanvas->setVisible(true);
    mapCanvas->freeze(false);
    mapCanvas->refresh();                           //更新画布

}


void MainWindow::on_actionOpen_vector_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open shape file"), "", "*.shp");
    if (fileName.isNull()) //如果文件未选择则返回
    {
        return;
    }

    QStringList temp = fileName.split('/');
    QString basename = temp.at(temp.size() - 1);
    QgsVectorLayer* vecLayer = new QgsVectorLayer(fileName, basename, "ogr");

    if (!vecLayer->isValid())
    {
        QMessageBox::critical(this, "error", QString("layer is invalid: \n") + fileName);
        return;
    }

    //QGIS 3注册方式
    QgsProject::instance()->addMapLayer(vecLayer);

    mapCanvas->setExtent(vecLayer->extent());
    layers.append(vecLayer);
    mapCanvas->setLayers(layers);
    mapCanvas->setVisible(true);
    mapCanvas->freeze(false);
    mapCanvas->refresh();

}



void MainWindow::on_actionRemove_file_triggered()
{
    layers.clear();                                 //从链表中清除所有图层
    mapCanvas->setLayers(layers);                   //将图层画到画布上
    mapCanvas->setVisible(true);
    mapCanvas->freeze(false);
    mapCanvas->refresh();                           //更新画布

    layerTreeView->close();                         //这里直接关闭图层管理器 ，需优化


}
//初始化图层管理器
void MainWindow::initLayerTreeView()
{
    QgsLayerTreeModel* model = new QgsLayerTreeModel(QgsProject::instance()->layerTreeRoot(), this);
    model->setFlag(QgsLayerTreeModel::AllowNodeRename);
    model->setFlag(QgsLayerTreeModel::AllowNodeReorder);
    model->setFlag(QgsLayerTreeModel::AllowNodeChangeVisibility);
    model->setFlag(QgsLayerTreeModel::ShowLegendAsTree);
    model->setAutoCollapseLegendNodes(10);
    layerTreeView->setModel(model);
    layerTreeView->setFixedWidth(200);


    // 连接地图画布和图层管理器
    layerTreeCanvasBridge = new QgsLayerTreeMapCanvasBridge(QgsProject::instance()->layerTreeRoot(), mapCanvas, this);
    connect(QgsProject::instance(), SIGNAL(writeProject(QDomDocument&)),
            layerTreeCanvasBridge, SLOT(writeProject(QDomDocument&)));
    connect(QgsProject::instance(), SIGNAL(readProject(QDomDocument)),
            layerTreeCanvasBridge, SLOT(readProject(QDomDocument)));
}
