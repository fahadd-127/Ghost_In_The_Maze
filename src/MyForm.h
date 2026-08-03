#ifndef MYFORM_H
#define MYFORM_H

#include <QWidget>
#include "ui_MyForm.h"

class MyForm : public QWidget
{
  Q_OBJECT

  public:
    MyForm(QWidget *parent = nullptr);

  private:
    void setupGameOverDialog();
    void showGameOverDialog();

    Ui::MyForm ui;
};

#endif
