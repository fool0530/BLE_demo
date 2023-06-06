#include "blewiditem.h"
#include "ui_blewiditem.h"

BleWidItem::BleWidItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BleWidItem)
{
    ui->setupUi(this);


    this->btn_Name = ui->btn_name;
    this->btn_Addr = ui->btn_Addr;
    this->btn_Rssi = ui->btn_Rssi;


    connect(ui->btn_name,SIGNAL(clicked()),this,SLOT(on_btnclick()));
    connect(btn_Addr,SIGNAL(clicked()),this,SLOT(on_btnclick()));
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
