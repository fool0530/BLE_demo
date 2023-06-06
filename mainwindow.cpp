#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<QMessageBox>
#include <QLowEnergyDescriptor>
#include <QByteArray>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //初始化界面
    initUi();

    //查找按钮绑定
    connect(ui->btn_Search,&QPushButton::clicked,this,&MainWindow::on_btnSerchClicked);

    connect(ui->btn_Send,&QPushButton::clicked,this,&MainWindow::on_btnSendClicked);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnSerchClicked()
{
    m_devices.clear();
    m_services.clear();
    ui->listWidget->setDisabled(true);
    ui->btn_Search->setText("Searching...");
    ui->btn_Search->setStyleSheet("background-color: yellow;");

    int count = ui->listWidget->count();
    for(int i =count-1;0<=i;i--){
        if(i == 0)continue;
        ui->listWidget->removeItemWidget(ui->listWidget->item(i));
        delete ui->listWidget->item(i);
        //ui->listWidget->clear();
    }
    flagofLowE = ui->radiobtn__LowEngergy->isChecked();

    initBle();
    ui->btn_Search->setText("Searched");
    ui->btn_Search->setStyleSheet("background-color: #99ff99;");
    ui->listWidget->setDisabled(false);
}

void MainWindow::initUi()
{
    //设置设备列表的标题
    QListWidgetItem *item = new QListWidgetItem();
    //创建自定义的窗口 然后放入到listwidget中
    BleWidItem *bwi = new BleWidItem();

    //设置label的显示
    bwi->btn_Name->setText("名称");
    bwi->btn_Addr->setText("地址");
    bwi->btn_Rssi->setText("信号强度");

    item->setSizeHint(QSize(ui->listWidget->width(),bwi->height()));
    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item,bwi);

    //设置flag
    flagofLowE = ui->radiobtn__LowEngergy->isChecked();
    ui->radiobtn__LowEngergy->setCheckable(true);
    ui->radiobtn__LowEngergy->setAutoExclusive(false);
}

void MainWindow::initBle()
{
    //创建设备发现对象
    m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    m_discoveryAgent->setLowEnergyDiscoveryTimeout(5000);

    // 查找设备
    qDebug() << "Searching for devices...";
    m_discoveryAgent->start();

    while (m_discoveryAgent->isActive())//发现蓝牙设备 true false
    {
        //根据指定的标志处理调用线程的所有挂起事件，直到没有其他事件要处理为止。
        QCoreApplication::processEvents();
    }


    qDebug() << "Found" << m_discoveryAgent->discoveredDevices().count() << "devices:" <<Qt::endl;
    int count = 0;
    foreach (const QBluetoothDeviceInfo &device, m_discoveryAgent->discoveredDevices())
    {

//        if(true == flagofLowE)//低功耗
//        {
//            qDebug() << "if(true == flagofLowE)//低功耗";
//            if(device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
//            {
                m_devices.append(device);

                //设置设备列表的标题
                QListWidgetItem *item = new QListWidgetItem();
                //创建自定义的窗口 然后放入到listwidget中
                BleWidItem *bwi = new BleWidItem();

                //设置label的显示
                bwi->btn_Name->setText(device.name());
                bwi->btn_Addr->setText(device.address().toString());
                bwi->btn_Rssi->setText(tr("%1").arg(device.rssi()));
                bwi->index = count++;


                item->setSizeHint(QSize(ui->listWidget->width(),bwi->height()));
                ui->listWidget->addItem(item);
                ui->listWidget->setItemWidget(item,bwi);

                connect(bwi,SIGNAL(emit_send(int)),this,SLOT(Binding_device(int)));
//            }
//        }else{

//            continue;
//        }
//        continue;
    }

}

