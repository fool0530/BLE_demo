#include "blewiditem.h"
#include "ui_blewiditem.h"

BleWidItem::BleWidItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BleWidItem)
{
    ui->setupUi(this);


    this->btn_Name = ui->btn_name;
    this->Box_Uuid = ui->comboBox;
    this->btn_Rssi = ui->btn_Rssi;


    connect(ui->btn_name,SIGNAL(clicked()),this,SLOT(on_btnclick()));
    connect(Box_Uuid,SIGNAL(currentIndexChanged(int)),this,SLOT(on_BoxUidChanged(int)));
    connect(btn_Rssi,SIGNAL(clicked()),this,SLOT(on_btnclick()));
}

BleWidItem::~BleWidItem()
{  
    delete ui;
}

void BleWidItem::on_btnclick()
{
    emit emit_send(index);
}

void BleWidItem::on_BoxUidChanged(int index)
{
    Q_UNUSED(index)
    QString uuid = Box_Uuid->currentText();
    emit BoxUidChanged(uuid);
}
