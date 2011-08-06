/*
 * Description: VirtualATR.h - A Virtual-ATR Application
 * Author: Patrick O'Leary
 * Date: April 16, 2010
 */
#ifndef VIRTUALATR_INCLUDED
#define VIRTUALATR_INCLUDED

#include <vector>

#include <GL/gl.h>
#include <GL/GLObject.h>
#include <GLMotif/FileSelectionDialog.h>
#include <GLMotif/Menu.h>
#include <GLMotif/RadioBox.h>
#include <GLMotif/Slider.h>
#include <GLMotif/ToggleButton.h>
#include <Misc/CallbackData.h>
#include <Vrui/Tools/LocatorTool.h>
#include <Vrui/LocatorToolAdapter.h>
#include <Vrui/ToolManager.h>
#include <Vrui/Application.h>

/* Begin Forward declarations: */
class ATR;
class BaseLocator;
class ClippingPlane;
class CuttingPlane;
class ElementList;
namespace Visualization {
namespace Abstract {
class Algorithm;
class CoordinateTransformer;
class DataSet;
class DataSetRenderer;
class Element;
class Module;
class Parameters;
class VariableManager;
}
}
namespace GLMotif {
class Popup;
class PopupMenu;
class PopupWindow;
class TextField;
}
/* End Forward declarations: */

class VirtualATR: public Vrui::Application, public GLObject {
private:
	typedef Visualization::Abstract::DataSet DataSet;
	typedef Visualization::Abstract::VariableManager VariableManager;
	typedef Visualization::Abstract::DataSetRenderer DataSetRenderer;
	typedef Visualization::Abstract::Parameters Parameters;
	typedef Visualization::Abstract::Algorithm Algorithm;
	typedef Visualization::Abstract::Element Element;
	typedef Visualization::Abstract::CoordinateTransformer CoordinateTransformer;
	typedef Visualization::Abstract::Module Module;
	typedef Plugins::FactoryManager<Module> ModuleManager;
	typedef std::vector<BaseLocator*> BaseLocatorList;

	friend class BaseLocator;
	friend class CuttingPlaneLocator;
	friend class EvaluationLocator;
	friend class ScalarEvaluationLocator;
	friend class VectorEvaluationLocator;
	friend class ExtractorLocator;

	/* Embedded classes: */
private:
	struct DataItem: public GLObject::DataItem {
	public:
		/* Elements: */
		int data;
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
	};

public:
	/* Constructors and destructors: */
	VirtualATR(int& argc, char**& argv, char**& appDefaults);
	virtual ~VirtualATR(void);

	/* Methods: */
	void centerDisplayCallback(Misc::CallbackData * callbackData);
	virtual void display(GLContextData& contextData) const;
	virtual void frame(void);
	ClippingPlane * getClippingPlanes(void);
	int getNumberOfClippingPlanes(void);
	virtual void initContext(GLContextData& contextData) const;
	void menuToggleSelectCallback(
			GLMotif::ToggleButton::ValueChangedCallbackData * callbackData);
	void sliderCallback(
			GLMotif::Slider::ValueChangedCallbackData * callbackData);

private:
	ModuleManager moduleManager; // Manager to load 3D visualization modules from dynamic libraries
	Module * module; // Visualization module
	DataSet * dataSet; // Data set to visualize
	VariableManager * variableManager; // Manager to organize data sets and scalar and vector variables
	GLColor<GLfloat,4> dataSetRenderColor; // Color to use when rendering the data set
	DataSetRenderer* dataSetRenderer; // Renderer for the data set
	CoordinateTransformer* coordinateTransformer; // Transformer from Cartesian coordinates back to data set coordinates
	int firstScalarAlgorithmIndex; // Index of first module-provided scalar algorithm in algorithm menu
	int firstVectorAlgorithmIndex; // Index of first module-provided vector algorithm in algorithm menu
	size_t numberOfCuttingPlanes; // Maximum number of cutting planes supported
	CuttingPlane* cuttingPlanes; // Array of available cutting planes
	ElementList* elementList; // List of previously extracted visualization elements
	int algorithm; // The currently selected algorithm
	GLMotif::ToggleButton* showElementListToggle; // Toggle button to show the element list dialog
	/* Lock flags for modal dialogs: */
	bool inLoadPalette; // Flag whether the user is currently selecting a palette to load
	bool inLoadElements; // Flag whether the user is currently selecting an element file to load
	int analysisTool;
	ATR * atr;
	BaseLocatorList baseLocators;
	ClippingPlane * clippingPlanes;
	GLMotif::PopupMenu* mainMenu;
	int numberOfClippingPlanes;
	GLMotif::PopupWindow* renderDialog;
	GLMotif::PopupWindow* osccDialog;
	GLMotif::ToggleButton * showVesselToggle;
	GLMotif::ToggleButton * showVesselToggleRD;
	GLMotif::ToggleButton * showClear_VesselToggle;
	GLMotif::ToggleButton * showClear_VesselToggleRD;
	GLMotif::ToggleButton * wireframeToggle;
	GLMotif::ToggleButton * wireframeToggleRD;

