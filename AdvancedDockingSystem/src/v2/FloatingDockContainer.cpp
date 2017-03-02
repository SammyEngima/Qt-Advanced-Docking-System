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
/// \file   FloatingDockContainer.cpp
/// \author Uwe Kindler
/// \date   01.03.2017
/// \brief  Implementation of CFloatingDockContainer class
//============================================================================


//============================================================================
//                                   INCLUDES
//============================================================================
#include "FloatingDockContainer.h"

#include <QBoxLayout>
#include <QApplication>
#include <QMouseEvent>
#include <QPointer>

#include <iostream>

#include "DockContainerWidget.h"
#include "DockAreaWidget.h"
#include "DockManager.h"
#include "DockWidget.h"
#include "DockOverlay.h"

namespace ads
{
static unsigned int zOrderCounter = 0;

/**
 * Private data class of CFloatingDockContainer class (pimpl)
 */
struct FloatingDockContainerPrivate
{
	CFloatingDockContainer* _this;
	CDockContainerWidget* DockContainer;
	unsigned int zOrderIndex = ++zOrderCounter;
	QPointer<CDockManager> DockManager;
	bool DraggingActive = false;
	QPoint DragStartMousePosition;
	CDockContainerWidget* DropContainer = nullptr;

	/**
	 * Private data constructor
	 */
	FloatingDockContainerPrivate(CFloatingDockContainer* _public);

	void titleMouseReleaseEvent();
	void updateDropOverlays(const QPoint& GlobalPos);
	void setDraggingActive(bool Active);
};
// struct FloatingDockContainerPrivate

//============================================================================
FloatingDockContainerPrivate::FloatingDockContainerPrivate(CFloatingDockContainer* _public) :
	_this(_public)
{

}



//============================================================================
void FloatingDockContainerPrivate::titleMouseReleaseEvent()
{
	setDraggingActive(false);
	if (!DropContainer)
	{
		return;
	}

	std::cout << "Dropped" << std::endl;
	/*CMainContainerWidget* MainContainerWidget = mainContainerWidget();
	m_DropContainer->dropFloatingWidget(this, QCursor::pos());*/
	DockManager->containerOverlay()->hideOverlay();
	DockManager->dockAreaOverlay()->hideOverlay();
}



//============================================================================
void FloatingDockContainerPrivate::updateDropOverlays(const QPoint& GlobalPos)
{
	if (!_this->isVisible() || !DockManager)
	{
		return;
	}

    auto Containers = DockManager->dockContainers();
    CDockContainerWidget* TopContainer = nullptr;
    for (auto ContainerWidget : Containers)
    {
    	if (!ContainerWidget->isVisible())
    	{
    		continue;
    	}

    	if (DockContainer == ContainerWidget)
    	{
    		continue;
    	}

    	QPoint MappedPos = ContainerWidget->mapFromGlobal(GlobalPos);
    	if (ContainerWidget->rect().contains(MappedPos))
    	{
    		if (!TopContainer || ContainerWidget->isInFrontOf(TopContainer))
    		{
    			TopContainer = ContainerWidget;
    		}
    	}
    }

    DropContainer = TopContainer;
    auto ContainerOverlay = DockManager->containerOverlay();
    auto DockAreaOverlay = DockManager->dockAreaOverlay();

    if (!TopContainer)
    {
    	ContainerOverlay->hideOverlay();
    	DockAreaOverlay->hideOverlay();
    	return;
    }

	ContainerOverlay->showOverlay(TopContainer);
	ContainerOverlay->raise();

    auto DockArea = TopContainer->dockAreaAt(GlobalPos);
    if (DockArea)
    {
    	DockAreaOverlay->setAllowedAreas(AllDockAreas);
        DockAreaOverlay->showOverlay(DockArea);
    }
    else
    {
    	DockAreaOverlay->hideOverlay();
    }


    if (TopContainer)
    {
    	ContainerOverlay->showOverlay(TopContainer);
		ContainerOverlay->raise();
    }
    else
    {
    	ContainerOverlay->hideOverlay();
    }
}


//============================================================================
void FloatingDockContainerPrivate::setDraggingActive(bool Active)
{
	DraggingActive = Active;
}


//============================================================================
CFloatingDockContainer::CFloatingDockContainer(CDockManager* DockManager) :
	QWidget(DockManager, Qt::Window),
	d(new FloatingDockContainerPrivate(this))
{
	d->DockManager = DockManager;
    QBoxLayout* l = new QBoxLayout(QBoxLayout::TopToBottom);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);
    setLayout(l);

