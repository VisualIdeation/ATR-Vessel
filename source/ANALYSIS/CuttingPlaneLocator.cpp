/*
 * CuttingPlaneLocator - Class for locators rendering cutting planes.
 * Copyright (c) 2006-2009 Oliver Kreylos
 * Date: July 13, 2010
 */

#include <ANALYSIS/CuttingPlaneLocator.h>

#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>

#include <ANALYSIS/CuttingPlane.h>
#include <VirtualATR.h>

/************************************
Methods of class CuttingPlaneLocator:
************************************/

CuttingPlaneLocator::CuttingPlaneLocator(Vrui::LocatorTool* sLocatorTool,VirtualATR* sApplication)
	:BaseLocator(sLocatorTool,sApplication),
	 cuttingPlane(0)
	{
	/* Find a cutting plane index for this locator: */
	for(size_t i=0;i<application->numberOfCuttingPlanes;++i)
		if(!application->cuttingPlanes[i].allocated)
			{
			cuttingPlane=&application->cuttingPlanes[i];
			break;
			}
	
	/* Allocate the cutting plane: */
	if(cuttingPlane!=0)
		{
		cuttingPlane->allocated=true;
		cuttingPlane->active=false;
		}
	}

CuttingPlaneLocator::~CuttingPlaneLocator(void)
	{
	/* De-allocate the cutting plane: */
	if(cuttingPlane!=0)
		{
		cuttingPlane->active=false;
		cuttingPlane->allocated=false;
		}
	}

void CuttingPlaneLocator::motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData)
	{
	if(cuttingPlane!=0&&cuttingPlane->active)
		{
		/* Update the cutting plane equation: */
		Vrui::Vector planeNormal=cbData->currentTransformation.transform(Vrui::Vector(0,1,0));
		Vrui::Point planePoint=cbData->currentTransformation.getOrigin();
		cuttingPlane->plane=CuttingPlane::Plane(planeNormal,planePoint);
		}
	}

void CuttingPlaneLocator::buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData)
	{
	/* Activate the cutting plane: */
	if(cuttingPlane!=0)
		cuttingPlane->active=true;
	}

void CuttingPlaneLocator::buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData)
	{
	/* Deactivate the cutting plane: */
	if(cuttingPlane!=0)
		cuttingPlane->active=false;
	}
