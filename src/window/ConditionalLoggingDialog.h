/*
  Copyright (c) 2026 Jayachandran Dharuman
  This file is part of CANgaroo.
*/

#pragma once

#include <QDialog>
#include <QTreeWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <core/ConditionalLoggingManager.h>
#include <core/ThemeManager.h>

class Backend;

class ConditionalLoggingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConditionalLoggingDialog(Backend &backend, QWidget *parent = nullptr);
    ~ConditionalLoggingDialog();

private slots:
    void addCondition();
    void removeCondition();
    void selectLogFile();
    void onAccept();
    void onItemChanged(QTreeWidgetItem *item, int column);
    void applyTheme(ThemeManager::Theme theme);

private:
    void setupUi();
    void populateSignals();

    Backend &_backend;
    QTreeWidget *_conditionsTree;
    QTreeWidget *_signalsTree;
    QLineEdit *_logFileEdit;
    QCheckBox *_andLogicCheckBox;
    QCheckBox *_enabledCheckBox;
};
