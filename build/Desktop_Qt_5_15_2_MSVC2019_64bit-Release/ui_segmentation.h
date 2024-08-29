/********************************************************************************
** Form generated from reading UI file 'segmentation.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SEGMENTATION_H
#define UI_SEGMENTATION_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Segmentation
{
public:

    void setupUi(QWidget *Segmentation)
    {
        if (Segmentation->objectName().isEmpty())
            Segmentation->setObjectName(QString::fromUtf8("Segmentation"));
        Segmentation->resize(400, 300);

        retranslateUi(Segmentation);

        QMetaObject::connectSlotsByName(Segmentation);
    } // setupUi

    void retranslateUi(QWidget *Segmentation)
    {
        Segmentation->setWindowTitle(QCoreApplication::translate("Segmentation", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Segmentation: public Ui_Segmentation {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SEGMENTATION_H
