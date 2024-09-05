#include "mainwindows.h"
#include "./UI/ui_mainwindows.h"
#include <QDebug>
#include <QToolBar>


//自己引入的库
#include "QTextCodec"
#include <qmenubar.h>                      //QT菜单栏库，用来存放菜单
#include <qfiledialog.h>                   //QT文件目录库，用来打开文件选择窗口
#include "qmessagebox.h"                   //QT信息盒子，用来显示操作提示

#include "qgslayertreemodel.h"            //使用给定层树构建新的树模型,一般与QgsLayerTreeView 一起使用
#include "qgridlayout.h"                  //栅格布局管理器

#include "qtoolbutton.h"                  //工具按钮,和普通工具相比可以带图标
#include "qdockwidget.h"                  //可悬浮窗口
#include "qgis_devlayertreeviewmenuprovider.h" //图层管理器右键菜单类
#include "qgslayertreeregistrybridge.h"        //创建与层树根同步给定项目的实例,收听地图层注册表中的更新，并在层树中进行更改。

MainWindow *MainWindow::my = nullptr;

MainWindow::~MainWindow()
{
    delete ui;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    my = this;

    setWindowTitle(u8"高分遥感图像分析");

    this->resize(1200, 800);               //设置窗口大小

    //初始化地图画布
    mapCanvas = new QgsMapCanvas();
    mapCanvas->setCachingEnabled(true);
    mapCanvas->setCanvasColor(QColor(255, 255, 255));   //设置地图画布为白色
    mapCanvas->setVisible(true);
    mapCanvas->enableAntiAliasing(true);

    //初始化图层管理器
    layerTreeView = new QgsLayerTreeView(this);
    initLayerTreeView();

    auto gridLayout = new QGridLayout;
    gridLayout->addWidget((QWidget*)mapCanvas);
    centralWidget()->setLayout(gridLayout);

    addDockWidget(Qt::LeftDockWidgetArea, m_layerTreeDock);        //初始位置
    ui->menuTool->addAction(m_layerTreeDock->toggleViewAction());

    mSegmentation = new SegmentationDockWidget(this);
    this->addDockWidget(Qt::LeftDockWidgetArea,mSegmentation);
    ui->menuTool->addAction(mSegmentation->toggleViewAction());


    QMenu *menu = ui->menubar->addMenu(u8"布局设置");

    save_layout = new QAction(u8"保存布局");
    connect(save_layout,&QAction::triggered,this,&MainWindow::slot_save_layout);
    menu->addAction(save_layout);

    restart_layout = new QAction(u8"恢复布局");
    connect(restart_layout,&QAction::triggered,this,&MainWindow::slot_restart_layout);
    menu->addAction(restart_layout);


    //exe执行程序旁边生成一个配置文件,用来寸界面布局QDockWidget类成员的信息,文件名随便取,格式随便写
    setting = new QSettings(QCoreApplication::applicationDirPath() + "/layout_config.txt",QSettings::IniFormat);
    setting->setIniCodec(QTextCodec::codecForName("utf-8"));//文件字节编码设置成utf-8

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
    //layerTreeView->close();                         //这里直接关闭图层管理器 ，需优化

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

    //右键菜单
    layerTreeView->setMenuProvider(new qgis_devLayerTreeViewMenuProvider(layerTreeView, mapCanvas));
    //connect(QgsProject::instance()->layerTreeRegistryBridge(), SIGNAL(addedLayersToLayerTree(const QList<QgsMapLayer*>)), this,
    //    SLOT(slot_autoSelectAddedLayer(const QList<QgsMapLayer*>)));
    connect(QgsProject::instance()->layerTreeRegistryBridge(), &QgsLayerTreeRegistryBridge::addedLayersToLayerTree, this,
            &MainWindow::slot_autoSelectAddedLayer);

    // 设置这个路径是为了获取图标文件
    QString iconDir = "E:\\QtDocuments\\My_Qgis\\image\\"; //注意包含中文路径图标会不显示，这里的图标可以去原QGIS软件中去找

    // QgsLayerTreeViewDefaultActions包含了默认实现的函数如添加Group
    QAction *actionAddGroup = new QAction( tr( "Add Group" ), this );
    actionAddGroup->setIcon( QIcon(iconDir + "mActionAddGroup.png"));
    actionAddGroup->setToolTip( tr( "Add Group" ) );
    connect( actionAddGroup, &QAction::triggered, layerTreeView->defaultActions(), &QgsLayerTreeViewDefaultActions::addGroup );

    QAction *actionShowAllLayers = new QAction( tr( "Show All Layers" ), this );
    actionShowAllLayers->setIcon( QIcon(iconDir + "mActionShowAllLayers.png"));
    actionShowAllLayers->setToolTip( tr( "Show All Layers" ) );
    connect(actionShowAllLayers,&QAction::triggered,this,&MainWindow::showAllLayers);

    QAction *actionHideAllLayers = new QAction( tr( "Hide All Layers" ), this );
    actionHideAllLayers->setIcon( QIcon(iconDir + "mActionHideAllLayers.png"));
    actionHideAllLayers->setToolTip( tr( "Hide All Layers" ) );
    connect(actionHideAllLayers,&QAction::triggered,this,&MainWindow::hideAllLayers);

    QAction *actionShowSelectedLayers = new QAction( tr( "Show Selected Layers" ), this );
    actionShowSelectedLayers->setIcon( QIcon(iconDir + "mActionShowSelectedLayers.png"));
    actionShowSelectedLayers->setToolTip( tr( "Show Selected Layers" ) );
    connect(actionShowSelectedLayers,&QAction::triggered,this,&MainWindow::showSelectedLayers);

    QAction *actionHideSelectedLayers = new QAction( tr( "Hide Selected Layers" ), this );
    actionHideSelectedLayers->setIcon( QIcon(iconDir + "mActionHideSelectedLayers.png"));
    actionHideSelectedLayers->setToolTip( tr( "Hide Selected Layers" ) );
    connect(actionHideSelectedLayers,&QAction::triggered,this,&MainWindow::hideSelectedLayers);

    // expand / collapse tool buttons
    QAction *actionExpandAll = new QAction( tr( "Expand All" ), this );
    actionExpandAll->setIcon( QIcon(iconDir + "mActionExpandTree.png") );
    actionExpandAll->setToolTip( tr( "Expand All" ) );
    connect( actionExpandAll, &QAction::triggered, layerTreeView, &QgsLayerTreeView::expandAllNodes );
    QAction *actionCollapseAll = new QAction( tr( "Collapse All" ), this );
    actionCollapseAll->setIcon( QIcon(iconDir + "mActionCollapseTree.png") );
    actionCollapseAll->setToolTip( tr( "Collapse All" ) );
    connect( actionCollapseAll, &QAction::triggered, layerTreeView, &QgsLayerTreeView::collapseAllNodes );

    QAction *actionRemoveLayer = new QAction( tr( "Remove Layer" ), this );
    actionRemoveLayer->setIcon( QIcon(iconDir + "mActionRemoveLayer.png") );
    actionRemoveLayer->setToolTip(tr( "Remvoe Layer" ));
    connect(actionRemoveLayer,&QAction::triggered,this,&MainWindow::removeLayer);

    //在DockWidget上部添加一个工具栏
    QToolBar *toolbar = new QToolBar();
    //    toolbar->setIconSize( iconSize( true ) );
    //    toolbar->addAction( mActionStyleDock );
    toolbar->addAction( actionAddGroup );
    toolbar->addAction( actionShowAllLayers );
    toolbar->addAction( actionHideAllLayers );
    toolbar->addAction( actionShowSelectedLayers );
    toolbar->addAction( actionHideSelectedLayers );
    toolbar->addAction( actionExpandAll );
    toolbar->addAction( actionCollapseAll );
    toolbar->addAction( actionRemoveLayer );

    //将工具栏和Layer Tree View添加到界面
    QVBoxLayout *vboxLayout = new QVBoxLayout;
    vboxLayout->setContentsMargins(0, 0, 0, 0);
    vboxLayout->setSpacing(0);
    vboxLayout->addWidget(toolbar);
    vboxLayout->addWidget(layerTreeView);

    // 装进dock widget中
    m_layerTreeDock = new QDockWidget(tr("Layer Tree"), this);
    m_layerTreeDock->setObjectName("Layers");
    m_layerTreeDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget* w = new QWidget();
    w->setLayout(vboxLayout);
    m_layerTreeDock->setWidget(w);

    // 连接地图画布和图层管理器
    layerTreeCanvasBridge = new QgsLayerTreeMapCanvasBridge(QgsProject::instance()->layerTreeRoot(), mapCanvas, this);
    connect(QgsProject::instance(), SIGNAL(writeProject(QDomDocument&)),
            layerTreeCanvasBridge, SLOT(writeProject(QDomDocument&)));
    connect(QgsProject::instance(), SIGNAL(readProject(QDomDocument)),
            layerTreeCanvasBridge, SLOT(readProject(QDomDocument)));
}

