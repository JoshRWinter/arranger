#include "ArrangerPanel.h"

ArrangerPanel::ArrangerPanel()
{
	setAutoFillBackground(true);
	QPalette palette;
	palette.setColor(QPalette::Background, QColor(220, 220, 220));
	setPalette(palette);
}
