/*
 * EvaluationLocator - Base class for locators evaluating properties of data sets.
 * Copyright (c) 2006-2009 Oliver Kreylos
 * Date: July 13, 2010
 */

#ifndef EVALUATIONLOCATOR_INCLUDED
#define EVALUATIONLOCATOR_INCLUDED

#include <Geometry/Point.h>
#include <Vrui/Geometry.h>

#include <Abstract/DataSet.h>

#include <ANALYSIS/BaseLocator.h>

/* Forward declarations: */
namespace GLMotif {
class PopupWindow;
class RowColumn;
class TextField;
}
namespace Visualization {
namespace Abstract {
class CoordinateTransformer;
}
}

class EvaluationLocator:public BaseLocator
	{
	/* Embedded classes: */
	protected:
	typedef Visualization::Abstract::DataSet::Locator Locator;
	typedef Visualization::Abstract::CoordinateTransformer CoordinateTransformer;
	
	/* Elements: */
	GLMotif::PopupWindow* evaluationDialogPopup; // Pointer to the evaluation dialog window
	GLMotif::RowColumn* evaluationDialog; // Pointer to the evaluation dialog
	GLMotif::TextField* pos[3]; // The coordinate labels for the evaluation position
	Locator* locator; // A locator for evaluation
	Vrui::Point point; // The evaluation point
	bool dragging; // Flag if the locator is currently dragging the evaluation point
	bool hasPoint; // Flag whether the locator has a position

	/* Constructors and destructors: */
	public:
	EvaluationLocator(Vrui::LocatorTool* sTool,VirtualATR* sApplication,const char* dialogWindowTitle);
	virtual ~EvaluationLocator(void);

	/* Methods from Vrui::LocatorToolAdapter: */
	virtual void motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData);
	virtual void buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData);
	virtual void buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData);
	
	/* Methods from class BaseLocator: */
	virtual void highlightLocator(GLContextData& contextData) const;
	};

#endif
