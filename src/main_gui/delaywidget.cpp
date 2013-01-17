/*******************************************************************************
 *                                                                             *
 *   F1LT - unofficial Formula 1 live timing application                       *
 *   Copyright (C) 2012-2013  Mariusz Pilarek (pieczaro@gmail.com)             *
 *                                                                             *
 *   This program is free software: you can redistribute it and/or modify      *
 *   it under the terms of the GNU General Public License as published by      *
 *   the Free Software Foundation, either version 3 of the License, or         *
 *   (at your option) any later version.                                       *
 *                                                                             *
 *   This program is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 *   GNU General Public License for more details.                              *
 *                                                                             *
 *   You should have received a copy of the GNU General Public License         *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *                                                                             *
 *******************************************************************************/


#include "delaywidget.h"
#include "ui_delaywidget.h"

#include <QDebug>

DelayWidget::DelayWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DelayWidget), delay(0)
{
    ui->setupUi(this);
    ui->syncLabel->setVisible(false);
//    connect(ui->spinBox, SIGNAL(valueChanged(int)), this, SIGNAL(delayChanged(int)));
}

DelayWidget::~DelayWidget()
{
    delete ui;
}


void DelayWidget::on_spinBox_valueChanged(int arg1)
{
    qDebug() << "DELAYWIDGET" << delay << arg1;

    int prevDelay = delay;
    delay = arg1;

    emit delayChanged(prevDelay, delay);
}

void DelayWidget::synchronizingTimer(bool sync)
{
    ui->syncLabel->setVisible(sync);
}
