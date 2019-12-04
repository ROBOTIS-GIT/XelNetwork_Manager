#ifndef TAB_DXL_H
#define TAB_DXL_H

#include <QWidget>
#include "dynamixel_sdk.h"                                   // Uses Dynamixel SDK library

namespace Ui {
  class TabDxl;
}

class LogDebug;
class QTableWidgetItem;

class TabDxl : public QWidget
{
  Q_OBJECT

public:
  explicit TabDxl(QWidget *parent = nullptr);
  ~TabDxl();

  int      dxl_table_length;
  int      dxl_baud_index;
  int      dxl_protocol_index;
  bool     dxl_write_enable;
  QString  dxl_port_string;
  QString  dxl_id_string;
  LogDebug *log_debug;

  void setDxlPortString(QString port_string);
  void setDxlIdString(QString id_string);
  void setDxlBaudIdnex(int buad_index);
  void setDxlProtocolIdnex(int protocol_index);
  void drawControlTable(int model_num);

  QString getControlValueString(uint8_t *p_data, QString type_string);
  bool    getDxlDump(uint16_t addr, uint8_t *p_data, uint16_t len);
  bool    write(dynamixel::PortHandler *portHandler, dynamixel::PacketHandler *packetHandler, uint8_t id, uint16_t addr, uint16_t length, uint8_t *p_data);
  bool    dxlWrite(uint16_t addr, uint16_t length, uint8_t *p_data);

private slots:
  void on_buttonPing_clicked();
  void on_buttonPings_clicked();
  void on_buttonReadData_clicked();
  void on_buttonUpload_clicked();
  void on_buttonPortRefresh_clicked();
  //void on_lineEdit_port_textChanged(const QString &arg1);
  void on_lineEdit_id_textChanged(const QString &arg1);
  void on_comboBoxSerialPort_currentTextChanged(const QString &arg1);
  void on_comboBox_baudrate_currentIndexChanged(int index);
  void on_comboBoxProtocolVersion_currentIndexChanged(int index);
  void on_tableWidget_cellChanged(int row, int column);
  void on_tableWidget_ID_doubleClicked(const QModelIndex &index);
  void on_updateMsgType(int index);
  void on_updateBaudrate(int index);
  void on_updateMasterBaudrate(int index);
  void on_updateTopicMode(int index);
private:
  Ui::TabDxl *ui;
};

#endif // TAB_DXL_H
