/*
    This file is part of KCachegrind.

    SPDX-FileCopyrightText: 2009-2016 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only
*/

/*
 * QCachegrind configuration dialog
 */

#include "configdialog.h"

#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QListWidget>
#include <QStackedWidget>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>

#include "generalsettings.h"
#include "sourcesettings.h"
#include "colorsettings.h"

//
// ConfigDialog
//

ConfigDialog::ConfigDialog(TraceData* data, QWidget* parent, const QString &s)
    : QDialog(parent)
{
    setWindowTitle(tr("Configure QCachegrind"));

    _listWidget = new QListWidget(this);
    _listWidget->setMaximumWidth(140);
    _widgetStack = new QStackedWidget(this);
    _titleLabel = new QLabel(this);
    QFont labelFont;
    labelFont.setBold(true);
    _titleLabel->setFont(labelFont);
    _errorLabel = new QLabel(this);
    _errorLabel->setIndent(9);

    QDialogButtonBox* bbox = new QDialogButtonBox(this);
    bbox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

    QVBoxLayout* vbox1 = new QVBoxLayout();
    vbox1->addWidget(_titleLabel);
    QFrame* f1 = new QFrame(this);
    f1->setFrameShape(QFrame::HLine);
    vbox1->addWidget(f1);
    vbox1->addWidget(_errorLabel);
    vbox1->addWidget(_widgetStack);

    QHBoxLayout* hbox = new QHBoxLayout();
    hbox->addWidget(_listWidget);
    hbox->addLayout(vbox1);
    QVBoxLayout* vbox = new QVBoxLayout(this);
    vbox->addLayout(hbox);
    QFrame* f2 = new QFrame(this);
    f2->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    vbox->addWidget(f2);
    vbox->addWidget(bbox);

    connect(bbox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(bbox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(_listWidget, SIGNAL(currentTextChanged(QString)),
            this, SLOT(listItemChanged(QString)));
    connect(&_clearTimer, &QTimer::timeout, this, &ConfigDialog::clearError);

    addPage(new GeneralSettings(this));
    addPage(new SourceSettings(data, this));
    addPage(new ColorSettings(data, this));

    activate(s);
}

void ConfigDialog::addPage(ConfigPage* p)
{
    _widgetStack->addWidget(p);
    _listWidget->addItem(p->title());
    _pages.insert(p->title(), p);
}

void ConfigDialog::listItemChanged(QString s)
{
    ConfigPage* p = _pages.value(s);
    if (!p) return;

    _titleLabel->setText(p->longTitle());
    _widgetStack->setCurrentWidget(p);
    if (!_activeSetting.isEmpty()) {
        p->activate(_activeSetting);
        _activeSetting.clear();
    }
}

void ConfigDialog::clearError()
{
    _errorLabel->setText(QString());
}

void ConfigDialog::activate(QString s)
{
    if (s.isEmpty())
        _listWidget->setCurrentRow(0);

    QString page = s;
    _activeSetting.clear();
    int p = s.indexOf(QLatin1Char('/'));
    if (p>0) {
        page = s.left(p);
        _activeSetting = s.mid(p+1);
    }

    for(int row=0; row<_listWidget->count(); row++) {
        QListWidgetItem* i = _listWidget->item(row);
        if (i->text() != page) continue;

        if (_listWidget->currentRow() == row)
            // even without page change, forward activation
            listItemChanged(page);
        else
            _listWidget->setCurrentRow(row);
    }
}

QString ConfigDialog::currentPage()
{
    return _listWidget->currentItem()->text();
}

void ConfigDialog::accept()
{
    ConfigPage* p;
    QString errorMsg, errorItem;
    foreach(p, _pages)
        if (!p->check(errorMsg, errorItem)) {
            if (!errorMsg.isEmpty()) {
                errorMsg = QStringLiteral("<font color=red>%1</color>").arg(errorMsg);
                _errorLabel->setText(errorMsg);
                _clearTimer.start(5000);
            }
            activate(QStringLiteral("%1/%2").arg(p->title(), errorItem));
            return;
        }

    foreach(p, _pages)
        p->accept();

    QDialog::accept();
}

#include "moc_configdialog.cpp"
