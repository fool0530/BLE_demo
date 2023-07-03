#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<QMessageBox>
#include <QLowEnergyDescriptor>
#include <QByteArray>
#include <QBluetoothLocalDevice>
#include<QDateTime>


#include<windows.h>
#include <BluetoothAPIs.h>

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

    m_file = new QFile("output.txt");
    m_file->open(QIODevice::WriteOnly | QIODevice::Text); // 以只写文本模式打开文件
    m_stream = new QTextStream (m_file);

    ui->btn_Disconnect->setDisabled(true);
}

MainWindow::~MainWindow()
{
    if(m_file){
        m_file->close(); // 在析构函数中关闭文件
        delete m_file; // 释放内存
    }
    delete ui;
}

void MainWindow::on_btnSerchClicked()
{
    //DisconnectBleFromLocal();
    if(m_controler != nullptr){

         m_controler->disconnectFromDevice();

        delete m_controler;
        m_controler = nullptr;
        qDebug()<<"on_btnSerchClicked() -> m_controler->disconnectFromDevice();";
    }
    qDeleteAll(m_services);
    m_devices.clear();
    m_services.clear();
    Boxes.clear();

     m_service = nullptr;

    ui->listWidget->setDisabled(true);
    ui->btn_Search->setText("Searching...");
    ui->btn_Search->setStyleSheet("background-color: yellow;");

    int count = ui->listWidget->count();
    for(int i =count-1;0<=i;i--){
        if(i == 0)continue;
        ui->listWidget->removeItemWidget(ui->listWidget->item(i));
        delete ui->listWidget->item(i);
    }
    flagofLowE = ui->radiobtn__LowEngergy->isChecked();

    initBle();
    ui->btn_Search->setText("Searched");
    ui->btn_Search->setStyleSheet("background-color: #99ff99;");
    ui->listWidget->setDisabled(false);
}

void MainWindow::onScanFinished()
{
//    if (m_devicesNames.size() == 0)
//        emit statusInfoChanged("No Low Energy devices found", false);
}

void MainWindow::initUi()
{
    //设置设备列表的标题
    QListWidgetItem *item = new QListWidgetItem();
    //创建自定义的窗口 然后放入到listwidget中
    BleWidItem *bwi = new BleWidItem();

    //设置label的显示
    bwi->btn_Name->setText("名称");
    bwi->Box_Uuid->insertItem(0,"serverUuid");
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
    m_devices.clear();
    //创建设备发现对象
    m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);

    connect(m_discoveryAgent, SIGNAL(deviceDiscovered(const QBluetoothDeviceInfo&)),
            this, SLOT(addDevice(const QBluetoothDeviceInfo&)));
    connect(m_discoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)),
            this, SLOT(onDeviceScanError(QBluetoothDeviceDiscoveryAgent::Error)));

    // 查找设备
    qDebug() << "Searching for devices...";
    *m_stream << "Searching for devices...";
    m_discoveryAgent->start();


    qDebug() << "Found" << m_discoveryAgent->discoveredDevices().count() << "devices:" <<Qt::endl;
    *m_stream << "Searching for devices...";


}

void MainWindow::DisconnectBleFromLocal()
{
    qDebug() <<"uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu";

    qDebug() <<"uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu";
}

void MainWindow::onDeviceScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError)
        qDebug()<< ("The Bluetooth adaptor is powered off, power it on before doing discovery.");
    else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError)
        qDebug()<<("Writing or reading from the device resulted in an error.");
    else
        qDebug()<<("An unknown error has occurred.");
}

void MainWindow::addDevice(const QBluetoothDeviceInfo &device)
{
    //static int count = 0;
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        qWarning() << "Discovered LE Device name: " << device.name() << " Address: "
                   << device.address().toString();
        m_devices.append(device);
        //设置设备列表的标题
        QListWidgetItem *item = new QListWidgetItem();
        //创建自定义的窗口 然后放入到listwidget中
        BleWidItem *bwi = new BleWidItem();

        //设置label的显示
        //bwi->btn_Name->setText(device.address().toString());
        bwi->btn_Name->setText(device.name());
        Boxes.append(bwi->Box_Uuid);
        bwi->btn_Rssi->setText(tr("%1").arg(device.rssi()));
        bwi->index = m_devices.size()-1;
        qDebug()<<"**********************************************device"<<device.address();


        item->setSizeHint(QSize(ui->listWidget->width(),bwi->height()));
        ui->listWidget->addItem(item);
        ui->listWidget->setItemWidget(item,bwi);

        connect(bwi,SIGNAL(emit_send(int)),this,SLOT(Binding_device(int)));
        connect(bwi,SIGNAL(BoxUidChanged(QString)),this,SLOT(Retrieval_server(QString)));
    }
}

