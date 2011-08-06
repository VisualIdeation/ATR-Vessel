/*
 * VectorEvaluationLocator - Class for locators evaluating vector properties of
 * data sets.
 * Copyright (c) 2008-2009 Oliver Kreylos
 * Date: July 14, 2010
 */

#ifndef VECTOREVALUATIONLOCATOR_INCLUDED
#define VECTOREVALUATIONLOCATOR_INCLUDED

#include <GLMotif/Slider.h>

#include <Abstract/ScalarExtractor.h>
#include <Abstract/VectorExtractor.h>

#include <ANALYSIS/EvaluationLocator.h>

/* Forward declarations: */
class GLColorMap;
namespace GLMotif {
class TextField;
}

class VectorEvaluationLocator:public EvaluationLocator
	{
	/* Embedded classes: */
	private:
	typedef Visualization::Abstract::ScalarExtractor ScalarExtractor;
	typedef ScalarExtractor::Scalar Scalar;
	typedef Visualization::Abstract::VectorExtractor VectorExtractor;
	typedef VectorExtractor::Vector Vector;
	
	/* Elements: */
	const VectorExtractor* vectorExtractor; // Extractor for the evaluated vector value
	const ScalarExtractor* scalarExtractor; // Extractor for the evaluated scalar value (to color arrow rendering)
	const GLColorMap* colorMap; // Color map for the evaluated scalar value
	GLMotif::TextField* values[3]; // The vector component value text field
	bool valueValid; // Flag if the evaluation value is valid
	Vector currentValue; // The current evaluation value
	Scalar currentScalarValue; // The current scalar value
	GLMotif::TextField* arrowScaleValue; // Text field showing current arrow scale
	GLMotif::Slider* arrowScaleSlider; // Slider to adjust the arrow scale
	Scalar arrowLengthScale; // Scaling factor for arrow rendering
	
	/* Constructors and destructors: */
	public:
	VectorEvaluationLocator(Vrui::LocatorTool * sTool, VirtualATR * sApplication);
	virtual ~VectorEvaluationLocator(void);
	
	/* Methods from Vrui::LocatorToolAdapter: */
	virtual void motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData);
	
	/* Methods from class BaseLocator: */
	virtual void highlightLocator(GLContextData& contextData) const;
	
	/* New methods: */
	void arrowScaleSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
	};

#endif
