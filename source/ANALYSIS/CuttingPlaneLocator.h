/*
 * CuttingPlaneLocator - Class for locators rendering cutting planes.
 * Copyright (c) 2006-2009 Oliver Kreylos
 * Date: July 13, 2010
 */

#ifndef CUTTINGPLANELOCATOR_INCLUDED
#define CUTTINGPLANELOCATOR_INCLUDED

#include <ANALYSIS/BaseLocator.h>

/* Forward declarations: */
class CuttingPlane;

class CuttingPlaneLocator:public BaseLocator
	{
	/* Elements: */
	private:
	CuttingPlane* cuttingPlane; // Pointer to the cutting plane allocated for this tool
	
	/* Constructors and destructors: */
	public:
	CuttingPlaneLocator(Vrui::LocatorTool* sTool,VirtualATR* sApplication);
	virtual ~CuttingPlaneLocator(void);
	
	/* Methods from Vrui::LocatorToolAdapter: */
	virtual void motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData);
	virtual void buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData);
	virtual void buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData);
	};

#endif