//
void MainWindow::slot_autoSelectAddedLayer(const QList<QgsMapLayer*> layers)
{
    if (!layers.isEmpty())
    {
        QgsLayerTreeLayer* nodeLayer = QgsProject::instance()->layerTreeRoot()->findLayer(layers[0]->id());

        if (!nodeLayer)
            return;

        QModelIndex index = layerTreeView->layerTreeModel()->node2index(nodeLayer);
        layerTreeView->setCurrentIndex(index);
    }
}

void MainWindow::addDockWidget(Qt::DockWidgetArea area, QDockWidget* dockwidget)
{
    QMainWindow::addDockWidget(area, dockwidget);
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    dockwidget->show();
    mapCanvas->refresh();
}


void MainWindow::legendLayerZoomNative()
{
    if ( !layerTreeView )
        return;

    //find current Layer
    QgsMapLayer *currentLayer = layerTreeView->currentLayer();
    if ( !currentLayer )
        return;

    if ( QgsRasterLayer *layer = qobject_cast<QgsRasterLayer *>( currentLayer ) )
    {
        QgsDebugMsgLevel( "Raster units per pixel  : " + QString::number( layer->rasterUnitsPerPixelX() ), 2 );
        QgsDebugMsgLevel( "MapUnitsPerPixel before : " + QString::number( mapCanvas->mapUnitsPerPixel() ), 2 );

        QList< double >nativeResolutions;
        if ( layer->dataProvider() )
        {
            nativeResolutions = layer->dataProvider()->nativeResolutions();
        }

        // get length of central canvas pixel width in source raster crs
        QgsRectangle e = mapCanvas->extent();
        QSize s = mapCanvas->mapSettings().outputSize();
        QgsPointXY p1( e.center().x(), e.center().y() );
        QgsPointXY p2( e.center().x() + e.width() / s.width(), e.center().y() + e.height() / s.height() );
        QgsCoordinateTransform ct( mapCanvas->mapSettings().destinationCrs(), layer->crs(), QgsProject::instance() );
        p1 = ct.transform( p1 );
        p2 = ct.transform( p2 );
        const double diagonalSize = std::sqrt( p1.sqrDist( p2 ) ); // width (actually the diagonal) of reprojected pixel
        if ( !nativeResolutions.empty() )
        {
            // find closest native resolution
            QList< double > diagonalNativeResolutions;
            diagonalNativeResolutions.reserve( nativeResolutions.size() );
            for ( double d : std::as_const( nativeResolutions ) )
                diagonalNativeResolutions << std::sqrt( 2 * d * d );

            int i;
            for ( i = 0; i < diagonalNativeResolutions.size() && diagonalNativeResolutions.at( i ) < diagonalSize; i++ )
            {
                QgsDebugMsgLevel( QStringLiteral( "test resolution %1: %2" ).arg( i ).arg( diagonalNativeResolutions.at( i ) ), 2 );
            }
            if ( i == nativeResolutions.size() ||
                ( i > 0 && ( ( diagonalNativeResolutions.at( i ) - diagonalSize ) > ( diagonalSize - diagonalNativeResolutions.at( i - 1 ) ) ) ) )
            {
                QgsDebugMsgLevel( QStringLiteral( "previous resolution" ), 2 );
                i--;
            }

            mapCanvas->zoomByFactor( nativeResolutions.at( i ) / mapCanvas->mapUnitsPerPixel() );
        }
        else
        {
            mapCanvas->zoomByFactor( std::sqrt( layer->rasterUnitsPerPixelX() * layer->rasterUnitsPerPixelX() + layer->rasterUnitsPerPixelY() * layer->rasterUnitsPerPixelY() ) / diagonalSize );
        }

        mapCanvas->refresh();
        QgsDebugMsgLevel( "MapUnitsPerPixel after  : " + QString::number( mapCanvas->mapUnitsPerPixel() ), 2 );
    }
}



