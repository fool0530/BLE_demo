#ifndef BLEWIDITEM_H
#define BLEWIDITEM_H

#include<QDebug>

#include <QWidget>
#include<QPushButton>
#include<QComboBox>

namespace Ui {
class BleWidItem;
}

class BleWidItem : public QWidget
{
    Q_OBJECT

public:
    explicit BleWidItem(QWidget *parent = nullptr);
    ~BleWidItem();

    QPushButton *btn_Name;
    QComboBox *Box_Uuid;
    QPushButton *btn_Rssi;

    int index = 0;



private slots:
    void on_btnclick();

    void on_BoxUidChanged(int index);


signals:
    void emit_send(int);
    void BoxUidChanged(QString uuid);

private:


    Ui::BleWidItem *ui;




};


#endif // BLEWIDITEM_H