bool MainWindow::Binding_device(int index)
{
    qDebug()<<"Binding_device"<<index;

    if(Boxes.at(index)->count()>0){
        qDebug()<<Boxes.at(index)->count();

        Boxes.at(index)->clear();

      }

    qDebug()<<"Binding_device(int index)"<<index<<m_devices.at(index).name()<<Qt::endl;
    *m_stream <<"Binding_device(int index)"<<index<<m_devices.at(index).name()<<Qt::endl;
    //QBluetoothCharacteristic::properties()

        if(m_controler != nullptr){

            m_controler->disconnectFromDevice();

            delete m_controler;
            m_controler = nullptr;
            qDebug() << u8" if(m_controler){";
        }

        qDebug() << "m_devices.at(index)"<<index<<m_devices.size();
    if(m_devices.size()>index)
    {
        m_controler = new QLowEnergyController(m_devices.at(index), this);//QLowEnergyController::createCentral(m_devices.at(index),this);
    }else{
        qDebug()<<"没设备可连！";
        return false;
    }

    ui->label_BTDevName->setText(QString("%1").arg(m_devices.at(index).name()));

    ui->label_Dev_state->setText(("正在连接，请等待......"));

    connect(m_controler,&QLowEnergyController::connected,[=](){

        qDebug() << u8"连接成功";

        //搜索服务
        this->m_controler->discoverServices();

        ui->label_Dev_state->setText(("连接状态：连接成功，准备连接服务，请等待......"));
        ui->label_Dev_state->setStyleSheet("background-color: #99ff99;");
    });

    connect(m_controler,QOverload<QLowEnergyController::Error>::of(&QLowEnergyController::error),[=](QLowEnergyController::Error newError){
        ui->label_Dev_state->setText(("连接状态：连接失败"));

        //emit stateChanged("连接状态：连接失败");
    });

    connect(m_controler,&QLowEnergyController::serviceDiscovered, [=](QBluetoothUuid serviceUuid) {
        static int num =0;
        //保存所有服务ID

        Boxes.at(index)->insertItem(num++,serviceUuid.toString());

        //Retrieval_server(serviceUuid);
    });

//    connect(m_controler, &QLowEnergyController::discoveryFinished, this, [=](){
//        delete m_service;
//        m_service=0;
//    });

    connect(m_controler,&QLowEnergyController::disconnected, [=]() {
        //DisconnectBleFromLocal();
//        qDebug()<<"-------------"<<"connect(m_controler,&QLowEnergyController::disconnected, [=]() {"<<"-----------";
//        if(m_discoveryAgent){
//            delete m_discoveryAgent;
//        }

        if(m_devices.count()){
            //qDeleteAll(m_services);
            m_devices.clear();
        }
        if(m_service){
            delete m_service;
            m_service = nullptr;
        }
        if(m_controler != nullptr){

            m_controler->disconnectFromDevice();

            delete m_controler;
            m_controler = nullptr;
            ui->listWidget->setDisabled(true);
            qDebug()<<"connect(m_controler,&QLowEnergyController::disconnected, [=]() {";
        }
    });

    this->m_controler->connectToDevice();
    return true;
}