void MainWindow::showAllLayers()
{
    QgsDebugMsgLevel( QStringLiteral( "Showing all layers!" ), 3 );
    layerTreeView->layerTreeModel()->rootGroup()->setItemVisibilityCheckedRecursive( true );
}

void MainWindow::hideAllLayers()
{
    QgsDebugMsgLevel( QStringLiteral( "hiding all layers!" ), 3 );

    const auto constChildren = layerTreeView->layerTreeModel()->rootGroup()->children();
    for ( QgsLayerTreeNode *node : constChildren )
    {
        node->setItemVisibilityCheckedRecursive( false );
    }
}

void MainWindow::showSelectedLayers()
{
    QgsDebugMsgLevel( QStringLiteral( "show selected layers!" ), 3 );

    const auto constSelectedNodes = layerTreeView->selectedNodes();
    for ( QgsLayerTreeNode *node : constSelectedNodes )
    {
        QgsLayerTreeNode *nodeIter = node;
        while ( nodeIter )
        {
            nodeIter->setItemVisibilityChecked( true );
            nodeIter = nodeIter->parent();
        }
    }
}

void MainWindow::hideSelectedLayers()
{
    QgsDebugMsgLevel( QStringLiteral( "hiding selected layers!" ), 3 );

    const auto constSelectedNodes = layerTreeView->selectedNodes();
    for ( QgsLayerTreeNode *node : constSelectedNodes )
    {
        node->setItemVisibilityChecked( false );
    }
}

