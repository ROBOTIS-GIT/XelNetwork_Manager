#include "tab_xel.h"
#include "ui_tab_xel.h"
#include "log_debug.h"
#include <stdio.h>
#include <vector>
#include <QThread>
#include <QtWidgets>
#include <QSerialPortInfo>

#include "qtcsv/reader.h"
#include "qtcsv/stringdata.h"
#include "qtcsv/variantdata.h"


#define XEL_BAUDRATE_ROW    4
#define XEL_BAUDRATE_COL    6

#define MASTER_XEL_BAUDRATE_ROW    9
#define MASTER_XEL_BAUDRATE_COL    6

#define XEL_MSG_TYPE_ROW    7
#define XEL_MSG_TYPE_COL    6

#define XEL_TOPIC_MODE_ROW  8
#define XEL_TOPIC_MODE_COL  6


// Protocol version
#define PROTOCOL_VERSION                2.0                 // See which protocol version is used in the Dynamixel

// Default setting
#define DXL_ID                          11                   // Dynamixel ID: 1
#define BAUDRATE                        1000000
#define DEVICENAME                      "/dev/ttyACM0"      // Check which port is being used on your controller


typedef union
{
  int8_t   s8;
  uint8_t  u8;
  int16_t  s16;
  uint16_t u16;
  int32_t  s32;
  uint32_t u32;
  float    f32;
} DxlControlValue;


typedef struct
{
  QString area;
  QString addr;
  QString name;
  QString size;
  QString type;
  QString acc;
} DxlControlTable;

enum XEL_MODEL_NUMBER
{
  SENSOR_XEL = 0x5040, //20544
  POWER_XEL = 0x5041, //20545
  COMMXEL_W = 0x5042, //20546
  COMM_XEL = 0x5043 //20547
};

static DxlControlTable CtSensorXelDefault[] =
{
  {"CON", "0",    "Model_Number",          "2",    "uint16_t",    "R" },
  {"CON", "2",    "Model_Info",            "4",    "uint32_t",    "R" },
  {"CON", "6",    "Firmware_Version",      "1",    "uint8_t",     "R" },
  {"RAM", "7",    "ID",                    "1",    "uint8_t",     "RW"},
  {"RAM", "8",    "Baud",                  "1",    "uint8_t",     "RW"},
  {"RAM", "9",    "Protocol ver",          "1",    "uint8_t",     "RW"},

  {"CON", "10",   "Topic name",            "16",   "str_t",       "R" },
  {"CON", "26",   "Topic type",            "1",    "uint8_t",     "R" },
  {"CON", "27",   "Topic mode",            "1",    "uint8_t",     "R" },
  {"CON", "28",   "Interval(ms) to pub",   "2",    "uint16_t",    "R" },
  {"CON", "30",   "Data address",          "2",    "uint16_t",    "R" },
  {"CON", "32",   "Next item address",     "2",    "uint16_t",    "R" },

  {"",    "",     "",                      "",     "",            ""  }
};

static DxlControlTable CtPowerXel[] =
{
  {"CON", "0",    "Model_Number",          "2",    "uint16_t",    "R" },
  {"CON", "2",    "Model_Info",            "4",    "uint32_t",    "R" },
  {"CON", "6",    "Firmware_Version",      "1",    "uint8_t",     "R" },
  {"EEP", "7",    "ID",                    "1",    "uint8_t",     "RW"},
  {"EEP", "8",    "Baud",                  "1",    "uint8_t",     "RW"},

  {"EEP", "32",   "XEL_DATA_TYPE",         "1",    "uint8_t",     "R" },
  {"EEP", "33",   "XEL_DATA_HZ",           "4",    "uint32_t",    "RW"},
  {"EEP", "37",   "XEL_DATA_NAME",         "32",   "str_t",       "RW"},
  {"EEP", "69",   "XEL_DATA_DIRECTION",    "1",    "uint8_t",     "R" },
  {"RAM", "70",   "XEL_DATA_ADDR",         "2",    "uint16_t",    "R" },
  {"RAM", "72",   "XEL_DATA_LENGTH",       "1",    "uint8_t",     "R" },
  {"RAM", "128",  "XEL_DATA",              "4",    "uint32_t",    "R" },

  {"",    "",     "",                      "",     "",            ""  }
};

