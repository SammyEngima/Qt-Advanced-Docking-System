#ifndef DockOverlayH
#define DockOverlayH
/*******************************************************************************
** QtAdcancedDockingSystem
** Copyright (C) 2017 Uwe Kindler
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/


//============================================================================
//                                   INCLUDES
//============================================================================
#include <QPointer>
#include <QHash>
#include <QRect>
#include <QFrame>
class QGridLayout;

#include "ads_globals.h"

namespace ads
{
struct DockOverlayPrivate;
class CDockOverlayCross;

/*!
 * DockOverlay paints a translucent rectangle over another widget. The geometry
 * of the rectangle is based on the mouse location.
 */
class CDockOverlay : public QFrame
{
	Q_OBJECT
private:
	DockOverlayPrivate* d; //< private data class
	friend class DockOverlayPrivate;
	friend class DockOverlayCross;

public:
	enum eMode
	{
		ModeDockAreaOverlay,
		ModeContainerOverlay
	};

	/**
	 * Creates a dock overlay
	 */
	CDockOverlay(QWidget* parent, eMode Mode = ModeDockAreaOverlay);

	/**
	 * Virtual destructor
	 */
	virtual ~CDockOverlay();

	/**
	 * Configures the areas that are allowed for docking
	 */
	void setAllowedAreas(DockWidgetAreas areas);

	/**
	 * Returns flags with all allowed drop areas
	 */
	DockWidgetAreas allowedAreas() const;

	/**
	 * Returns the drop area under the current cursor location
	 */
	DockWidgetArea dropAreaUnderCursor() const;

	/**
	 * Show the drop overly for the given target widget
	 */
	DockWidgetArea showOverlay(QWidget* target);

	/**
	 * Show drop overlay for the given target widget and the given rectangle
	 */
	void showOverlay(QWidget* target, const QRect& targetAreaRect);

	/**
	 * Hides the overlay
	 */
	void hideOverlay();

protected:
	virtual void paintEvent(QPaintEvent *e) override;
	virtual void showEvent(QShowEvent* e) override;
	virtual void hideEvent(QHideEvent* e) override;
	virtual void resizeEvent(QResizeEvent* e) override;
	virtual void moveEvent(QMoveEvent* e) override;
};


struct DockOverlayCrossPrivate;
/*!
 * DockOverlayCross shows a cross with 5 different drop area possibilities.
 * I could have handled everything inside DockOverlay, but because of some
 * styling issues it's better to have a separate class for the cross.
 */
class CDockOverlayCross : public QWidget
{
	Q_OBJECT
private:
	DockOverlayCrossPrivate* d;
	friend class DockOverlayCrossPrivate;
	friend class CDockOverlay;

public:
	/**
	 * Creates an overlay corss for the given overlay
	 */
	CDockOverlayCross(CDockOverlay* overlay);

	/**
	 * Virtual destructor
	 */
	virtual ~CDockOverlayCross();

	/**
	 * Returns the dock widget area depending on the current cursor location.
	 * The function checks, if the mouse cursor is inside of any drop indicator
	 * widget and returns the corresponding DockWidgetArea.
	 */
	DockWidgetArea cursorLocation() const;

	/**
	 * Sets up the overlay cross for the given overlay mode
	 */
	void setupOverlayCross(CDockOverlay::eMode Mode);

	/**
	 * Resets and updates the
	 */
	void reset();

protected:
	virtual void showEvent(QShowEvent* e) override;
	void setAreaWidgets(const QHash<DockWidgetArea, QWidget*>& widgets);

private:

}; // CDockOverlayCross

} // namespace ads
#endif // DockOverlayH