void MainWindow::removeLayer()
{
    if(!layerTreeView)
        return;
    //    const QList<QgsMapLayer *> selectedLayers = mLayerTreeView->selectedLayersRecursive();

    if ( QMessageBox::warning(my, tr( "Remove layers and groups" ), tr("Are you sure?"), QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
    {
        return;
    }

    const QList<QgsLayerTreeNode *> selectedNodes = layerTreeView->selectedNodes( true );
    for ( QgsLayerTreeNode *node : selectedNodes )
    {
        if ( QgsLayerTreeGroup *group = qobject_cast< QgsLayerTreeGroup * >( node ) )
        {
            if ( QgsGroupLayer *groupLayer = group->groupLayer() )
            {
                QgsProject::instance()->removeMapLayer( groupLayer );
            }
        }
        QgsLayerTreeGroup *parentGroup = qobject_cast<QgsLayerTreeGroup *>( node->parent() );
        if ( parentGroup )
            parentGroup->removeChildNode( node );
    }
    mapCanvas->refresh();
}

//布局设置
void MainWindow::slot_save_layout()
{
    setting->setValue("LAYOUT/geometry",saveGeometry());//保存QDockWidget类成员的形状和大小
    setting->setValue("LAYOUT/state",saveState());//保存QDockWidget类成员的状态有的
    setting->sync();//立刻写入磁盘文件中
}

void MainWindow::slot_restart_layout()
{
    //下面读取形状大小和读取状态可以选择一个读取,根据喜好可以注释一行
    restoreGeometry(setting->value("LAYOUT/geometry").toByteArray());
    restoreState(setting->value("LAYOUT/state").toByteArray());

    QList<QDockWidget* > dwList = this->findChildren<QDockWidget*>();
    foreach(QDockWidget* dw,dwList){
        restoreDockWidget(dw);
    }
}