bool MainWindow::Binding_device(int index)
{

    qDebug()<<"Binding_device(int index)"<<index<<m_devices.at(index).name()<<Qt::endl;
    //QBluetoothCharacteristic::properties()

    m_controler = QLowEnergyController::createCentral(m_devices.at(index),this);

    ui->label_BTDevName->setText(QString::fromLocal8Bit("%1").arg(m_devices.at(index).name()));

    ui->label_Dev_state->setText(QString::fromLocal8Bit("正在连接，请等待......"));

    connect(m_controler,&QLowEnergyController::connected,[=](){

        qDebug() << u8"连接成功";

        //搜索服务
        this->m_controler->discoverServices();
        ui->label_Dev_state->setText(QString::fromLocal8Bit("连接状态：连接成功，准备连接服务，请等待......"));
    });

    connect(m_controler,QOverload<QLowEnergyController::Error>::of(&QLowEnergyController::error),[=](QLowEnergyController::Error newError){
        ui->label_Dev_state->setText(QString::fromLocal8Bit("连接状态：连接失败"));

        //emit stateChanged("连接状态：连接失败");
    });

    connect(m_controler,&QLowEnergyController::serviceDiscovered, [=](QBluetoothUuid serviceUuid) {


        Retrieval_server(serviceUuid);
    });

    this->m_controler->connectToDevice();
    return true;
}

void MainWindow::Retrieval_server(const QBluetoothUuid &serviceUuid)
{
    qDebug() << u8"发现服务" << serviceUuid;

    //保存所有服务 zzl
    m_services.append(m_controler->createServiceObject(QBluetoothUuid(serviceUuid),this));

    //过滤服务
//    if(serviceUuid == QBluetoothUuid(QUuid("{00000000-cc7a-482a-984a-7f2ed5b3e58f}"))){
//         qDebug() << u8"过滤成功";
//         m_service = m_controler->createServiceObject(QBluetoothUuid(serviceUuid),this);
//    }
    m_service = m_controler->createServiceObject(QBluetoothUuid(serviceUuid),this);

    if(m_service==NULL)
    {
        qDebug() << u8"服务建立失败!";
        ui->label_Dev_state->setText(QString::fromLocal8Bit("连接状态：服务建立失败！请重试"));
        return;
    }
    qDebug() << u8"服务建立成功!";
    ui->label_Dev_state->setText(QString::fromLocal8Bit("连接状态：服务建立成功！可以返回"));


    connect(this->m_service,&QLowEnergyService::stateChanged, this, [this]() {
        qDebug() << "service state change" << m_service->state();

            list=this->m_service->characteristics();

         if(m_service->state() == QLowEnergyService::ServiceDiscovered) {//已发现服务详细信息
             //获取特征对象
             QList<QLowEnergyCharacteristic> QLowEnergyCharacteristiclist= this->m_service->characteristics();

             for(int i=0;i<QLowEnergyCharacteristiclist.count();i++)
             {

                 qDebug() <<Qt::endl<<"QLowEnergyCharacteristiclist.at(i).properties()"<< QLowEnergyCharacteristiclist.at(i).properties()<<"i"<<i;
                 qDebug() <<"QLowEnergyCharacteristiclist.at(i).name"<< QLowEnergyCharacteristiclist.at(i).name();
                 qDebug() <<"QLowEnergyCharacteristiclist.at(i).uuid"<< QLowEnergyCharacteristiclist.at(i).uuid();
                 qDebug() <<"QLowEnergyCharacteristiclist.at(i).value"<< QLowEnergyCharacteristiclist.at(i).value();
                 for(int j =0;j<QLowEnergyCharacteristiclist.at(i).descriptors().size();j++){
                      qDebug() <<"*****"<<Qt::endl<<"QLowEnergyCharacteristic QLowEnergyCharacteristiclist "<<i<<"Characteristic propertie"<<j;
                      qDebug() <<"*****"<<"QLowEnergyCharacteristiclist.at(i).descriptors().at(j).name();"<<QLowEnergyCharacteristiclist.at(i).descriptors().at(j).name();
                      qDebug() <<"*****"<<"QLowEnergyCharacteristiclist.at(i).descriptors().at(j).type();"<<QLowEnergyCharacteristiclist.at(i).descriptors().at(j).type();
                      qDebug() <<"*****"<<"QLowEnergyCharacteristiclist.at(i).descriptors().at(j).uuid();"<<QLowEnergyCharacteristiclist.at(i).descriptors().at(j).uuid();
                      qDebug() <<"*****"<<"QLowEnergyCharacteristiclist.at(i).descriptors().at(j).value();"<<QLowEnergyCharacteristiclist.at(i).descriptors().at(j).value();

                 }

                 /*如果QLowEnergyCharacteristic对象有效，则返回true，否则返回false*/
                 if(QLowEnergyCharacteristiclist.at(i).isValid())
                 {
                     if((QLowEnergyCharacteristiclist.at(i).properties() & 0x10))//可写
                     {
                         qDebug() << "-------------";

                         //描述符定义特征如何由特定客户端配置。
                         QLowEnergyDescriptor m_notificationDesc = QLowEnergyCharacteristiclist.at(i).descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
                         if(m_notificationDesc.isValid())
                         {
                             //写描述符
                             m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));
                         }
                     }

                     if((QLowEnergyCharacteristiclist.at(i).properties() & QLowEnergyCharacteristic::WriteNoResponse)
                             || (QLowEnergyCharacteristiclist.at(i).properties() & QLowEnergyCharacteristic::Write))//可写
                     {
                         m_characteristic = QLowEnergyCharacteristiclist.at(i);
                         qDebug() << "m_characteristic.uuid"<<m_characteristic.uuid()<<"name"<<m_characteristic.name();
                         if(QLowEnergyCharacteristiclist.at(i).properties() & QLowEnergyCharacteristic::WriteNoResponse)
                             m_writeMode = QLowEnergyService::WriteWithoutResponse;
                         else
                             m_writeMode = QLowEnergyService::WriteWithResponse;
                     }
                 }


             }

         }

    });


    ConnectCharacteristic(m_service);

    // 触发服务详情发现函数 ，不要忘记调用
    //启动服务包含的服务、特征和描述符的发现。发现过程是通过stateChanged()信号指示的。
    m_service->discoverDetails();


}

