/*
 * ScalarEvaluationLocator - Class for locators evaluating scalar properties of
 * data sets.
 * Copyright (c) 2008-2009 Oliver Kreylos
 * Date: July 14, 2010
 */

#ifndef SCALAREVALUATIONLOCATOR_INCLUDED
#define SCALAREVALUATIONLOCATOR_INCLUDED

#include <Abstract/ScalarExtractor.h>

#include <ANALYSIS/EvaluationLocator.h>

/* Forward declarations: */
namespace Misc {
class CallbackData;
}
namespace GLMotif {
class TextField;
}

class ScalarEvaluationLocator:public EvaluationLocator
	{
	/* Embedded classes: */
	private:
	typedef Visualization::Abstract::ScalarExtractor ScalarExtractor;
	typedef ScalarExtractor::Scalar Scalar;
	
	/* Elements: */
	const ScalarExtractor* scalarExtractor; // Extractor for the evaluated scalar value
	GLMotif::TextField* value; // The value text field
	bool valueValid; // Flag if the evaluation value is valid
	Scalar currentValue; // The current evaluation value

	/* Constructors and destructors: */
	public:
	ScalarEvaluationLocator(Vrui::LocatorTool * sTool, VirtualATR * sApplication);
	virtual ~ScalarEvaluationLocator(void);

	/* Methods from Vrui::LocatorToolAdapter: */
	virtual void motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData);
	
	/* New methods: */
	void insertControlPointCallback(Misc::CallbackData* cbData);
	};

#endif
