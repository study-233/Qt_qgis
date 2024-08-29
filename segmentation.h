#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <QWidget>

namespace Ui {
class Segmentation;
}

class Segmentation : public QWidget
{
    Q_OBJECT

public:
    explicit Segmentation(QWidget *parent = nullptr);
    ~Segmentation();

private slots:

private:
    Ui::Segmentation *ui;
};

#endif // SEGMENTATION_H