void MainWindow::ConnectCharacteristic(QLowEnergyService *m_service)
{
    // 通过监听特征对象的变化
    //写的和读取到的都会进入到这个位置
    ui->label_Dev_state->setText(("连接状态：已连接"));
    ui->label_Dev_state->setStyleSheet("background-color: #99FF99;");

    connect(m_service,&QLowEnergyService::characteristicChanged,[=](QLowEnergyCharacteristic c,QByteArray value) {
        qDebug()<<"c.uuid();"<<c.uuid();
        qDebug()<<( "蓝牙 发送消息 来了");
        ui->Edit_Recerive->insertPlainText(value.toHex()); //就将本次的字符数组暂存接收中
    });

    //service错误监听
    connect(m_service,QOverload<QLowEnergyService::ServiceError>::of(&QLowEnergyService::error),[=](QLowEnergyService::ServiceError newErrorr){
        ui->label_Dev_state->setStyleSheet("background-color: #ff9999;");
        if(QLowEnergyService::NoError == newErrorr)
        {
              qDebug() <<("没有发生错误。");
              ui->label_Dev_state->setText(("连接状态：没有发生错误"));
        }
        if(QLowEnergyService::OperationError==newErrorr)
        {
             qDebug() <<("错误: 当服务没有准备好时尝试进行操作!");
             ui->label_Dev_state->setText(("连接状态：错误:当服务没有准备好时尝试进行操作"));
        }
        if(QLowEnergyService::CharacteristicReadError==newErrorr)
        {
             qDebug() <<("尝试读取特征值失败!");
             ui->label_Dev_state->setText(("连接状态：尝试读取特征值失败"));
        }
        if(QLowEnergyService::CharacteristicWriteError==newErrorr)
        {
             qDebug() <<("尝试为特性写入新值失败!");
             ui->label_Dev_state->setText(("连接状态：尝试为特性写入新值失败"));
        }
        if(QLowEnergyService::DescriptorReadError==newErrorr)
        {
             qDebug() <<("尝试读取描述符值失败!");
             ui->label_Dev_state->setText(("连接状态：尝试读取描述符值失败"));
        }
        if(QLowEnergyService::DescriptorWriteError==newErrorr)
        {
             qDebug() <<(" 尝试向描述符写入新值失败!");
             ui->label_Dev_state->setText(("连接状态：尝试向描述符写入新值失败"));
        }
        if(QLowEnergyService::UnknownError==newErrorr)
        {
             qDebug() <<("与服务交互时发生未知错误!");
             ui->label_Dev_state->setText(("连接状态：与服务交互时发生未知错误！请重新连接"));
        }
         ui->label_Dev_state->setText(QString::fromLocal8Bit("m_service 连接状态：连接中断 error type %1").arg(newErrorr));
    });

    //没有写响应属性的Characteristic不会进入这个连接
    connect(m_service,&QLowEnergyService::characteristicWritten,[=](QLowEnergyCharacteristic c,QByteArray value) {
        ui->label_Dev_state->setStyleSheet("background-color: #99ff99;");

        qDebug() << "characteristicChanged state change::";
        qDebug() << "value length::" << value.length();
        qDebug() << "value ::" << value;
    });
}

