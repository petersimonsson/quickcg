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

#ifndef GRAPHICPROPERTIESDIALOG_H
#define GRAPHICPROPERTIESDIALOG_H

#include <QDialog>

namespace Ui {
    class GraphicPropertiesDialog;
}

class GraphicPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GraphicPropertiesDialog(QWidget *parent = 0);
    ~GraphicPropertiesDialog();

    void setProperties(const QList<QPair<QString, QVariant> > properties);
    QList<QPair<QString, QVariant> > properties() const;

    void setOnAirTimerEnabled(bool enabled);
    bool onAirTimerEnabled() const;

    void setOnAirTimerInterval(int ms);
    int onAirTimerInterval() const;

    void setGroup(const QString& name);
    QString group() const;

private:
    Ui::GraphicPropertiesDialog *ui;
};

#endif // GRAPHICPROPERTIESDIALOG_H