void MainWindow::Retrieval_server(QString uuid)
{

    qDebug() << QString("发现服务") << uuid;
    *m_stream <<"Binding_device(int index)"<<"find server"<<Qt::endl;
    QBluetoothUuid serviceUuid =  QBluetoothUuid(QUuid(uuid));

//    //保存所有服务 zzl
//    m_services.append(m_controler->createServiceObject(QBluetoothUuid(serviceUuid),this));


    if(m_service){
        delete m_service;
        m_service = nullptr;
    }
    //过滤服务
//    if(serviceUuid == QBluetoothUuid(QUuid("{0000fe40-cc7a-482a-984a-7f2ed5b3e58f}"))){
//         qDebug() << u8"过滤成功";
//         *m_stream <<"guolv success"<<Qt::endl;
//         m_service = m_controler->createServiceObject(QBluetoothUuid(serviceUuid),this);
//    }

    m_service = m_controler->createServiceObject(QBluetoothUuid(serviceUuid),this);

    //m_service = m_controler->createServiceObject(QBluetoothUuid(serviceUuid),this);

    if(m_service==nullptr)
    {

        qDebug() << QString("服务建立失败!");
        *m_stream <<"server Built On Failure!"<<Qt::endl;
        ui->label_Dev_state->setStyleSheet("background-color: #ff9999;");
        ui->label_Dev_state->setText(("连接状态：服务建立失败！请重试"));
        return;
    }
    qDebug() << "服务建立成功!";
    *m_stream <<"服务建立成功!"<<Qt::endl;
    ui->label_Dev_state->setText(("连接状态：服务建立成功！可以返回"));
    ui->label_Dev_state->setStyleSheet("background-color: #99ff99;");


    connect(this->m_service,&QLowEnergyService::stateChanged, this, [this]() {
        qDebug() << "service state change  *-*-*-*-*-*-*-*-*-*" << m_service->state();

            //list=this->m_service->characteristics();

         if(m_service->state() == QLowEnergyService::ServiceDiscovered) {//已发现服务详细信息
             //获取特征对象
             QList<QLowEnergyCharacteristic> QLowEnergyCharacteristiclist= this->m_service->characteristics();

             for(int i=0;i<QLowEnergyCharacteristiclist.count();i++)
             {

                 qDebug() <<Qt::endl<<"QLowEnergyCharacteristiclist.at(i).properties()"<< QLowEnergyCharacteristiclist.at(i).properties()<<"i"<<i;
                 qDebug() <<"QLowEnergyCharacteristiclist.at(i).name"<< QLowEnergyCharacteristiclist.at(i).name();
                 qDebug() <<"QLowEnergyCharacteristiclist.at(i).uuid"<< QLowEnergyCharacteristiclist.at(i).uuid();
                 qDebug() <<"QLowEnergyCharacteristiclist.at(i).value"<< QLowEnergyCharacteristiclist.at(i).value();
                 *m_stream <<"QLowEnergyCharacteristiclist.at(i).properties()"<< QLowEnergyCharacteristiclist.at(i).properties()<<"i"<<i<<Qt::endl;
                 *m_stream <<"QLowEnergyCharacteristiclist.at(i).name"<< QLowEnergyCharacteristiclist.at(i).name();
                 *m_stream <<"QLowEnergyCharacteristiclist.at(i).uuid"<< QLowEnergyCharacteristiclist.at(i).uuid().toString();
                 *m_stream <<"QLowEnergyCharacteristiclist.at(i).value"<< QLowEnergyCharacteristiclist.at(i).value();
                 for(int j =0;j<QLowEnergyCharacteristiclist.at(i).descriptors().size();j++){
                      qDebug() <<"*****"<<Qt::endl<<"QLowEnergyCharacteristic QLowEnergyCharacteristiclist "<<i<<"Characteristic propertie"<<j;
                      qDebug() <<"*****"<<"QLowEnergyCharacteristiclist.at(i).descriptors().at(j).name();"<<QLowEnergyCharacteristiclist.at(i).descriptors().at(j).name();
                      qDebug() <<"*****"<<"QLowEnergyCharacteristiclist.at(i).descriptors().at(j).type();"<<QLowEnergyCharacteristiclist.at(i).descriptors().at(j).type();
                      qDebug() <<"*****"<<"QLowEnergyCharacteristiclist.at(i).descriptors().at(j).uuid();"<<QLowEnergyCharacteristiclist.at(i).descriptors().at(j).uuid();
                      qDebug() <<"*****"<<"QLowEnergyCharacteristiclist.at(i).descriptors().at(j).value();"<<QLowEnergyCharacteristiclist.at(i).descriptors().at(j).value();
                      *m_stream <<"*****"<<Qt::endl<<"QLowEnergyCharacteristic QLowEnergyCharacteristiclist "<<i<<"Characteristic propertie"<<j;
                      *m_stream <<"*****"<<"QLowEnergyCharacteristiclist.at(i).descriptors().at(j).name();"<<QLowEnergyCharacteristiclist.at(i).descriptors().at(j).name();
                      *m_stream <<"*****"<<"QLowEnergyCharacteristiclist.at(i).descriptors().at(j).type();"<<QLowEnergyCharacteristiclist.at(i).descriptors().at(j).type();
                      *m_stream <<"*****"<<"QLowEnergyCharacteristiclist.at(i).descriptors().at(j).uuid();"<<QLowEnergyCharacteristiclist.at(i).descriptors().at(j).uuid().toString();
                      *m_stream <<"*****"<<"QLowEnergyCharacteristiclist.at(i).descriptors().at(j).value();"<<QLowEnergyCharacteristiclist.at(i).descriptors().at(j).value();

                 }

                 /*如果QLowEnergyCharacteristic对象有效，则返回true，否则返回false*/
                 if(QLowEnergyCharacteristiclist.at(i).isValid())
                 {
                     if((QLowEnergyCharacteristiclist.at(i).properties() & 0x10))//可写
                     {
                         qDebug() << "-------------";
                         *m_stream <<"-------------"<<Qt::endl;

                         //描述符定义特征如何由特定客户端配置。
                         m_notificationDesc = QLowEnergyCharacteristiclist.at(i).descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);

                         if(m_notificationDesc.isValid())
                         {
                             //写描述符
                             qDebug()<<"m_notificationDesc.value() 0100"<<m_notificationDesc.value();
                             qDebug()<<"m_notificationDesc.name() 0100"<<m_notificationDesc.name();
                             qDebug()<<"m_notificationDesc.uuid() 0100"<<m_notificationDesc.uuid();
                             qDebug()<<"m_notificationDesc.uuid() 0100"<<m_service->state();

                             static bool IS_JustOnce =true;
                             if(IS_JustOnce){
                                 m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));//FEE1  0100
                                 IS_JustOnce = false;
                             }

//                             m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));//FEE1  0100


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

    ui->btn_Disconnect->setDisabled(false);


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
        QDateTime currentTime = QDateTime::currentDateTime(); // 获取当前时间
        qint64 milliseconds = currentTime.toMSecsSinceEpoch(); // 获取当前时间的毫秒级表示
//        qDebug()<<"c.uuid();"<<c.uuid();
//        qDebug()<<( "蓝牙 发送消息 来了");
        qDebug()<<value.toHex();
//        qDebug() << "Current time:" << currentTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
//        qDebug() << "Milliseconds:" << milliseconds;
        static int datanum =0;
        ++datanum;
        qDebug()<<"包的数量 ::"<<datanum;
        *m_stream <<"c.uuid();"<<c.uuid().toString();
        *m_stream <<( "蓝牙 发送消息 来了");
        *m_stream <<value.toHex();
        *m_stream  << "Current time:" << currentTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
        *m_stream  << "Milliseconds:" << milliseconds;
        //ui->Edit_Recerive->insertPlainText(value.toHex()); //就将本次的字符数组暂存接收中
    });

    //service错误监听
    connect(m_service,QOverload<QLowEnergyService::ServiceError>::of(&QLowEnergyService::error),[=](QLowEnergyService::ServiceError newErrorr){
        ui->label_Dev_state->setStyleSheet("background-color: #ff9999;");
        if(QLowEnergyService::NoError == newErrorr)
        {
              qDebug() <<("没有发生错误。");
              *m_stream <<"没有发生错误。";
              ui->label_Dev_state->setText(("连接状态：没有发生错误"));
        }
        if(QLowEnergyService::OperationError==newErrorr)
        {
             qDebug() <<("错误: 当服务没有准备好时尝试进行操作!");
             *m_stream <<"错误: 当服务没有准备好时尝试进行操作!";
             ui->label_Dev_state->setText(("连接状态：错误:当服务没有准备好时尝试进行操作"));
        }
        if(QLowEnergyService::CharacteristicReadError==newErrorr)
        {
             qDebug() <<("尝试读取特征值失败!");
             *m_stream <<"错误: 尝试读取特征值失败!";
             ui->label_Dev_state->setText(("连接状态：尝试读取特征值失败"));
        }
        if(QLowEnergyService::CharacteristicWriteError==newErrorr)
        {
             qDebug() <<QString("尝试为特性写入新值失败!");
             *m_stream <<"错误: 尝试为特性写入新值失败!";
             ui->label_Dev_state->setText(("连接状态：尝试为特性写入新值失败"));
        }
        if(QLowEnergyService::DescriptorReadError==newErrorr)
        {
             qDebug() <<("尝试读取描述符值失败!");
             *m_stream <<"错误: 尝试读取描述符值失败!";
             ui->label_Dev_state->setText(QString::fromLocal8Bit("连接状态：尝试读取描述符值失败"));
        }
        if(QLowEnergyService::DescriptorWriteError==newErrorr)
        {
             qDebug() <<(" 尝试向描述符写入新值失败!");
             *m_stream <<"错误: 尝试向描述符写入新值失败!";
             ui->label_Dev_state->setText(("连接状态：尝试向描述符写入新值失败"));
        }
        if(QLowEnergyService::UnknownError==newErrorr)
        {
             qDebug() <<("与服务交互时发生未知错误!");
             *m_stream <<"错误: 与服务交互时发生未知错误!";
             ui->label_Dev_state->setText(("连接状态：与服务交互时发生未知错误！请重新连接"));
        }
         ui->label_Dev_state->setText(QString("m_service 连接状态：连接中断 error type %1").arg(newErrorr));
    });

    //没有写响应属性的Characteristic不会进入这个连接
    connect(m_service,&QLowEnergyService::characteristicWritten,[=](QLowEnergyCharacteristic c,QByteArray value) {
        ui->label_Dev_state->setStyleSheet("background-color: #99ff99;");
        QDateTime currentTime = QDateTime::currentDateTime(); // 获取当前时间
        qint64 milliseconds = currentTime.toMSecsSinceEpoch(); // 获取当前时间的毫秒级表示

        qDebug() << "characteristicChanged state change::";
        qDebug() << "value length::" << value.length();
        qDebug() << "value ::" << value;
        qDebug() << "time ::" << milliseconds;
    });
}

void MainWindow::SendDataToBLE(QByteArray bytearray)
{
//     qDebug()<<QString::fromLocal8Bit( "状态栏状态")<<ui->label_Dev_state->text()
//            <<QString::fromLocal8Bit( "是否相等")<< (ui->label_Dev_state->text() == QString::fromLocal8Bit("连接状态：已连接"));
    if(ui->label_Dev_state->text() != ("连接状态：已连接"))
    {
        QMessageBox::warning(this,("警告"),("蓝牙还未连接，请先连接后再进行操作！"));
        return ;
    }



    //将字符串拆分为每20个字符发送一次，超过后会发送失败
    for(int i=0;i<bytearray.length()/20+1;i++)
    {
        QByteArray s;
        if((i+1)*20 <= bytearray.length())
        {
            s = bytearray.mid(i*20,20);
        }
        else
        {
            s = bytearray.mid(i*20,-1);
        }
        if(!m_service){
            return ;
        }


        if(m_characteristic.properties()& QLowEnergyCharacteristic::WriteNoResponse){

            m_service->writeCharacteristic(m_characteristic,s ,QLowEnergyService::WriteMode::WriteWithoutResponse);
            qDebug() << "发送信息::"<<s;

        }
        else{
            m_service->writeCharacteristic(m_characteristic,s ,QLowEnergyService::WriteMode::WriteWithResponse);
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
    if(ui->Edit_Send->toPlainText() != nullptr){
        //qDebug() << "m_service->state()"<<m_service->state();
        SendDataToBLE(QByteArray::fromHex( ui->Edit_Send->toPlainText().toUtf8()));
    }

}


void MainWindow::on_btn_Disconnect_clicked()
{


    //localDevice.setHostMode(QBluetoothLocalDevice::HostPoweredOff);
    //DisconnectBleFromLocal();

//    m_discoveryAgent->stop();
//    if (m_notificationDesc.isValid() && m_service
//            && m_notificationDesc.value() == QByteArray::fromHex("0100")) {
//        m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0000"));
//        qDebug()<<"m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex(\"0000\"));";
//    }


    if(m_devices.count()){
        m_devices.clear();
    }
    if(m_service){
        qDebug()<<"m_service->disconnect();"<<m_service->disconnect();
        //m_service->disconnect();

        m_service->deleteLater();

    }
    //stopNotifications();
    if(m_controler != nullptr){

        m_controler->disconnectFromDevice();
//        m_controler->disconnect();
//        m_controler->deleteLater();

//        delete m_controler;
//        m_controler = nullptr;
        ui->listWidget->setDisabled(true);
    }


    if(m_discoveryAgent != 0){
        //m_discoveryAgent->deleteLater();
        //m_discoveryAgent->disconnect();

//        delete m_discoveryAgent;
//        m_discoveryAgent = nullptr;
    }


}

