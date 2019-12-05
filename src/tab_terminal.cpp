#include "tab_terminal.h"
#include "ui_tab_terminal.h"
#include "console.h"

TabTerminal::TabTerminal(QWidget *parent) :
  QWidget(parent), ui(new Ui::TabTerminal)
{
  ui->setupUi(this);

  //log_debug = new LogDebug;
  //log_debug->setEnabled(true);
  //ui->gridLayout_op3_log_debug->addWidget(log_debug);

  console = new Console;
  console->setEnabled(false);
  ui->gridLayout->addWidget(console);

  setAcceptDrops(true);
}

TabTerminal::~TabTerminal()
{
  delete ui;
}