static DxlControlTable CtCommXel[] =
{
  {"CON", "0",    "Model_Number",          "2",    "uint16_t",    "R" },
  {"CON", "2",    "Model_Info",            "4",    "uint32_t",    "R" },
  {"CON", "6",    "Firmware_Version",      "1",    "uint8_t",     "R" },
  {"EEP", "7",    "ID",                    "1",    "uint8_t",     "RW"},
  {"EEP", "8",    "Baud",                  "1",    "uint8_t",     "RW"},

  {"EEP", "9",    "Ethernet MAC address",  "6",    "mac_t",       "RW"},
  {"EEP", "15",   "Ethernet DHCP enable",  "1",    "uint8_t",     "RW"},
  {"EEP", "80",   "Ethernet remote IP",    "16",   "str_t",       "RW"},
  {"EEP", "96",   "Ethernet remote port",  "2",    "uint16_t",    "RW"},
  {"RAM", "98",   "Ethernet assigned IP",  "16",   "str_t",       "R" },
  {"EEP", "128",  "Node name",             "30",   "str_t",       "RW"},

  {"",    "",     "",                      "",     "",            ""  }
};

static DxlControlTable CtCommXel_W[] =
{
  {"CON", "0",    "Model Number",              "2",    "uint16_t",    "R" },
  {"CON", "2",    "Model Info",                "4",    "uint32_t",    "R" },
  {"CON", "6",    "Firmware Version",          "1",    "uint8_t",     "R" },
  {"CON", "7",    "ID",                        "1",    "uint8_t",     "R" },
  {"EEP", "8",    "Baudrate",                  "1",    "uint8_t",     "RW"},

  {"EEP", "10",   "ROS2 node name",            "32",   "str_t",       "RW"},
  {"EEP", "50",   "Auto scan start ID",        "1",    "uint8_t",     "RW"},
  {"EEP", "51",   "Auto scan end ID",          "1",    "uint8_t",     "RW"},
  {"EEP", "52",   "Auto scan interval(ms)",    "4",    "uint32_t",    "RW"},

  {"EEP", "58",   "DXL Master baudrate",       "1",    "uint8_t",     "RW"},
  {"EEP", "59",   "DXL Master protocol ver",   "1",    "uint8_t",     "RW"},
  {"EEP", "60",   "WiFi AP SSID",              "32",   "str_t",       "RW"},
  {"EEP", "92",   "WiFi AP SSID P/W",          "32",   "str_t",       "RW"},
  {"EEP", "124",  "Micro-XRCE-DDS Agent IP",   "16",   "str_t",       "RW"},
  {"EEP", "140",  "Micro-XRCE-DDS Agent Port", "2",    "uint16_t",    "RW"},

  {"",    "",     "",                          "",     "",            ""  }
};

static DxlControlTable CtDynamicXel[] =
{
  {"CON", "0",    "Model_Number",          "2",    "uint16_t",    "R" },
  {"CON", "2",    "Model_Info",            "4",    "uint32_t",    "R" },
  {"CON", "6",    "Firmware_Version",      "1",    "uint8_t",     "R" },
  {"EEP", "7",    "ID",                    "1",    "uint8_t",     "RW"},
  {"EEP", "8",    "Baud",                  "1",    "uint8_t",     "RW"},
  {"EEP", "11",   "Operating Mode",        "1",    "uint8_t",     "RW"},
  {"RAM", "64",   "Torque Enable",         "1",    "uint8_t",     "RW"},

  {"",    "",     "",                      "",     "",            ""}
};



static const QString BAUDRATE_STRING[] =
{
  "9600",
  "57600",
  "115200",
  "1000000",
  "2000000",
//  "3000000",
//  "4000000",
//  "4500000",

  ""
};

static const QString MSG_TYPE_STRING[] =
{
  "std_msgs::Bool",
  "std_msgs::Char",
  "std_msgs::Int8",
  "std_msgs::Int16",
  "std_msgs::Int32",
  "std_msgs::Int64",
  "std_msgs::UInt8",
  "std_msgs::UInt16",
  "std_msgs::UInt32",
  "std_msgs::UInt64",
  "std_msgs::Float32",
  "std_msgs::Float64",

  "geometry_msgs::Point",
  "geometry_msgs::Vector3",
  "geometry_msgs::Quaternion",
  "geometry_msgs::Twist",
  "geometry_msgs::Pose",

  ""
};

static const QString TOPIC_MODE_STRING[] =
{
  "Subscriber",
  "Publisher",

  ""
};

static int selected_model_num = 0;