void MainWindow::SendDataToBLE(QByteArray bytearray)
{
//     qDebug()<<QString::fromLocal8Bit( "状态栏状态")<<ui->label_Dev_state->text()
//            <<QString::fromLocal8Bit( "是否相等")<< (ui->label_Dev_state->text() == QString::fromLocal8Bit("连接状态：已连接"));
//    if(ui->label_Dev_state->text() != QString::fromLocal8Bit("连接状态：已连接"))
//    {
//        QMessageBox::warning(this,("警告"),("蓝牙还未连接，请先连接后再进行操作！"));
//        return ;
//    }

    //将字符串拆分为每20个字符发送一次，超过后会发送失败
    for(int i=0;i<bytearray.length()/20+1;i++)
    {
        QString s = "";
        if((i+1)*20 <= bytearray.length())
        {
            s = bytearray.mid(i*20,20);
        }
        else
        {
            s = bytearray.mid(i*20,-1);
        }


//        //以WriteWithoutResponse这种模式发送的数据  长度不能超过20bytes，也就是20个字符

        //没用
//        QString hexString = "0000"; // 16进制字符串，不包含前缀0x
//        QByteArray temp = QByteArray::fromHex(hexString.toUtf8());
//        m_service->writeCharacteristic(m_characteristic,temp ,QLowEnergyService::WriteMode::WriteWithoutResponse);
//        qDebug() << "QByteArray temp = QByteArray::fromHex(hexString.toUtf8())"<<temp;
         //没用
         //m_service->writeCharacteristic(m_characteristic, "0x0001", QLowEnergyService::WriteWithoutResponse);

//        QByteArray temp;
//        temp.append(char(0x00));
//        temp.append(char(0x01));
//        m_service->writeCharacteristic(m_characteristic,temp ,QLowEnergyService::WriteMode::WriteWithoutResponse);
//        qDebug() << "QByteArray temp;"<<temp;

//        if(ui->radiobtn__LowEngergy->isChecked())
//        {
//            m_service->writeCharacteristic(m_characteristic, "0x0001", QLowEnergyService::WriteWithoutResponse);
//        }
//        else{
//            m_service->writeCharacteristic(m_characteristic, "0x0000", QLowEnergyService::WriteWithoutResponse);
//        }


        if(m_characteristic.properties()& QLowEnergyCharacteristic::WriteNoResponse){
            m_service->writeCharacteristic(m_characteristic,s.toUtf8() ,QLowEnergyService::WriteMode::WriteWithoutResponse);

        }
        else{
            m_service->writeCharacteristic(m_characteristic,s.toUtf8() ,QLowEnergyService::WriteMode::WriteWithResponse);
            QEventLoop loop;
            connect(m_service, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)),
                    &loop, SLOT(quit()));
            loop.exec();
            qDebug() << "m_characteristic.uuid"<<m_characteristic.uuid()<<"name"<<m_characteristic.name();
        }
    }

}

void MainWindow::on_btnSendClicked()
{
    SendDataToBLE(QByteArray::fromHex( ui->Edit_Send->toPlainText().toUtf8()));

}


