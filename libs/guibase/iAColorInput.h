// Copyright 2016-2023, the open_iA contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QWidget>

//! Input line for a color value; with a text box, an indicator and a button for opening a color chooser
//! Currently only used in iAParameterDialog. If used somewhere else, extract method
//! definitions to separate .cpp file to avoid according errors
class iAColorInput : public QWidget
{
	Q_OBJECT
public:
	iAColorInput(QWidget* parent, QVariant const& value) :
		QWidget(parent),
		m_lineEdit(new QLineEdit(this)),
		m_colorIndicator(new QLabel(this)),
		m_colorDialogButton(new QToolButton(this))
	{
		setLayout(new QHBoxLayout);
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		layout()->setContentsMargins(0, 0, 0, 0);
		layout()->setSpacing(4);
		layout()->addWidget(m_lineEdit);
		m_colorIndicator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		m_colorIndicator->setAutoFillBackground(true);
		layout()->addWidget(m_colorIndicator);
		layout()->addWidget(m_colorDialogButton);
		m_colorDialogButton->setText("...");
		connect(m_colorDialogButton, &QToolButton::clicked, this,
			[this]() {
				QColorDialog dlg;
				QColor color(m_lineEdit->text());
				dlg.setCurrentColor(color);
				if (dlg.exec() != QDialog::Accepted || !dlg.selectedColor().isValid())
				{
					return;
				}
				QColor col = dlg.selectedColor();
				m_lineEdit->setText(col.name());
				updateIndicatorColor(col);
			});
		QColor col = value.value<QColor>();
		m_lineEdit->setText(col.name());
		updateIndicatorColor(col);
		connect(m_lineEdit, &QLineEdit::textChanged, this,
			[this]() {
				updateIndicatorColor(QColor(m_lineEdit->text()));
			});
	}
	void updateIndicatorColor(QColor const & color)
	{
		QPalette pal;
		pal.setColor(QPalette::Window, color);
		m_colorIndicator->setPalette(pal);
	}
	QVariant value() const
	{
		return QColor(m_lineEdit->text());
	}
	void setValue(QVariant const& value)
	{
		QColor col = value.value<QColor>();
		QSignalBlocker block(m_lineEdit);
		m_lineEdit->setText(col.name());
		updateIndicatorColor(col);
	}
private:
	QLineEdit* m_lineEdit;
	QLabel* m_colorIndicator;
	QToolButton* m_colorDialogButton;
};
