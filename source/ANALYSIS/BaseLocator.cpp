/*
 * BaseLocator - Base class for locators in Virtual-ATR application.
 * Copyright (c) 2005-2009 Oliver Kreylos
 * Date: July 13, 2010
 */

#include <ANALYSIS/BaseLocator.h>
#include <VirtualATR.h>

/****************************
Methods of class BaseLocator:
****************************/

BaseLocator::BaseLocator(Vrui::LocatorTool* sLocatorTool,VirtualATR* sApplication)
	:Vrui::LocatorToolAdapter(sLocatorTool),
	 application(sApplication)
	{
	}

void BaseLocator::highlightLocator(GLContextData& contextData) const
	{
	/* Render nothing */
	}

void BaseLocator::glRenderAction(GLContextData& contextData) const
	{
	/* Render nothing */
	}

void BaseLocator::glRenderActionTransparent(GLContextData& contextData) const
	{
	/* Render nothing */
	}
