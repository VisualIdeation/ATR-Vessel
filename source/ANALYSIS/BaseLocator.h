/*
 * BaseLocator - Base class for locators in Virtual-ATR application.
 * Copyright (c) 2005-2009 Oliver Kreylos
 * Date: July 13, 2010
 */

#ifndef BASELOCATOR_INCLUDED
#define BASELOCATOR_INCLUDED

#include <Vrui/LocatorToolAdapter.h>

/* Forward declarations: */
class GLContextData;
class VirtualATR;

class BaseLocator:public Vrui::LocatorToolAdapter
	{
	/* Elements: */
	protected:
	VirtualATR * application;

	/* Constructors and destructors: */
	public:
	BaseLocator(Vrui::LocatorTool* sTool,VirtualATR* sApplication);

	/* New methods: */
	virtual void highlightLocator(GLContextData& contextData) const; // Renders the locator itself
	virtual void glRenderAction(GLContextData& contextData) const; // Renders opaque elements and other objects controlled by the locator
	virtual void glRenderActionTransparent(GLContextData& contextData) const; // Renders transparent elements and other objects controlled by the locator
	};

#endif