	/* 
	 * OSCC_#: # represents angle at which cylinders are rotated 
	 * from 0 degrees. One number means all cylinders are at same 
	 * rotation; multiple numbers mean NW quadrant, then NE 
	 * quadrant, then SE quadrant, then SW quadrant (NOTE: 
	 * decimals are represented by underscores. EX 51.8 = 51_8). 
	 */
	GLMotif::ToggleButton * showOSCC_5Toggle;
	GLMotif::ToggleButton * showOSCC_5ToggleOSCCD;
	GLMotif::ToggleButton * showOSCC_51_8Toggle;
	GLMotif::ToggleButton * showOSCC_51_8ToggleOSCCD;
	GLMotif::ToggleButton * showOSCC_85Toggle;
	GLMotif::ToggleButton * showOSCC_85ToggleOSCCD;
	GLMotif::ToggleButton * showOSCC_120Toggle;
	GLMotif::ToggleButton * showOSCC_120ToggleOSCCD;
	GLMotif::ToggleButton * showOSCC_155Toggle;
	GLMotif::ToggleButton * showOSCC_155ToggleOSCCD;
	GLMotif::ToggleButton * showOSCC_30_85_85_85Toggle;
	GLMotif::ToggleButton * showOSCC_30_85_85_85ToggleOSCCD;
	GLMotif::ToggleButton * showOSCC_40_85_85_85Toggle;
	GLMotif::ToggleButton * showOSCC_40_85_85_85ToggleOSCCD;
	GLMotif::ToggleButton * showOSCC_50_85_85_85Toggle;
	GLMotif::ToggleButton * showOSCC_50_85_85_85ToggleOSCCD;
	GLMotif::ToggleButton * showOSCC_60_85_85_85Toggle;
	GLMotif::ToggleButton * showOSCC_60_85_85_85ToggleOSCCD;
	GLMotif::ToggleButton * showOSCC_70_85_85_85Toggle;
	GLMotif::ToggleButton * showOSCC_70_85_85_85ToggleOSCCD;
	GLMotif::ToggleButton * showOSCC_80_85_85_85Toggle;
	GLMotif::ToggleButton * showOSCC_80_85_85_85ToggleOSCCD;
	GLMotif::ToggleButton * showOSCC_90_85_85_85Toggle;
	GLMotif::ToggleButton * showOSCC_90_85_85_85ToggleOSCCD;
	GLMotif::ToggleButton * showOSCC_100_85_85_85Toggle;
	GLMotif::ToggleButton * showOSCC_100_85_85_85ToggleOSCCD;
	GLMotif::ToggleButton * showOSCC_110_85_85_85Toggle;
	GLMotif::ToggleButton * showOSCC_110_85_85_85ToggleOSCCD;
	GLMotif::ToggleButton * showOSCC_120_85_85_85Toggle;
	GLMotif::ToggleButton * showOSCC_120_85_85_85ToggleOSCCD;

	/* Private methods: */

	void changeAlgorithmCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void changeAnalysisToolsCallback(
			GLMotif::RadioBox::ValueChangedCallbackData * callbackData);
	void changeRenderingModeCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void changeScalarVariableCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void changeVectorVariableCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void clearElementsCallback(Misc::CallbackData* cbData);
	GLMotif::Popup * createAlgorithmsMenu(void);
	GLMotif::Popup * createAnalysisToolsSubMenu(void);
	GLMotif::Popup * createColorMenu(void);
	GLMotif::Popup * createElementsMenu(void);
	GLMotif::PopupMenu * createMainMenu(void);
	GLMotif::PopupWindow * createRenderDialog(void);
	GLMotif::PopupWindow * createOSCCDialog(void);
	GLMotif::Popup * createRenderTogglesMenu(void);
	GLMotif::Popup * createOSCCTogglesMenu(void);
	GLMotif::Popup * createVesselTogglesMenu(void);
	GLMotif::Popup * createRenderingModesMenu(void);
	GLMotif::Popup * createScalarVariablesMenu(void);
	GLMotif::Popup * createStandardLuminancePalettesMenu(void);
	void createStandardLuminancePaletteCallback(GLMotif::Menu::EntrySelectCallbackData* cbData);
	GLMotif::Popup * createStandardSaturationPalettesMenu(void);
	void createStandardSaturationPaletteCallback(GLMotif::Menu::EntrySelectCallbackData* cbData);
	GLMotif::Popup * createVectorVariablesMenu(void);
	void loadElements(const char* elementFileName,bool ascii); // Loads all visualization elements defined in the given file
	void loadElementsCallback(Misc::CallbackData* cbData);
	void loadElementsOKCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData);
	void loadElementsCancelCallback(GLMotif::FileSelectionDialog::CancelCallbackData* cbData);
	void loadPaletteCallback(Misc::CallbackData* cbData);
	void loadPaletteCancelCallback(GLMotif::FileSelectionDialog::CancelCallbackData* cbData);
	void loadPaletteOKCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData);
	void saveElementsCallback(Misc::CallbackData* cbData);
	void showColorBarCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void showElementListCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void showPaletteEditorCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	virtual void toolCreationCallback(
			Vrui::ToolManager::ToolCreationCallbackData * callbackData);
	virtual void toolDestructionCallback(
			Vrui::ToolManager::ToolDestructionCallbackData * callbackData);
};
#endif