TabDxl::TabDxl(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::TabDxl)
{
  int index = 0;

  ui->setupUi(this);

  log_debug = new LogDebug;
  log_debug->setEnabled(true);
  ui->gridLayout_op3_log_debug->addWidget(log_debug);

  const auto infos = QSerialPortInfo::availablePorts();
  for (const QSerialPortInfo &info : infos)
  {
    ui->comboBoxSerialPort->addItem(info.portName());
  }

  ui->comboBoxProtocolVersion->addItem("1.0");
  ui->comboBoxProtocolVersion->addItem("2.0");

  while(BAUDRATE_STRING[index].length() != 0)
  {
    ui->comboBox_baudrate->addItem(BAUDRATE_STRING[index++]);
  }
  ui->comboBox_baudrate->setCurrentIndex(3);

  ui->tableWidget->setColumnWidth(0,  40);
  ui->tableWidget->setColumnWidth(1,  40);
  ui->tableWidget->setColumnWidth(2, 150);
  ui->tableWidget->setColumnWidth(3,  30);
  ui->tableWidget->setColumnWidth(4,  70);
  ui->tableWidget->setColumnWidth(5,  30);
  ui->tableWidget->setColumnWidth(6, 100);
  ui->tableWidget->setColumnWidth(7,  50);

  dxl_table_length = 0;
  dxl_write_enable = false;

  drawControlTable(0);
}

void TabDxl::on_updateMsgType(int index)
{
  log_debug->printf("update Msg type %d\n", index);

  ui->tableWidget->item(XEL_MSG_TYPE_ROW, XEL_MSG_TYPE_COL)->setText(QVariant(index).toString());
  on_tableWidget_cellChanged(XEL_MSG_TYPE_ROW, XEL_MSG_TYPE_COL);
}

void TabDxl::on_updateBaudrate(int index)
{
  log_debug->printf("update Baudrate %d\n", index);

  ui->tableWidget->item(XEL_BAUDRATE_ROW, XEL_BAUDRATE_COL)->setText(QVariant(index).toString());
  on_tableWidget_cellChanged(XEL_BAUDRATE_ROW, XEL_BAUDRATE_COL);
}

void TabDxl::on_updateMasterBaudrate(int index)
{
  log_debug->printf("update Master Baudrate %d\n", index);

  ui->tableWidget->item(MASTER_XEL_BAUDRATE_ROW, MASTER_XEL_BAUDRATE_COL)->setText(QVariant(index).toString());
  on_tableWidget_cellChanged(MASTER_XEL_BAUDRATE_ROW, MASTER_XEL_BAUDRATE_COL);
}

void TabDxl::on_updateTopicMode(int index)
{
  log_debug->printf("update Topic mode %d\n", index);

  ui->tableWidget->item(XEL_TOPIC_MODE_ROW, XEL_TOPIC_MODE_COL)->setText(QVariant(index).toString());
  on_tableWidget_cellChanged(XEL_TOPIC_MODE_ROW, XEL_TOPIC_MODE_COL);
}

TabDxl::~TabDxl()
{
  delete ui;
}

void TabDxl::setDxlPortString(QString port_string)
{
  dxl_port_string = port_string;
  //ui->lineEdit_port->setText(port_string);
  ui->comboBoxSerialPort->setCurrentText(port_string);
}

void TabDxl::setDxlIdString(QString id_string)
{
  dxl_id_string = id_string;
  ui->lineEdit_id->setText(id_string);
}

void TabDxl::setDxlProtocolIdnex(int protocol_index)
{
  dxl_protocol_index = protocol_index;
  ui->comboBoxProtocolVersion->setCurrentIndex(protocol_index);
}

void TabDxl::setDxlBaudIdnex(int buad_index)
{
  dxl_baud_index = buad_index;
  ui->comboBox_baudrate->setCurrentIndex(buad_index);
}

