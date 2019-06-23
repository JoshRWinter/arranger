#include <fstream>

#include <QFileDialog>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QKeyEvent>

#include "Arranger.h"

Arranger::Arranger()
{
	resize(900, 700);
	setWindowTitle("Atlas Arranger");

	// layouts
	auto hbox = new QHBoxLayout;
	auto hbox_listitem_controls = new QHBoxLayout;
	setLayout(hbox);
	auto vbox = new QVBoxLayout;

	// some widgets
	list = new QListWidget;
	scroller = new QScrollArea(this);
	auto newtexture = new QPushButton("Add Texture");
	auto deletetexture = new QPushButton("Remove Texture");
	auto exportatlas = new QPushButton("Export");
	auto importatlas = new QPushButton("Import");
	auto reload = new QPushButton(style()->standardIcon(QStyle::SP_BrowserReload), "");
	m_panel = new ArrangerPanel(1);
	scroller->setWidget(m_panel);

	// sidebar widget settings
	const int sidebar_width = 275;
	list->setMaximumWidth(sidebar_width);
	newtexture->setMaximumWidth(sidebar_width);

	// hook up dem buttuns
	QObject::connect(newtexture, &QPushButton::clicked, this, &Arranger::slot_add_texture);
	QObject::connect(deletetexture, &QPushButton::clicked, this, &Arranger::slot_remove_texture);
	QObject::connect(exportatlas, &QPushButton::clicked, this, &Arranger::slot_export);
	QObject::connect(importatlas, &QPushButton::clicked, this, &Arranger::slot_import);
	QObject::connect(reload, &QPushButton::clicked, this, &Arranger::slot_reload);

	// stir it all together
	hbox_listitem_controls->addWidget(reload);
	vbox->addWidget(list);
	vbox->addLayout(hbox_listitem_controls);
	vbox->addWidget(newtexture);
	vbox->addWidget(deletetexture);
	vbox->addWidget(importatlas);
	vbox->addWidget(exportatlas);
	hbox->addLayout(vbox);
	hbox->addWidget(scroller);
}

void Arranger::keyPressEvent(QKeyEvent *key)
{
	if(key->key() == Qt::Key_Control)
		m_panel->set_align(false);
	else if(key->key() == Qt::Key_B)
		m_panel->set_border(!m_panel->get_border());
	else if(key->key() == Qt::Key_Return)
		QMessageBox::information(this, "Atlas info", m_panel->info().c_str());
}

void Arranger::keyReleaseEvent(QKeyEvent *key)
{
	if(key->key() == Qt::Key_Control)
		m_panel->set_align(true);
}

void Arranger::slot_add_texture()
{
	const QString path = QFileDialog::getOpenFileName(this, "Add Texture", "", "Targa TGA Images (*.tga)");
	const QDir current = QDir::currentPath();
	const QString import = current.relativeFilePath(path);

	if(import.isNull())
		return;

	try
	{
		m_panel->add(import.toStdString());
		list->addItem(import);
	}
	catch(const std::exception &e)
	{
		QMessageBox::critical(this, "Error", press::swrite("could not load file {}: {}", import.toStdString(), e.what()).c_str());
	}
}

void Arranger::slot_remove_texture()
{
	QListWidgetItem *current = list->currentItem();
	if(current == NULL)
		return;

	QString name = current->text();
	try
	{
		m_panel->remove(name.toStdString());
	}
	catch(const std::exception &e)
	{
		QMessageBox::warning(this, "Couldn't remove texture", e.what());
	}
	delete list->takeItem(list->currentRow());
}

void Arranger::slot_export()
{
	const QString save = QFileDialog::getSaveFileName(this, "Export to...");
	if(save.isNull())
		return;
	if((int)save.toStdString().rfind(".adesc") != save.length() - 6)
	{
		QMessageBox::critical(this, "Error", press::swrite("Unwilling to overwrite file {}", save.toStdString()).c_str());
		return;
	}

	std::ofstream out(save.toStdString());
	if(!out)
	{
		QMessageBox::critical(this, "Error", "Could not open file \"" + save + "\" for writing");
		return;
	}

	const std::vector<Entry> entries = m_panel->get_entries();
	for(const Entry &entry : entries)
		out << "\"" << entry.name << "\" " << entry.x << " " << entry.y << "\n";
}

void Arranger::slot_import()
{
	m_panel->clear();
	list->clear();
	const QString import = QFileDialog::getOpenFileName(this, "Import atlas...");
	if(import.isNull())
		return;

	// read lines and import atlases
	std::ifstream in(import.toStdString());
	if(!in)
	{
		QMessageBox::critical(this, "Error", "Could not open file \"" + import + "\" for reading");
		return;
	}

	int lineno = 0;
	try
	{
		while(in.good())
		{
			++lineno;

			std::string line;
			std::getline(in, line);
			if(line.length() == 0 || line == "\n")
				continue;

			if(line.at(0) != '"')
				throw std::runtime_error("malformed input line");

			const int second_quote_position = line.find("\"", 1);
			const int first_space = line.find(" ", second_quote_position + 1);
			const int second_space = line.find(" ", first_space + 1);

			// file name
			const std::string filepath = line.substr(1, second_quote_position - 1);

			// x coord
			const std::string str_xcoord = line.substr(first_space + 1, second_space - first_space - 1);
			int xcoord;
			if(sscanf(str_xcoord.c_str(), "%d", &xcoord) != 1)
				throw std::runtime_error("couldn't convert \"" + str_xcoord + "\" to an integer");

			// ycoord
			const std::string str_ycoord = line.substr(second_space + 1);
			int ycoord;
			if(sscanf(str_ycoord.c_str(), "%d", &ycoord) != 1)
				throw std::runtime_error("couldn't convert \"" + str_ycoord + "\" to an integer");

			m_panel->add(filepath, xcoord, ycoord);
			list->addItem(filepath.c_str());
		}

		m_panel->flip();
	}
	catch(const std::runtime_error &e)
	{
		QMessageBox::critical(this, "Error", e.what());
	}
	catch(...)
	{
		QMessageBox::critical(this, "Error", "Malformed input");
	}
}

void Arranger::slot_reload()
{
	QListWidgetItem *current = list->currentItem();
	if(current == NULL)
	{
		QMessageBox::warning(this, "Invalid", "Select a texture from the list on the left");
		return;
	}

	try
	{
		m_panel->reload(current->text().toStdString());
	}
	catch(const std::runtime_error &e)
	{
		QMessageBox::critical(this, "Error", e.what());
	}
}
