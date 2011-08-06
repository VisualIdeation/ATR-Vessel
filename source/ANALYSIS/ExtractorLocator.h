/*
 * ExtractorLocator - Class for locators applying visualization algorithms to
 * data sets.
 * Copyright (c) 2005-2009 Oliver Kreylos
 * Date: July 14, 2010
 */

#ifndef EXTRACTORLOCATOR_INCLUDED
#define EXTRACTORLOCATOR_INCLUDED

#include <Abstract/DataSet.h>

#include <ANALYSIS/BaseLocator.h>
#include <ANALYSIS/Extractor.h>

/* Forward declarations: */
namespace GLMotif {
class Widget;
class PopupWindow;
class Label;
}

class ExtractorLocator:public BaseLocator,public Extractor
	{
	/* Embedded classes: */
	private:
	typedef Visualization::Abstract::DataSet DataSet;
	typedef DataSet::Locator Locator;
	
	/* Elements: */
	private:
	GLMotif::Widget* settingsDialog; // The element extractor's settings dialog
	GLMotif::PopupWindow* busyDialog; // Dialog window to show while a non-incremental extractor is busy
	GLMotif::Label* percentageLabel; // Label to display completion percentage in the busy dialog
	Locator* locator; // A locator for the visualization algorithm
	bool dragging; // Flag if the tool's button is currently pressed
	unsigned int lastSeedRequestID; // ID used to identify the last issued seed request
	
	/* Private methods: */
	GLMotif::PopupWindow* createBusyDialog(const char* algorithmName); // Creates the busy dialog
	void busyFunction(float completionPercentage); // Called during long-running operations
	
	/* Constructors and destructors: */
	public:
	ExtractorLocator(Vrui::LocatorTool * sTool,VirtualATR * sApplication,Algorithm * sExtractor);
	virtual ~ExtractorLocator(void);
	
	/* Methods from Vrui::LocatorToolAdapter: */
	virtual void motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData);
	virtual void buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData);
	virtual void buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData);
	
	/* Methods from BaseLocator: */
	virtual void highlightLocator(GLContextData& contextData) const;
	virtual void glRenderAction(GLContextData& contextData) const;
	virtual void glRenderActionTransparent(GLContextData& contextData) const;
	
	/* Methods from Extractor: */
	virtual void update(void);
	};

#endif
