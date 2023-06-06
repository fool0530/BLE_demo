#ifndef BLEWIDITEM_H
#define BLEWIDITEM_H

#include<QDebug>

#include <QWidget>
#include<QPushButton>

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
    QPushButton *btn_Addr;
    QPushButton *btn_Rssi;

    int index = 0;

private slots:
    void on_btnclick();


signals:
    void emit_send(int);

private:


    Ui::BleWidItem *ui;




};


#endif // BLEWIDITEM_H
