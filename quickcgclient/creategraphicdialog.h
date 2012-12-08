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

#ifndef CREATEGRAPHICDIALOG_H
#define CREATEGRAPHICDIALOG_H

#include <QDialog>

namespace Ui {
    class CreateGraphicDialog;
}

class CreateGraphicDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateGraphicDialog(QWidget *parent = 0);
    ~CreateGraphicDialog();

    void setTemplates(const QStringList &list);

    QString name() const;
    QString templateName() const;

private:
    Ui::CreateGraphicDialog *ui;
};

#endif // CREATEGRAPHICDIALOG_H
