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

#include "creategraphicdialog.h"
#include "ui_creategraphicdialog.h"

CreateGraphicDialog::CreateGraphicDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateGraphicDialog)
{
    ui->setupUi(this);
}

CreateGraphicDialog::~CreateGraphicDialog()
{
    delete ui;
}

void CreateGraphicDialog::setTemplates(const QStringList &list)
{
    ui->m_templateCombo->clear();
    ui->m_templateCombo->addItems(list);
}

QString CreateGraphicDialog::name() const
{
    return ui->m_nameEdit->text();
}

QString CreateGraphicDialog::templateName() const
{
    return ui->m_templateCombo->currentText();
}
