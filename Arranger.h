#ifndef ARRANGER_H
#define ARRANGER_H

#include <QWidget>
#include <QListWidget>

#include "press.h"
#include "ArrangerPanel.h"

class Arranger : public QWidget
{
public:
	Arranger();

protected:
	virtual void keyPressEvent(QKeyEvent*) override;
	virtual void keyReleaseEvent(QKeyEvent*) override;

private:
	void slot_add_texture();
	void slot_remove_texture();
	void slot_export();

	QListWidget *list;
	ArrangerPanel *m_panel;
};

#endif