QString TabDxl::getControlValueString(uint8_t *p_data, QString type_string)
{
  QString ret_string = "0";
  DxlControlValue *p_control_value = (DxlControlValue *)p_data;


  if(p_data == nullptr) return "0";


  if( type_string == "int8_t")
  {
    ret_string = QVariant(p_control_value->s8).toString();
  }
  else if( type_string == "uint8_t")
  {
    ret_string = QVariant(p_control_value->u8).toString();
  }
  else if( type_string == "int16_t")
  {
    ret_string = QVariant(p_control_value->s16).toString();
  }
  else if( type_string == "uint16_t")
  {
    ret_string = QVariant(p_control_value->u16).toString();
  }
  else if( type_string == "int32_t")
  {
    ret_string = QVariant(p_control_value->s32).toString();
  }
  else if( type_string == "uint32_t")
  {
    ret_string = QVariant(p_control_value->u32).toString();
  }
  else if( type_string == "str_t")
  {
    ret_string = QString((char *)p_data);
  }
  else if( type_string == "mac_t")
  {
    uint8_t mac[6];
    char mac_string[18];
    for(uint8_t i = 0; i < 6; i++)
    {
      mac[i] = p_data[i];
    }
    sprintf(mac_string, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    ret_string = QString(mac_string);
  }
  else
  {
    log_debug->printf("err\n");
  }

  return ret_string;
}

bool TabDxl::getDxlDump(uint16_t addr, uint8_t *p_data, uint16_t len)
{
  dynamixel::PortHandler *portHandler = dynamixel::PortHandler::getPortHandler(dxl_port_string.toLocal8Bit().data());
  dynamixel::PacketHandler *packetHandler = dynamixel::PacketHandler::getPacketHandler(dxl_protocol_index==1?2.0:1.0);

  uint8_t  dxl_error       = 0;
  int      dxl_comm_result = COMM_TX_FAIL;
  uint8_t *data            = (uint8_t*)calloc(len, sizeof(uint8_t));

  uint8_t dxl_id;
  int dxl_baud;
  bool ret = false;

  dxl_id = ui->lineEdit_id->text().toInt();
  dxl_baud = ui->comboBox_baudrate->currentText().toInt();

  // Open port
  if (portHandler->openPort() != true)
  {
    log_debug->printf("Failed to open the port!\n");
    log_debug->printf("Press any key to terminate...\n");
    return false;
  }

  // Set port baudrate
  if (portHandler->setBaudRate(dxl_baud) != true)
  {
    log_debug->printf("Failed to change the baudrate!\n");
    log_debug->printf("Press any key to terminate...\n");
    return false;
  }

  dxl_comm_result = packetHandler->readTxRx(portHandler, dxl_id, addr, len, data, &dxl_error);
  if (dxl_comm_result == COMM_SUCCESS)
  {
    if (dxl_error == 0 && dxl_id != BROADCAST_ID){
      log_debug->printf("\n");
      for (int i = addr; i < addr+len; i++)
      {
        log_debug->printf("ADDR %.3d [0x%.4X] :     %.3d [0x%.2X] \n", i, i, data[i-addr], data[i-addr]);
        p_data[i] = data[i-addr];
      }
      log_debug->printf("\n");

      ret = true;
    }else{
      packetHandler->printRxPacketError(dxl_error);
      log_debug->printf("\n Error occured!! [code: %03d] \n\n", dxl_error);
    }
  }
  else
  {
    packetHandler->printTxRxResult(dxl_comm_result);
    log_debug->printf("\n Fail to read! \n\n");
  }

  free(data);

  // Close port
  portHandler->closePort();

  return ret;
}


bool TabDxl::write(dynamixel::PortHandler *portHandler, dynamixel::PacketHandler *packetHandler, uint8_t id, uint16_t addr, uint16_t length, uint8_t *p_data)
{
  uint8_t dxl_error = 0;
  int dxl_comm_result = COMM_TX_FAIL;

  dxl_comm_result = packetHandler->writeTxRx(portHandler, id, addr, length, p_data, &dxl_error);

  if (dxl_comm_result == COMM_SUCCESS)
  {
    if (dxl_error != 0)
    {
      packetHandler->printRxPacketError(dxl_error);
      return false;
    }
  }
  else
  {
    packetHandler->printTxRxResult(dxl_comm_result);
    log_debug->printf("\n Fail to write! \n\n");
    return false;
  }

  return true;
}


bool TabDxl::dxlWrite(uint16_t addr, uint16_t length, uint8_t *p_data)
{
  dynamixel::PortHandler *portHandler = dynamixel::PortHandler::getPortHandler(dxl_port_string.toLocal8Bit().data());
  dynamixel::PacketHandler *packetHandler = dynamixel::PacketHandler::getPacketHandler(dxl_protocol_index==1?2.0:1.0);

//  int dxl_comm_result = COMM_TX_FAIL;             // Communication result
//  uint8_t dxl_error = 0;                          // Dynamixel error
//  uint16_t dxl_model_number;                      // Dynamixel model number

  uint8_t dxl_id;
  int dxl_baud;

  bool ret = true;

  dxl_id = ui->lineEdit_id->text().toInt();
  dxl_baud = ui->comboBox_baudrate->currentText().toInt();

  // Open port
  if (portHandler->openPort())
  {
    log_debug->printf("Succeeded to open the port!\n");
  }
  else
  {
    log_debug->printf("Failed to open the port!\n");
    log_debug->printf("Press any key to terminate...\n");
    return false;
  }

  // Set port baudrate
  if (portHandler->setBaudRate(dxl_baud))
  {
    log_debug->printf("Succeeded to change the baudrate!\n");
  }
  else
  {
    log_debug->printf("Failed to change the baudrate!\n");
    log_debug->printf("Press any key to terminate...\n");
    return false;
  }

  ret = write(portHandler, packetHandler, dxl_id, addr, length, p_data);

  // Close port
  portHandler->closePort();

  return ret;
}

void TabDxl::on_buttonPing_clicked()
{
  dynamixel::PortHandler *portHandler = dynamixel::PortHandler::getPortHandler(dxl_port_string.toLocal8Bit().data());
  dynamixel::PacketHandler *packetHandler = dynamixel::PacketHandler::getPacketHandler(dxl_protocol_index==1?2.0:1.0);

  int dxl_comm_result = COMM_TX_FAIL;             // Communication result
  int dxl_baud;

  dxl_baud = (uint32_t)ui->comboBox_baudrate->currentText().toInt();

  log_debug->printf("%s\r\n", dxl_port_string.toStdString().c_str());

  // Open port
  if (portHandler->openPort())
  {
    log_debug->printf("Succeeded to open the port!\n");
  }
  else
  {
    log_debug->printf("Failed to open the port!\n");
    log_debug->printf("Press any key to terminate...\n");
    return;
  }

  portHandler->setPacketTimeout(5000.0);

  // Set port baudrate
  if (portHandler->setBaudRate(dxl_baud))
  {
    log_debug->printf("Succeeded to change the baudrate!\n");
  }
  else
  {
    log_debug->printf("Failed to change the baudrate!\n");
    log_debug->printf("Press any key to terminate...\n");
    return;
  }

  ui->tableWidget_ID->clearContents();
  ui->tableWidget_ID->setRowCount(0);

  int connected_xel_cnt = 0;
  int row;
  uint16_t model_num;
  QString model_num_str;
  for(uint8_t id = 0; id < 253; id++)
  {
    dxl_comm_result = packetHandler->ping(portHandler, id, &model_num);

    if (dxl_comm_result == COMM_SUCCESS){
      log_debug->printf("Detected XEL : ");
      log_debug->printf("[ID:%03d] \n", id);
      connected_xel_cnt++;

      row = ui->tableWidget_ID->rowCount();
      if(row < connected_xel_cnt){
        ui->tableWidget_ID->insertRow(row);
      }

      switch(model_num)
      {
      case SENSOR_XEL:
        model_num_str = "SensorXEL";
        break;
      case POWER_XEL:
        model_num_str = "PowerXEL";
        break;
      case COMM_XEL:
        model_num_str = "CommXEL";
        break;
      case COMMXEL_W:
        model_num_str = "CommXEL-W";
        break;
      default:
        model_num_str = "DYNAMIXEL";
        break;
      }

      auto tbl_id = new QTableWidgetItem(QVariant(id).toString());
      auto tbl_model_num = new QTableWidgetItem(model_num_str);

      tbl_id->setTextAlignment(Qt::AlignCenter);
      tbl_model_num->setTextAlignment(Qt::AlignCenter);

      ui->tableWidget_ID->setItem(row, 0, tbl_id);
      ui->tableWidget_ID->setItem(row, 1, tbl_model_num);
    }
  }

  // Close port
  portHandler->closePort();
}


void TabDxl::on_buttonReadData_clicked()
{
  uint8_t dxl_buf[1024*4];

  dxl_write_enable = false;

  if( getDxlDump(0, dxl_buf, 149) == false ) return;

  int addr;
  QString type_string;
  QString item_string;

  drawControlTable(selected_model_num);

  for(int i=0; i<dxl_table_length; i++)
  {
    addr = ui->tableWidget->item(i,1)->text().toInt();
    type_string = ui->tableWidget->item(i,4)->text();
    item_string = getControlValueString(&dxl_buf[addr], type_string);

    //log_debug->printf("addr %d\n", addr);
    //log_debug->printString(type_string + "\n");
    //log_debug->printString(getControlValueString(&p_dxl_buf[addr], type_string)+"\n");

    ui->tableWidget->item(i,6)->setText(item_string);
  }

  QComboBox *baudrate_combo;
  baudrate_combo = (QComboBox *)ui->tableWidget->cellWidget(XEL_BAUDRATE_ROW, XEL_BAUDRATE_COL);
  baudrate_combo->setCurrentIndex(ui->tableWidget->item(XEL_BAUDRATE_ROW, XEL_BAUDRATE_COL)->text().toInt());

  switch(selected_model_num)
  {
    case SENSOR_XEL: //sensorXel
    case POWER_XEL: //powerXel
    {
      QComboBox *data_type_combo;
      data_type_combo = (QComboBox *)ui->tableWidget->cellWidget(XEL_MSG_TYPE_ROW, XEL_MSG_TYPE_COL);
      data_type_combo->setCurrentIndex(ui->tableWidget->item(XEL_MSG_TYPE_ROW, XEL_MSG_TYPE_COL)->text().toInt());

      QComboBox *data_direction_combo;
      data_direction_combo = (QComboBox *)ui->tableWidget->cellWidget(XEL_TOPIC_MODE_ROW, XEL_TOPIC_MODE_COL);
      data_direction_combo->setCurrentIndex(ui->tableWidget->item(XEL_TOPIC_MODE_ROW, XEL_TOPIC_MODE_COL)->text().toInt());
    }
      break;

    case COMM_XEL: //commXel
      break;

    case COMMXEL_W: //commXel-W
      QComboBox *data_type_combo;
      data_type_combo = (QComboBox *)ui->tableWidget->cellWidget(XEL_BAUDRATE_ROW, XEL_BAUDRATE_COL);
      data_type_combo->setCurrentIndex(ui->tableWidget->item(XEL_BAUDRATE_ROW, XEL_BAUDRATE_COL)->text().toInt());

      QComboBox *data_direction_combo;
      data_direction_combo = (QComboBox *)ui->tableWidget->cellWidget(MASTER_XEL_BAUDRATE_ROW, MASTER_XEL_BAUDRATE_COL);
      data_direction_combo->setCurrentIndex(ui->tableWidget->item(MASTER_XEL_BAUDRATE_ROW, MASTER_XEL_BAUDRATE_COL)->text().toInt());
      break;

    default: //DXL
      break;
  }

  dxl_write_enable = true;
}


void TabDxl::on_buttonUpload_clicked()
{
  QDir program_dir;
#if defined (__WIN32__) || (__WIN64__)
  QString program_name = "/xel_loader/xel_loader.exe";
#else //__linux__
  QString program_name = "/xel_loader/xel_loader";
#endif
  QString program_path = program_dir.currentPath()+program_name;

  QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                        "",
                                                        tr("*.bin"));

  QStringList args_name = {" - port name = ", " - baudrate = ", " - F/W address = ", " - binary file path = ", " - option1 = ", " - option2 = "};
  QStringList arguments = {dxl_port_string, "1000000", "0x08003000", fileName, "1", "1"};

  log_debug->printf("\r\n\r\n ======= Start Upload ======= \r\n");
  for(int i=0; i<arguments.size(); i++){
    log_debug->printf(args_name.at(i).toStdString().c_str());
    log_debug->printf(arguments.at(i).toStdString().c_str());
    log_debug->printf("\r\n");
  }
  log_debug->printf(" ======= Upload End ======= \r\n\r\n");

  QProcess process;
  process.start(program_path, arguments);
  process.waitForFinished(-1);
}