    d->DockContainer = new CDockContainerWidget(DockManager, this);
	l->addWidget(d->DockContainer);
	DockManager->registerFloatingWidget(this);

	qApp->installEventFilter(this);
}


//============================================================================
CFloatingDockContainer::CFloatingDockContainer(CDockAreaWidget* DockArea) :
	CFloatingDockContainer(DockArea->dockManager())
{
	d->DockContainer->addDockArea(DockArea);
}


//============================================================================
CFloatingDockContainer::CFloatingDockContainer(CDockWidget* DockWidget) :
	CFloatingDockContainer(DockWidget->dockManager())
{
	d->DockContainer->addDockWidget(CenterDockWidgetArea, DockWidget);
}

//============================================================================
CFloatingDockContainer::~CFloatingDockContainer()
{
	if (d->DockManager)
	{
		d->DockManager->removeFloatingWidget(this);
	}
	delete d;
}


//============================================================================
CDockContainerWidget* CFloatingDockContainer::dockContainer() const
{
	return d->DockContainer;
}


//============================================================================
void CFloatingDockContainer::changeEvent(QEvent *event)
{
	QWidget::changeEvent(event);
	if ((event->type() == QEvent::ActivationChange) && isActiveWindow())
    {
		std::cout << "FloatingWidget::changeEvent QEvent::ActivationChange " << std::endl;
		d->zOrderIndex = ++zOrderCounter;
        return;
    }
}


//============================================================================
void CFloatingDockContainer::moveEvent(QMoveEvent *event)
{
	QWidget::moveEvent(event);
	if (!qApp->mouseButtons().testFlag(Qt::LeftButton))
	{
		if (d->DraggingActive)
		{
			d->setDraggingActive(false);
		}
		return;
	}

	if (d->DraggingActive)
	{
		d->updateDropOverlays(QCursor::pos());
	}
}


//============================================================================
bool CFloatingDockContainer::event(QEvent *e)
{
	if ((e->type() == QEvent::NonClientAreaMouseButtonPress))
	{
		if (QGuiApplication::mouseButtons() == Qt::LeftButton)
		{
			std::cout << "FloatingWidget::event Event::NonClientAreaMouseButtonPress" << e->type() << std::endl;
			d->setDraggingActive(true);
		}
	}
	else if (e->type() == QEvent::NonClientAreaMouseButtonDblClick)
	{
		std::cout << "FloatingWidget::event QEvent::NonClientAreaMouseButtonDblClick" << std::endl;
		d->setDraggingActive(false);
	}
	else if ((e->type() == QEvent::NonClientAreaMouseButtonRelease) && d->DraggingActive)
	{
		std::cout << "FloatingWidget::event QEvent::NonClientAreaMouseButtonRelease" << std::endl;
		d->titleMouseReleaseEvent();
	}

	return QWidget::event(e);
}


//============================================================================
bool CFloatingDockContainer::eventFilter(QObject *watched, QEvent *event)
{
	if (event->type() == QEvent::MouseButtonRelease && d->DraggingActive)
	{
		std::cout << "FloatingWidget::eventFilter QEvent::MouseButtonRelease" << std::endl;
		d->titleMouseReleaseEvent();
	}
	else if ((event->type() == QEvent::MouseMove) && d->DraggingActive)
	{
		QMouseEvent* MouseEvent = dynamic_cast<QMouseEvent*>(event);
		int BorderSize = (frameSize().width() - size().width()) / 2;
		const QPoint moveToPos = QCursor::pos() - d->DragStartMousePosition - QPoint(BorderSize, 0);
		move(moveToPos);
		return true;
	}
	return false;
}


//============================================================================
void CFloatingDockContainer::startFloating(const QPoint& Pos, const QSize& Size)
{
	resize(Size);
	d->setDraggingActive(true);
	QPoint TargetPos = QCursor::pos() - Pos;
	move(TargetPos);
    show();
	d->DragStartMousePosition = Pos;
}
} // namespace ads

//---------------------------------------------------------------------------
// EOF FloatingDockContainer.cpp
