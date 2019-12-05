/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "log_debug.h"
#include "settingsdialog.h"
#include "tab_xel.h"
#include "tab_terminal.h"

#include <QMessageBox>
#include <QLabel>
#include <QtSerialPort/QSerialPort>
#include <QScrollBar>
#include <QPushButton>
#include <QStandardItemModel>
#include <QSettings>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  setCentralWidget(ui->tabWidget);

  //console = new Console;
  //console->setEnabled(false);
  //ui->gridLayout->addWidget(console);

  tab_dxl = new TabDxl;
  QHBoxLayout *layout = new QHBoxLayout;
  layout->addWidget(tab_dxl);
  ui->tab_xel->setLayout(layout);

  tab_terminal = new TabTerminal;
  QHBoxLayout *layout_terminal = new QHBoxLayout;
  layout_terminal->addWidget(tab_terminal);
  ui->tab_terminal->setLayout(layout_terminal);

  serial = new QSerialPort(this);
  settings = new SettingsDialog;

  ui->actionConnect->setEnabled(true);
  ui->actionDisconnect->setEnabled(false);
  ui->actionQuit->setEnabled(true);
  ui->actionConfigure->setEnabled(true);

  status = new QLabel;
  ui->statusBar->addWidget(status);

  initActionsConnections();

  connect(serial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error), this, &MainWindow::handleError);
  connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);
  connect(tab_terminal->console, &Console::getData, this, &MainWindow::writeData);

  scrollWidth = 1;
  plotEnable = 0;

  QStandardItemModel *model = new QStandardItemModel();
  QStandardItem *Item = new QStandardItem();
  Item->setText("baud 3000000");
  Item->setCheckable( true );
  Item->setCheckState( Qt::Checked );
  model->setItem( 0, Item );

  m_sSettingsFile = QApplication::applicationDirPath() + "/settings.ini";
  tab_dxl->log_debug->printString(QApplication::applicationDirPath() + "/settings.txt");

  loadSettings();
}


MainWindow::~MainWindow()
{
  saveSettings();

  delete settings;
  delete ui;
}


void MainWindow::openSerialPort()
{
  SettingsDialog::Settings p = settings->settings();
  serial->setPortName(p.name);
  serial->setBaudRate(p.baudRate);
  serial->setDataBits(p.dataBits);
  serial->setParity(p.parity);
  serial->setStopBits(p.stopBits);
  serial->setFlowControl(p.flowControl);
  if (serial->open(QIODevice::ReadWrite))
  {
    tab_terminal->console->setEnabled(true);
    tab_terminal->console->setLocalEchoEnabled(p.localEchoEnabled);
    ui->actionConnect->setEnabled(false);
    ui->actionDisconnect->setEnabled(true);
    ui->actionConfigure->setEnabled(false);
    showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                      .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                      .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
  }
  else
  {
    QMessageBox::critical(this, tr("Error"), serial->errorString());
    showStatusMessage(tr("Open error"));
  }
}


void MainWindow::closeSerialPort()
{
  if (serial->isOpen())
  {
    serial->close();
  }
  tab_terminal->console->setEnabled(false);
  ui->actionConnect->setEnabled(true);
  ui->actionDisconnect->setEnabled(false);
  ui->actionConfigure->setEnabled(true);
  showStatusMessage(tr("Disconnected"));
}


void MainWindow::about()
{
  QMessageBox::about(this, tr("About XEL Manager"),
                     tr("Tool for accessing XEL's control table"
                        " "));
}


void MainWindow::writeData(const QByteArray &data)
{
  if(serial->isOpen())
  {
    serial->write(data);
  }
}

void MainWindow::readData()
{
  if( plotEnable == 0 )
  {
    QByteArray data = serial->readAll();
    tab_terminal->console->putData(data);
  }
  else
  {
    while(serial->canReadLine())
    {
       QByteArray data = serial->readLine();
       tab_terminal->console->putData(data);
    }
  }
}


void MainWindow::handleError(QSerialPort::SerialPortError error)
{
  if (error == QSerialPort::ResourceError) {
      QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
      closeSerialPort();
  }
}


void MainWindow::initActionsConnections()
{
  connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
  connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
  connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
  connect(ui->actionConfigure, &QAction::triggered, settings, &MainWindow::show);
  connect(ui->actionClear, &QAction::triggered, tab_terminal->console, &Console::clear);
  connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
  connect(ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
}


void MainWindow::showStatusMessage(const QString &message)
{
  status->setText(message);
}


void MainWindow::closeEvent (QCloseEvent *event)
{
  QMessageBox::StandardButton resBtn = QMessageBox::question( this, "XEL Manager",
                                                              tr("Are you sure?\n"),
                                                              QMessageBox::No | QMessageBox::Yes,
                                                              QMessageBox::Yes);
  if (resBtn != QMessageBox::Yes)
  {
      event->ignore();
  }
  else
  {
    saveSettings();
    event->accept();
  }
}


void MainWindow::loadSettings()
{
  //QSettings cmdSettings(m_sSettingsFile, QSettings::NativeFormat);
  QSettings cmdSettings(m_sSettingsFile, QSettings::IniFormat); //QSettings::NativeFormat);
  QString sText = cmdSettings.value("linecmd").toString();
  SettingsDialog::Settings p = settings->settings();

  p.name = cmdSettings.value("port_name").toString();
  p.baudRate = cmdSettings.value("port_baudRate").toInt();
  p.stringBaudRate = cmdSettings.value("port_stringBaudRate").toString();
  p.localEchoEnabled = cmdSettings.value("port_localEchoEnabled").toInt();

  tab_dxl->setDxlPortString(cmdSettings.value("dxl_port").toString());
  tab_dxl->setDxlIdString(cmdSettings.value("dxl_id").toString());
  tab_dxl->setDxlBaudIdnex(cmdSettings.value("dxl_baud").toInt());

  settings->setSettings(p);

  qDebug() << cmdSettings.fileName();
  qDebug() << QApplication::applicationDirPath();
}


void MainWindow::saveSettings()
{
  QSettings cmdSettings(m_sSettingsFile, QSettings::IniFormat); //QSettings::NativeFormat);
  SettingsDialog::Settings p = settings->settings();

  cmdSettings.setValue("port_name", p.name);
  cmdSettings.setValue("port_baudRate", p.baudRate);
  cmdSettings.setValue("port_stringBaudRate",p.stringBaudRate);
  cmdSettings.setValue("port_localEchoEnabled", p.localEchoEnabled);
  cmdSettings.setValue("dxl_port", tab_dxl->dxl_port_string);
  cmdSettings.setValue("dxl_id",   tab_dxl->dxl_id_string);
  cmdSettings.setValue("dxl_baud", tab_dxl->dxl_baud_index);
}
