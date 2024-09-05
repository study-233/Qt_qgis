#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtMath>

//工具类
#include "segmentationdockwidget.h"

//库
#include <qmenu.h>                  //QT菜单类
#include <qaction.h>                //QT行为类

#include "qgsrasterlayer.h"         //QGIS栅格图层
#include "qgsvectorlayer.h"         //QGIS矢量图层
#include "qgsmaplayer.h"            //QGIS图层
#include "qgsmapcanvas.h"           //QGIS画布

#include "qgslayertreeview.h"              //QGIS图层管理器
#include "qgslayertreemapcanvasbridge.h"   //连接画布和图层管理器


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static MainWindow* instance() { return my; }

private slots:
    void on_actionOpen_raster_triggered();

    void on_actionOpen_vector_triggered();

    void on_actionRemove_file_triggered();

    void slot_autoSelectAddedLayer(QList<QgsMapLayer*> layers);

    void showAllLayers();

    void hideAllLayers();

    void showSelectedLayers();

    void hideSelectedLayers();

    void removeLayer();

    void slot_save_layout();

    void slot_restart_layout();
private:
    Ui::MainWindow *ui;
    static MainWindow *my;

    SegmentationDockWidget *mSegmentation = nullptr;
    QDockWidget* m_layerTreeDock = nullptr;

    QAction *save_layout,*restart_layout;
    QSettings *setting; //保存布局用的配置文件类,也可以用别的方式保存

    //地图画布
    QgsMapCanvas* mapCanvas;
    QList<QgsMapLayer*> layers;              //存储加载的图层

    //图层管理器
    QgsLayerTreeView* layerTreeView;
    QgsLayerTreeMapCanvasBridge* layerTreeCanvasBridge;

public:
    void initLayerTreeView();               //初始化图层管理器函数
    void addDockWidget(Qt::DockWidgetArea area, QDockWidget* dockwidget); //添加可悬浮窗口初始位置
    void legendLayerZoomNative();
};
#endif // MAINWINDOW_H
