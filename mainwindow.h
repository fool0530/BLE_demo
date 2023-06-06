#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include"blewiditem.h"

//#include<QList>

#include<QtBluetooth/QBluetoothLocalDevice>
#include<QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QBluetoothSocket>
#include <QBluetoothDeviceInfo>
#include <QBluetoothAddress>
#include <QBluetoothHostInfo>
#include <QLowEnergyController>
#include <QBluetoothSocket>
#include <QBluetoothServer>
//#include <QLowEnergyCharacteristic>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initUi();
    void initBle();


signals:
    void toConectCharacteristic(QLowEnergyService *);


private slots:
    bool Binding_device(int index);/*连接设备*/
    void Retrieval_server(const QBluetoothUuid &serviceUuid);/*连接服务*/

    void ConnectCharacteristic(QLowEnergyService * m_service); /* 接收蓝牙消息的函数 */

    void SendDataToBLE(QByteArray bytearray);

    void on_btnSendClicked();
    void on_btnSerchClicked();

private:
    Ui::MainWindow *ui;

    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent = nullptr;
    QList<QBluetoothDeviceInfo> m_devices;
    QLowEnergyController *m_controler= nullptr;
    QList<QLowEnergyService *> m_services;
    QLowEnergyService *m_service;
    QBluetoothUuid m_serviceUUID;
    QList<QLowEnergyCharacteristic> list;
    QLowEnergyCharacteristic m_characteristic;
    QLowEnergyService::WriteMode m_writeMode;
    QByteArray bytearray = NULL;

public:
    bool flagofLowE = false;
};
#endif // MAINWINDOW_H