void TabDxl::on_buttonPortRefresh_clicked()
{
  const auto infos = QSerialPortInfo::availablePorts();
  ui->comboBoxSerialPort->clear();
  for (const QSerialPortInfo &info : infos)
  {
    ui->comboBoxSerialPort->addItem(info.portName());
  }
}

//void TabDxl::on_lineEdit_port_textChanged(const QString &arg1)
//{
//  dxl_port_string = arg1;
//}

void TabDxl::on_lineEdit_id_textChanged(const QString &arg1)
{
  dxl_id_string = arg1;
}

void TabDxl::on_comboBoxSerialPort_currentTextChanged(const QString &arg1)
{
  dxl_port_string = arg1;
}

void TabDxl::on_comboBoxProtocolVersion_currentIndexChanged(int index)
{
  dxl_protocol_index = index;
}

void TabDxl::on_comboBox_baudrate_currentIndexChanged(int index)
{
  dxl_baud_index = index;
}

void TabDxl::on_tableWidget_cellChanged(int row, int column)
{
  if(dxl_write_enable)
  {
    if(column == 6)
    {
      bool result = false;
      int addr;
      int size;
      int value;
      QString type;
      QString str;

      addr  = ui->tableWidget->item(row,1)->text().toInt();
      size  = ui->tableWidget->item(row,3)->text().toInt();
      value = ui->tableWidget->item(row,6)->text().toInt();
      type  = ui->tableWidget->item(row,4)->text();
      str   = ui->tableWidget->item(row,6)->text();

      log_debug->printf("write addr: %d, size: %d, value:%d\n", addr, size, value);

      if (type == "str_t")
      {
        result = dxlWrite(addr, size, (uint8_t *)str.toStdString().c_str());
      }
      else if(type == "mac_t")
      {
        uint8_t data[6];
        char* data_str = (char*)str.toStdString().c_str();
        for(int i = 0, j = 0; i < 6 && j < 12; i++, j+=2)
        {
          data_str[j] -= '0';
          if(data_str[j] > 9) data_str[j] -= 7;
          if(data_str[j] > 15) data_str[j] -= 0x20;
          data[i] = (uint8_t)data_str[j];
          data[i] <<= 4;

          data_str[j+1] -= '0';
          if(data_str[j+1] > 9) data_str[j+1] -= 7;
          if(data_str[j+1] > 15) data_str[j+1] -= 0x20;
          data[i] += (uint8_t)data_str[j+1];
        }

        result = dxlWrite(addr, size, data);
      }
      else
      {
        result = dxlWrite(addr, size, (uint8_t *)&value);
      }

      log_debug->printf(result==true?"OK\n":"FAIL\n");

      selected_model_num = ui->tableWidget->item(0,6)->text().toInt();

      on_buttonReadData_clicked();
    }
  }
}

