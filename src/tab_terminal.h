#ifndef TABTERMINAL_H
#define TABTERMINAL_H

#include <QWidget>
#include <QtSerialPort/QSerialPort>

namespace Ui {
class TabTerminal;
}

class Console;
class LogDebug;
class SettingsDialog;

class TabTerminal : public QWidget
{
  Q_OBJECT

public:
  explicit TabTerminal(QWidget *parent = nullptr);
  ~TabTerminal();

  QSerialPort *serial;
  Console *console;

private:
  Ui::TabTerminal *ui;
};

#endif // TABTERMINAL_H
