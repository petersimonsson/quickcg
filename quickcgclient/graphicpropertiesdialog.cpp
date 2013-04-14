// Copyright 2012  Peter Simonsson <peter.simonsson@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "graphicpropertiesdialog.h"
#include "ui_graphicpropertiesdialog.h"

#include <QLabel>
#include <QLineEdit>
#include <QDebug>

GraphicPropertiesDialog::GraphicPropertiesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GraphicPropertiesDialog)
{
    ui->setupUi(this);

    ui->m_onAirTimerInterval->setRange(0, 1000);
}

GraphicPropertiesDialog::~GraphicPropertiesDialog()
{
    delete ui;
}

void GraphicPropertiesDialog::setProperties(const QList<QPair<QString, QVariant> > properties)
{
    for(int i = 0; i < properties.count(); ++i)
    {
        QLineEdit* edit = new QLineEdit(properties.at(i).second.toString(), this);
        edit->setProperty("qcgProperty", properties.at(i).first);
        ui->m_propertyLayout->addRow(properties.at(i).first.mid(3) + ":", edit);

        if(i == 0)
        {
            edit->setFocus();
        }
    }
}

QList<QPair<QString, QVariant> > GraphicPropertiesDialog::properties() const
{
    QList<QPair<QString, QVariant> > list;

    foreach(QObject* object, children())
    {
        QLineEdit* edit = qobject_cast<QLineEdit*>(object);

        if(edit)
        {
            QString name = edit->property("qcgProperty").toString();

            if(!name.isEmpty())
            {
                list.append(QPair<QString, QVariant>(name, edit->text()));
            }
        }
    }

    return list;
}

void GraphicPropertiesDialog::setOnAirTimerEnabled(bool enabled)
{
    ui->m_onAirTimerGroup->setChecked(enabled);
}

bool GraphicPropertiesDialog::onAirTimerEnabled() const
{
    return ui->m_onAirTimerGroup->isChecked();
}

void GraphicPropertiesDialog::setOnAirTimerInterval(int ms)
{
    ui->m_onAirTimerInterval->setValue(ms / 1000);
}

int GraphicPropertiesDialog::onAirTimerInterval() const
{
    return ui->m_onAirTimerInterval->value() * 1000;
}

void GraphicPropertiesDialog::setGroup(const QString& name)
{
    ui->m_groupEdit->setText(name);
}

QString GraphicPropertiesDialog::group() const
{
    return ui->m_groupEdit->text();
}