void TabDxl::on_tableWidget_ID_doubleClicked(const QModelIndex &index)
{
  int row;
  int id;
  QString model_str;

  row = index.row();
  id = ui->tableWidget_ID->item(row,0)->text().toInt();
  model_str = ui->tableWidget_ID->item(row,1)->text();

  if (id > 0)
  {
    ui->lineEdit_id->setText(ui->tableWidget_ID->item(row,0)->text());
    ui->lineEdit_model->setText(model_str);

    if(model_str == "SensorXEL")
    {
      selected_model_num = SENSOR_XEL;
    }
    else if(model_str == "PowerXEL")
    {
      selected_model_num = POWER_XEL;
    }
    else if(model_str == "CommXEL")
    {
      selected_model_num = COMM_XEL;
    }
    else if(model_str == "CommXEL-W")
    {
      selected_model_num = COMMXEL_W;
    }
    else
    {
      selected_model_num = 0;
    }

    on_buttonReadData_clicked();
  }
}

void TabDxl::drawControlTable(int model_num)
{
  DxlControlTable *p_table;
  int index = 0;

  for(int i = 0; i < dxl_table_length; i++)
  {
    ui->tableWidget->removeRow(i);
  }
  ui->tableWidget->setRowCount(0);

  switch(model_num)
  {
    case SENSOR_XEL: //sensorXel
      p_table = CtSensorXelDefault;
      break;

    case POWER_XEL: //powerXel
      p_table = CtPowerXel;
      break;

    case COMM_XEL: //commXel
      p_table = CtCommXel;
      break;

    case COMMXEL_W: //commXel-W
      p_table = CtCommXel_W;
      break;

    default: //DXL
      p_table = CtDynamicXel;
      break;
  }

  while(p_table[index].addr.length() != 0)
  {
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);

    auto tbl_area = new QTableWidgetItem(p_table[index].area);
    auto tbl_addr = new QTableWidgetItem(p_table[index].addr);
    auto tbl_name = new QTableWidgetItem(p_table[index].name);
    auto tbl_size = new QTableWidgetItem(p_table[index].size);
    auto tbl_type = new QTableWidgetItem(p_table[index].type);
    auto tbl_acc  = new QTableWidgetItem(p_table[index].acc);
    auto tbl_data = new QTableWidgetItem(" ");
    auto tbl_graph= new QTableWidgetItem();

    tbl_graph->setCheckState(Qt::Unchecked);

    tbl_area->setTextAlignment(Qt::AlignCenter);
    tbl_addr->setTextAlignment(Qt::AlignCenter);
    tbl_name->setTextAlignment(Qt::AlignCenter);
    tbl_size->setTextAlignment(Qt::AlignCenter);
    tbl_type->setTextAlignment(Qt::AlignCenter);
    tbl_acc->setTextAlignment(Qt::AlignCenter);
    tbl_data->setTextAlignment(Qt::AlignCenter);
    tbl_graph->setTextAlignment(Qt::AlignCenter);

    tbl_area->setFlags(tbl_area->flags() & ~Qt::ItemIsEditable);
    tbl_addr->setFlags(tbl_addr->flags() & ~Qt::ItemIsEditable);
    tbl_name->setFlags(tbl_name->flags() & ~Qt::ItemIsEditable);
    tbl_size->setFlags(tbl_size->flags() & ~Qt::ItemIsEditable);
    tbl_type->setFlags(tbl_type->flags() & ~Qt::ItemIsEditable);
    tbl_acc->setFlags(tbl_acc->flags() & ~Qt::ItemIsEditable);

    ui->tableWidget->setItem(index, 0, tbl_area);
    ui->tableWidget->setItem(index, 1, tbl_addr);
    ui->tableWidget->setItem(index, 2, tbl_name);
    ui->tableWidget->setItem(index, 3, tbl_size);
    ui->tableWidget->setItem(index, 4, tbl_type);
    ui->tableWidget->setItem(index, 5, tbl_acc);
    ui->tableWidget->setItem(index, 6, tbl_data);
    ui->tableWidget->setItem(index, 7, tbl_graph);

    index++;
  }
  dxl_table_length = index;

  /* BAUDRATE COMBO*/
  index = 0;
  QComboBox *baudrate_box = new QComboBox;
  while(BAUDRATE_STRING[index] != nullptr)
  {
    baudrate_box->addItem(BAUDRATE_STRING[index++]);
  }
  connect(baudrate_box, SIGNAL(currentIndexChanged(int)), this, SLOT(on_updateBaudrate(int)));
  ui->tableWidget->setCellWidget(XEL_BAUDRATE_ROW,  XEL_BAUDRATE_COL, baudrate_box);

  switch(model_num)
  {
    case SENSOR_XEL: //sensorXel
    case POWER_XEL: //powerXel
    {
      /* DATA_TYPE COMBO*/
      index = 0;
      QComboBox *data_type_box = new QComboBox;
      while(MSG_TYPE_STRING[index] != nullptr)
      {
        data_type_box->addItem(MSG_TYPE_STRING[index++]);
      }
      connect(data_type_box, SIGNAL(currentIndexChanged(int)), this, SLOT(on_updateMsgType(int)));
      ui->tableWidget->setCellWidget(XEL_MSG_TYPE_ROW,  XEL_MSG_TYPE_COL, data_type_box);

      /* DATA_DIRECTION COMBO*/
      index = 0;
      QComboBox *data_direction_box = new QComboBox;
      while(TOPIC_MODE_STRING[index] != nullptr)
      {
        data_direction_box->addItem(TOPIC_MODE_STRING[index++]);
      }
      connect(data_direction_box, SIGNAL(currentIndexChanged(int)), this, SLOT(on_updateTopicMode(int)));
      ui->tableWidget->setCellWidget(XEL_TOPIC_MODE_ROW,  XEL_TOPIC_MODE_COL, data_direction_box);
    }
      break;

    case COMM_XEL: //commXel
      break;
    case COMMXEL_W: //commXel
    {
      /* BAUDRATE COMBO*/
      index = 0;
      QComboBox *baudrate_box = new QComboBox;
      while(BAUDRATE_STRING[index] != nullptr)
      {
        baudrate_box->addItem(BAUDRATE_STRING[index++]);
      }
      connect(baudrate_box, SIGNAL(currentIndexChanged(int)), this, SLOT(on_updateBaudrate(int)));
      ui->tableWidget->setCellWidget(XEL_BAUDRATE_ROW,  XEL_BAUDRATE_COL, baudrate_box);

      /* Master BAUDRATE COMBO*/
      index = 0;
      QComboBox *master_baudrate_box = new QComboBox;
      while(BAUDRATE_STRING[index] != nullptr)
      {
        master_baudrate_box->addItem(BAUDRATE_STRING[index++]);
      }
      connect(master_baudrate_box, SIGNAL(currentIndexChanged(int)), this, SLOT(on_updateMasterBaudrate(int)));
      ui->tableWidget->setCellWidget(MASTER_XEL_BAUDRATE_ROW,  MASTER_XEL_BAUDRATE_COL, master_baudrate_box);
    }
      break;
    default:
      break;
  }

  //ui->lineEdit_port->setText(dxl_port_string);
  ui->comboBoxSerialPort->setCurrentText(dxl_port_string);

  getControlValueString(nullptr, "int8_t");
}
