/*
 * Description: VirtualATR.cpp - Virtual-ATR Application
 * Author: Patrick O'Leary
 * Date: April 16, 2010
 */

#include <ctype.h>
#include <string.h>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <string>

#include <GL/gl.h>

#include <Comm/MulticastPipe.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLMatrixTemplates.h>
#include <GL/GLMaterial.h>
#include <GL/GLContextData.h>
#include <GL/GLModels.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLPolylineTube.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>
#include <GL/GLTransformationWrappers.h>
#include <GL/GLWindow.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/Blind.h>
#include <GLMotif/Label.h>
#include <GLMotif/Button.h>
#include <GLMotif/CascadeButton.h>
#include <GLMotif/Menu.h>
#include <GLMotif/Separator.h>
#include <GLMotif/SubMenu.h>
#include <GLMotif/Popup.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/TextField.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/Timer.h>
#include <Misc/FileNameExtensions.h>
#include <Misc/CreateNumberedFileName.h>
#include <Misc/File.h>
#include <Vrui/CoordinateManager.h>
#include <Vrui/Tools/SurfaceNavigationTool.h>
#include <Vrui/Vrui.h>
#include <Vrui/Application.h>

#include <Abstract/DataSetRenderer.h>
#include <Abstract/CoordinateTransformer.h>
#include <Abstract/VariableManager.h>
#include <Abstract/Algorithm.h>
#include <Abstract/Element.h>
#include <Abstract/Module.h>
#include <ANALYSIS/BaseLocator.h>
#include <ANALYSIS/ClippingPlane.h>
#include <ANALYSIS/ClippingPlaneLocator.h>
#include <ANALYSIS/CuttingPlane.h>
#include <ANALYSIS/CuttingPlaneLocator.h>
#include <ANALYSIS/ExtractorLocator.h>
#include <ANALYSIS/ScalarEvaluationLocator.h>
#include <ANALYSIS/VectorEvaluationLocator.h>
#include <MODEL/ATR.h>

#include <ElementList.h>
#include <VirtualATR.h>

namespace {

/****************
Helper functions:
****************/

std::string readToken(Misc::File& file,int& nextChar)
	{
	/* Skip whitespace: */
	while(nextChar!=EOF&&isspace(nextChar))
		nextChar=fgetc(file.getFilePtr());

	/* Read the next token: */
	std::string result="";
	if(nextChar=='"')
		{
		/* Skip the opening quote: */
		nextChar=fgetc(file.getFilePtr());

		/* Read a quoted token: */
		while(nextChar!=EOF&&nextChar!='"')
			{
			result+=char(nextChar);
			nextChar=fgetc(file.getFilePtr());
			}

		if(nextChar=='"')
			nextChar=fgetc(file.getFilePtr());
		else
			Misc::throwStdErr("unterminated quoted token in input file");
		}
	else
		{
		/* Read an unquoted token: */
		while(nextChar!=EOF&&!isspace(nextChar))
			{
			result+=char(nextChar);
			nextChar=fgetc(file.getFilePtr());
			}
		}

	return result;
	}

}

/*****************************************
 Methods of class VirtualATR::DataItem:
 *****************************************/

VirtualATR::DataItem::DataItem(void) {

}

VirtualATR::DataItem::~DataItem(void) {

}

/****************************************************
 Constructors and Destructors of class VirtualATR:
 ****************************************************/

/* VirtualATR - Constructor for VirtualATR class.
 * 		extends Vrui::Application
 *
 * parameter argc - int&
 * parameter argv - char**&
 * parameter applicationDefaults - char**&
 */
VirtualATR::VirtualATR(int& argc, char**& argv, char**& appDefaults) :
	Vrui::Application(argc, argv, appDefaults), algorithm(0), analysisTool(0),
	clippingPlanes(0), coordinateTransformer(0), cuttingPlanes(0), dataSet(0), dataSetRenderer(0), elementList(0), firstScalarAlgorithmIndex(0), firstVectorAlgorithmIndex(0), inLoadElements(false), inLoadPalette(false), mainMenu(0), module(0), moduleManager(VIRTUALATR_MODULENAMETEMPLATE), numberOfClippingPlanes(0), numberOfCuttingPlanes(0), renderDialog(0), osccDialog(0), showElementListToggle(0), variableManager(0) {

	/* Create the ATR Scene */
	atr = new ATR();
	atr->config();
	atr->toggleClear_Vessel();
	atr->toggleOSCC_5();
	atr->toggleOSCC_85();
	atr->toggleOSCC_120();
	atr->toggleOSCC_155();
	atr->toggleOSCC_30_85_85_85();
	atr->toggleOSCC_40_85_85_85();
	atr->toggleOSCC_50_85_85_85();
	atr->toggleOSCC_60_85_85_85();
	atr->toggleOSCC_70_85_85_85();
	atr->toggleOSCC_80_85_85_85();
	atr->toggleOSCC_90_85_85_85();
	atr->toggleOSCC_100_85_85_85();
	atr->toggleOSCC_110_85_85_85();
	atr->toggleOSCC_120_85_85_85();

	/* Initialize Clippling Planes */
	numberOfClippingPlanes = 6;
	clippingPlanes = new ClippingPlane[numberOfClippingPlanes];
	for (int i = 0; i < numberOfClippingPlanes; ++i) {
		clippingPlanes[i].setAllocated(false);
		clippingPlanes[i].setActive(false);
	}

	/* Parse the command line: */
	std::string moduleClassName="";
	std::vector<std::string> dataSetArgs;
	const char* argColorMapName=0;
	std::vector<const char*> loadFileNames;
	for(int i=1;i<argc;++i)
		{
		if(argv[i][0]=='-')
			{
			if(strcasecmp(argv[i]+1,"class")==0)
				{
				/* Get visualization module class name and data set arguments from command line: */
				++i;
				if(i>=argc)
					Misc::throwStdErr("VirtualATR::VirtualATR: missing module class name after -class");
				moduleClassName=argv[i];
				++i;
				while(i<argc&&strcmp(argv[i],";")!=0)
					{
					dataSetArgs.push_back(argv[i]);
					++i;
					}
				}
			else if(strcasecmp(argv[i]+1,"palette")==0)
				{
				++i;
				if(i<argc)
					argColorMapName=argv[i];
				else
					std::cerr<<"Missing palette file name after -palette"<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"load")==0)
				{
				++i;
				if(i<argc)
					{
					/* Load an element file later: */
					loadFileNames.push_back(argv[i]);
					}
				else
					std::cerr<<"Missing element file name after -load"<<std::endl;
				}
			}
		else
			{
			/* Read the meta-input file of the given name: */
			Misc::File inputFile(argv[i],"rt");

			/* Parse the meta-input file: */
			int nextChar=fgetc(inputFile.getFilePtr());

			/* Read the module class name: */
			moduleClassName=readToken(inputFile,nextChar);

			/* Read the data set arguments: */
			dataSetArgs.clear();
			while(true)
				{
				/* Read the next argument: */
				std::string arg=readToken(inputFile,nextChar);
				if(arg=="")
					break;

				/* Store the argument in the list: */
				dataSetArgs.push_back(arg);
				}
			}
		}

	/* Check if a module class name and data set arguments were provided: */
	if(moduleClassName=="")
		Misc::throwStdErr("VirtualATR::VirtualATR: no visualization module class name provided");
	if(dataSetArgs.empty())
		Misc::throwStdErr("VirtualATR::VirtualATR: no data set arguments provided");

	/* Load a visualization module and a data set: */
	try
		{
		std::cout << "Before\n";
		/* Load the appropriate visualization module: */
		module=moduleManager.loadClass(moduleClassName.c_str());
		std::cout << "After\n";

		/* Load a data set: */
		Misc::Timer t;
		Comm::MulticastPipe* pipe=Vrui::openPipe(); // Implicit synchronization point
		dataSet=module->load(dataSetArgs,pipe);
		delete pipe; // Implicit synchronization point
		t.elapse();
		if(Vrui::isMaster())
			std::cout<<"Time to load data set: "<<t.getTime()*1000.0<<" ms"<<std::endl;
		}
	catch(std::runtime_error err)
		{
		Misc::throwStdErr("VirtualATR::VirtualATR: Could not load data set due to exception %s",err.what());
		}
	
       	/* Create a variable manager: */
	variableManager=new VariableManager(dataSet,argColorMapName);

	/* Determine the color to render the data set: */
	for(int i=0;i<3;++i)
		dataSetRenderColor[i]=1.0f-Vrui::getBackgroundColor()[i];
	dataSetRenderColor[3]=0.2f;

	/* Create a data set renderer: */
	dataSetRenderer=module->getRenderer(dataSet);

	/* Get the data set's coordinate transformer: */
	coordinateTransformer=dataSet->getCoordinateTransformer();

	/* Create cutting planes: */
	numberOfCuttingPlanes=6;
	cuttingPlanes=new CuttingPlane[numberOfCuttingPlanes];
	for(size_t i=0;i<numberOfCuttingPlanes;++i)
		{
		cuttingPlanes[i].allocated=false;
		cuttingPlanes[i].active=false;
		}

	/* Create the element list: */
	elementList=new ElementList(Vrui::getWidgetManager());

	/* Load all element files listed on the command line: */
	for(std::vector<const char*>::const_iterator lfnIt=loadFileNames.begin();lfnIt!=loadFileNames.end();++lfnIt)
		{
		/* Determine the type of the element file: */
		if(Misc::hasCaseExtension(*lfnIt,".asciielem"))
			{
			/* Load an ASCII elements file: */
			loadElements(*lfnIt,true);
			}
		else if(Misc::hasCaseExtension(*lfnIt,".binelem"))
			{
			/* Load a binary elements file: */
			loadElements(*lfnIt,false);
			}
		}

	/* Create the user interface: */
	mainMenu = createMainMenu();
	Vrui::setMainMenu(mainMenu);
	renderDialog = createRenderDialog();
	osccDialog = createOSCCDialog();

	/* Initialize Vrui navigation transformation: */
//	centerDisplayCallback(0);
} // end VirtualATR()

/*
 * ~VirtualATR - Destructor for VirtualATR class.
 */
VirtualATR::~VirtualATR(void) {
	/* Delete the user interface: */
	delete mainMenu;
	delete renderDialog;
	delete osccDialog;
} // end ~VirtualATR()

/*******************************
 Methods of class VirtualATR:
 *******************************/

/*
 * centerDisplayCallback
 *
 * parameter callbackData - Misc::CallbackData *
 */
void VirtualATR::centerDisplayCallback(Misc::CallbackData * callbackData) {
	/* Center the Sphere in the available display space, but do not scale it: */
	Vrui::NavTransform nav = Vrui::NavTransform::identity;
	nav *= Vrui::NavTransform::translateFromOriginTo(Vrui::getDisplayCenter());
	nav *= Vrui::NavTransform::scale(Vrui::Scalar(8) * Vrui::getInchFactor()
			/ Vrui::Scalar(1));
	Vrui::setNavigationTransformation(nav);
} // end centerDisplayCallback()

/*
 * changeAlgorithmCallback
 * parameter cbData - GLMotif::RadioBox::ValueChangedCallbackData *
 */
void VirtualATR::changeAlgorithmCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	/* Set the new algorithm: */
	algorithm=cbData->radioBox->getToggleIndex(cbData->newSelectedToggle);
	} // end changeAlgorithmCallback()


/*
 * changeAnalysisToolsCallback
 *
 * parameter callbackData - GLMotif::RadioBox::ValueChangedCallbackData *
 */
void VirtualATR::changeAnalysisToolsCallback(
		GLMotif::RadioBox::ValueChangedCallbackData * callbackData) {
	/* Set the new analysis tool: */
	analysisTool = callbackData->radioBox->getToggleIndex(
			callbackData->newSelectedToggle);
} // end changeAnalysisToolsCallback()

/*
 * changeRenderingModeCallback
 * parameter cbData - GLMotif::RadioBox::ValueChangedCallbackData *
 */
void VirtualATR::changeRenderingModeCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	/* Set the new rendering mode: */
	dataSetRenderer->setRenderingMode(cbData->radioBox->getToggleIndex(cbData->newSelectedToggle));
	} // end changeRenderingModeCallback()

/*
 * changeScalarVariableCallback
 * parameter cbData - GLMotif::RadioBox::ValueChangedCallbackData *
 */
void VirtualATR::changeScalarVariableCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	if(!inLoadPalette)
		{
		/* Set the new scalar variable: */
		variableManager->setCurrentScalarVariable(cbData->radioBox->getToggleIndex(cbData->newSelectedToggle));
		}
	} // end changeScalarVariableCallback()

/*
 * changeVectorVariableCallback
 * parameter cbData - GLMotif::RadioBox::ValueChangedCallbackData *
 */
void VirtualATR::changeVectorVariableCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	/* Set the new vector variable: */
	variableManager->setCurrentVectorVariable(cbData->radioBox->getToggleIndex(cbData->newSelectedToggle));
	} // end changeVectorVariableCallback()

/*
 * clearElementsCallback
 * parameter cbData - Misc::CallbackData *
 */
void VirtualATR::clearElementsCallback(Misc::CallbackData * cbData)
	{
	/* Delete all finished visualization elements: */
	elementList->clear();
	} // end clearElementsCallback()

/*
 * createAlgorithmsMenu
 * return - GLMotif::Popup *
 */
GLMotif::Popup * VirtualATR::createAlgorithmsMenu(void)
	{
	GLMotif::Popup* algorithmsMenuPopup=new GLMotif::Popup("AlgorithmsMenuPopup",Vrui::getWidgetManager());

	GLMotif::RadioBox* algorithms=new GLMotif::RadioBox("Algorithms",algorithmsMenuPopup,false);
	algorithms->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);

	/* Add the cutting plane algorithm: */
	int algorithmIndex=0;
	algorithms->addToggle("Cutting Plane");
	++algorithmIndex;

	if(variableManager->getNumScalarVariables()>0)
		{
		/* Add the scalar evaluator algorithm: */
		algorithms->addToggle("Evaluate Scalars");
		++algorithmIndex;

		/* Add scalar algorithms: */
		firstScalarAlgorithmIndex=algorithmIndex;
		for(int i=0;i<module->getNumScalarAlgorithms();++i)
			{
			algorithms->addToggle(module->getScalarAlgorithmName(i));
			++algorithmIndex;
			}
		}

	if(variableManager->getNumVectorVariables()>0)
		{
		/* Add the vector evaluator algorithm: */
		algorithms->addToggle("Evaluate Vectors");
		++algorithmIndex;

		/* Add vector algorithms: */
		firstVectorAlgorithmIndex=algorithmIndex;
		for(int i=0;i<module->getNumVectorAlgorithms();++i)
			{
			algorithms->addToggle(module->getVectorAlgorithmName(i));
			++algorithmIndex;
			}
		}

	algorithms->setSelectedToggle(algorithm);
	algorithms->getValueChangedCallbacks().add(this,&VirtualATR::changeAlgorithmCallback);

	algorithms->manageChild();

	return algorithmsMenuPopup;
	} // end createAlgorithmsMenu()

/*
 * createAnalysisToolsSubMenu
 *
 * return - GLMotif::Popup *
 */
GLMotif::Popup * VirtualATR::createAnalysisToolsSubMenu(void) {
	GLMotif::Popup * analysisToolsMenuPopup = new GLMotif::Popup(
			"analysisToolsMenuPopup", Vrui::getWidgetManager());
	GLMotif::RadioBox * analysisTools = new GLMotif::RadioBox("analysisTools",
			analysisToolsMenuPopup, false);
	analysisTools->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);

	/* Add the clipping plane analysisTool: */
	int analysisToolIndex = 0;
	analysisTools->addToggle("Clipping Plane");
	++analysisToolIndex;

	analysisTools->setSelectedToggle(analysisTool);
	analysisTools->getValueChangedCallbacks().add(this,
			&VirtualATR::changeAnalysisToolsCallback);

	analysisTools->manageChild();

	return analysisToolsMenuPopup;
} // end createAnalysisToolsSubMenu()

/*
 * createColorMenu
 * return - GLMotif::Popup *
 */
GLMotif::Popup * VirtualATR::createColorMenu(void)
	{
	GLMotif::Popup* colorMenuPopup=new GLMotif::Popup("ColorMenuPopup",Vrui::getWidgetManager());

	/* Create the color menu and add entries for all standard palettes: */
	GLMotif::SubMenu* colorMenu=new GLMotif::SubMenu("ColorMenu",colorMenuPopup,false);

	GLMotif::CascadeButton* standardLuminancePalettesCascade=new GLMotif::CascadeButton("StandardLuminancePalettesCascade",colorMenu,"Create Luminance Palette");
	standardLuminancePalettesCascade->setPopup(createStandardLuminancePalettesMenu());

	GLMotif::CascadeButton* standardSaturationPalettesCascade=new GLMotif::CascadeButton("StandardSaturationPalettesCascade",colorMenu,"Create Saturation Palette");
	standardSaturationPalettesCascade->setPopup(createStandardSaturationPalettesMenu());

	GLMotif::Button* loadPaletteButton=new GLMotif::Button("LoadPaletteButton",colorMenu,"Load Palette File");
	loadPaletteButton->getSelectCallbacks().add(this,&VirtualATR::loadPaletteCallback);

	GLMotif::ToggleButton* showColorBarToggle=new GLMotif::ToggleButton("ShowColorBarToggle",colorMenu,"Show Color Bar");
	showColorBarToggle->getValueChangedCallbacks().add(this,&VirtualATR::showColorBarCallback);

	GLMotif::ToggleButton* showPaletteEditorToggle=new GLMotif::ToggleButton("ShowPaletteEditorToggle",colorMenu,"Show Palette Editor");
	showPaletteEditorToggle->getValueChangedCallbacks().add(this,&VirtualATR::showPaletteEditorCallback);

	colorMenu->manageChild();

	return colorMenuPopup;
	} // end createColorMenu()

/*
 * createElementsMenu
 * return - GLMotif::Popup *
 */
GLMotif::Popup * VirtualATR::createElementsMenu(void)
	{
	GLMotif::Popup* elementsMenuPopup=new GLMotif::Popup("ElementsMenuPopup",Vrui::getWidgetManager());

	/* Create the elements menu: */
	GLMotif::SubMenu* elementsMenu=new GLMotif::SubMenu("ElementsMenu",elementsMenuPopup,false);

	showElementListToggle=new GLMotif::ToggleButton("ShowElementListToggle",elementsMenu,"Show Element List");
	showElementListToggle->getValueChangedCallbacks().add(this,&VirtualATR::showElementListCallback);

	GLMotif::Button* loadElementsButton=new GLMotif::Button("LoadElementsButton",elementsMenu,"Load Visualization Elements");
	loadElementsButton->getSelectCallbacks().add(this,&VirtualATR::loadElementsCallback);

	GLMotif::Button* saveElementsButton=new GLMotif::Button("SaveElementsButton",elementsMenu,"Save Visualization Elements");
	saveElementsButton->getSelectCallbacks().add(this,&VirtualATR::saveElementsCallback);

	new GLMotif::Separator("ClearElementsSeparator",elementsMenu,GLMotif::Separator::HORIZONTAL,0.0f,GLMotif::Separator::LOWERED);

	GLMotif::Button* clearElementsButton=new GLMotif::Button("ClearElementsButton",elementsMenu,"Clear Visualization Elements");
	clearElementsButton->getSelectCallbacks().add(this,&VirtualATR::clearElementsCallback);

	elementsMenu->manageChild();

	return elementsMenuPopup;
	} // end createElementsMenu()

/*
 * createMainMenu
 *
 * return - GLMotif::Popup *
 */
GLMotif::PopupMenu* VirtualATR::createMainMenu(void) {
	/* Create a top-level shell for the main menu: */
	GLMotif::PopupMenu * mainMenuPopup = new GLMotif::PopupMenu(
			"MainMenuPopup", Vrui::getWidgetManager());
	mainMenuPopup->setTitle("Interactive ATR");

	/* Create the actual menu inside the top-level shell: */
	GLMotif::Menu * mainMenu = new GLMotif::Menu("MainMenu", mainMenuPopup,
			false);

	/* Create a cascade button to show the "Rendering Modes" submenu: */
	GLMotif::CascadeButton * renderTogglesCascade = new GLMotif::CascadeButton(
			"RenderTogglesCascade", mainMenu, "Rendering Modes");
	renderTogglesCascade->setPopup(createRenderTogglesMenu());

	/* Create a cascade button to show the "Analysis Tools" submenu: */
	GLMotif::CascadeButton * analysisToolsCascade = new GLMotif::CascadeButton(
			"AnalysisToolsCascade", mainMenu, "Analysis Tools");
	analysisToolsCascade->setPopup(createAnalysisToolsSubMenu());

	/* Create a toggle button to show the render settings dialog: */
	GLMotif::ToggleButton* showRenderDialogToggle = new GLMotif::ToggleButton(
			"showRenderDialogToggle", mainMenu, "Show Render Dialog");
	showRenderDialogToggle->setToggle(false);
	showRenderDialogToggle->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	/* Create a toggle button to show the oscc settings dialog: */
	GLMotif::ToggleButton* showOSCCDialogToggle = new GLMotif::ToggleButton(
			"showOSCCDialogToggle", mainMenu, "Show OSCC Dialog");
	showOSCCDialogToggle->setToggle(false);
	showOSCCDialogToggle->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	GLMotif::CascadeButton* renderingModesCascade=new GLMotif::CascadeButton("RenderingModesCascade",mainMenu,"Rendering Modes");
	renderingModesCascade->setPopup(createRenderingModesMenu());

	if(variableManager->getNumScalarVariables()>0)
		{
		GLMotif::CascadeButton* scalarVariablesCascade=new GLMotif::CascadeButton("ScalarVariablesCascade",mainMenu,"Scalar Variables");
		scalarVariablesCascade->setPopup(createScalarVariablesMenu());
		}

	if(variableManager->getNumVectorVariables()>0)
		{
		GLMotif::CascadeButton* vectorVariablesCascade=new GLMotif::CascadeButton("VectorVariablesCascade",mainMenu,"Vector Variables");
		vectorVariablesCascade->setPopup(createVectorVariablesMenu());
		}

	GLMotif::CascadeButton* algorithmsCascade=new GLMotif::CascadeButton("AlgorithmsCascade",mainMenu,"Algorithms");
	algorithmsCascade->setPopup(createAlgorithmsMenu());

	GLMotif::CascadeButton* elementsCascade=new GLMotif::CascadeButton("ElementsCascade",mainMenu,"Elements");
	elementsCascade->setPopup(createElementsMenu());

	GLMotif::CascadeButton* colorCascade=new GLMotif::CascadeButton("ColorCascade",mainMenu,"Color Maps");
	colorCascade->setPopup(createColorMenu());

	/* Create a button to reset the navigation coordinates to the default (showing the entire Sphere): */
	GLMotif::Button * centerDisplayButton = new GLMotif::Button(
			"CenterDisplayButton", mainMenu, "Center Display");
	centerDisplayButton->getSelectCallbacks().add(this,
			&VirtualATR::centerDisplayCallback);

	/* Calculate the main menu's proper layout: */
	mainMenu->manageChild();

	/* Return the created top-level shell: */
	return mainMenuPopup;
} // end createMainMenu()

/*
 * createRenderDialog
 *
 * return - GLMotif::PopupWindow *
 */
GLMotif::PopupWindow * VirtualATR::createRenderDialog(void) {
	const GLMotif::StyleSheet& ss = *Vrui::getWidgetManager()->getStyleSheet();

	GLMotif::PopupWindow* renderDialogPopup = new GLMotif::PopupWindow(
			"RenderDialogPopup", Vrui::getWidgetManager(), "Display Settings");
	renderDialogPopup->setResizableFlags(true, false);

	GLMotif::RowColumn* rowColumn = new GLMotif::RowColumn("RowColumn",
			renderDialogPopup, false);
	rowColumn->setOrientation(GLMotif::RowColumn::VERTICAL);
	rowColumn->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	rowColumn->setNumMinorWidgets(2);

	new GLMotif::Label("ObjectTransparencyLabel", rowColumn,
			"Object Transparency");

	GLMotif::Slider* surfaceTransparencySlider = new GLMotif::Slider(
			"SurfaceTransparencySlider", rowColumn,
			GLMotif::Slider::HORIZONTAL, ss.fontHeight * 5.0f);
	surfaceTransparencySlider->setValueRange(0.0, 1.0, 0.001);
	surfaceTransparencySlider->getValueChangedCallbacks().add(this,
			&VirtualATR::sliderCallback);

	new GLMotif::Label("GridTransparencyLabel", rowColumn, "Grid Transparency");

	GLMotif::Slider* gridTransparencySlider = new GLMotif::Slider(
			"GridTransparencySlider", rowColumn, GLMotif::Slider::HORIZONTAL,
			ss.fontHeight * 5.0f);
	gridTransparencySlider->setValueRange(0.0, 1.0, 0.001);
	gridTransparencySlider->setValue(0.1);
	gridTransparencySlider->getValueChangedCallbacks().add(this,
			&VirtualATR::sliderCallback);

	showVesselToggleRD = new GLMotif::ToggleButton("showVesselToggle",
			rowColumn, "Show Vessel");
	showVesselToggleRD->setBorderWidth(0.0f);
	showVesselToggleRD->setMarginWidth(0.0f);
	showVesselToggleRD->setHAlignment(GLFont::Left);
	showVesselToggleRD->setToggle(true);
	showVesselToggleRD->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	showClear_VesselToggleRD = new GLMotif::ToggleButton("showClear_VesselToggle",
			rowColumn, "Show Clear Vessel");
	showClear_VesselToggleRD->setBorderWidth(0.0f);
	showClear_VesselToggleRD->setMarginWidth(0.0f);
	showClear_VesselToggleRD->setHAlignment(GLFont::Left);
	showClear_VesselToggleRD->setToggle(false);
	showClear_VesselToggleRD->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	wireframeToggleRD = new GLMotif::ToggleButton("wireframeToggle", rowColumn,
			"Wireframe");
	wireframeToggleRD->setBorderWidth(0.0f);
	wireframeToggleRD->setMarginWidth(0.0f);
	wireframeToggleRD->setHAlignment(GLFont::Left);
	wireframeToggleRD->setToggle(false);
	wireframeToggleRD->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	rowColumn->manageChild();

	return renderDialogPopup;
} // end createRenderDialog()

/*
 * createOSCCDialog
 *
 * return - GLMotif::PopupWindow *
 */
GLMotif::PopupWindow * VirtualATR::createOSCCDialog(void) {
	const GLMotif::StyleSheet& ss = *Vrui::getWidgetManager()->getStyleSheet();

	GLMotif::PopupWindow* osccDialogPopup = new GLMotif::PopupWindow(
			"OSCCDialogPopup", Vrui::getWidgetManager(), "OSCC Display Settings");
	osccDialogPopup->setResizableFlags(true, false);

	GLMotif::RowColumn* rowColumn = new GLMotif::RowColumn("RowColumn",
			osccDialogPopup, false);
	rowColumn->setOrientation(GLMotif::RowColumn::VERTICAL);
	rowColumn->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	rowColumn->setNumMinorWidgets(2);

	/* 
	 * OSCC_#: # represents angle at which cylinders are rotated 
	 * from 0 degrees. One number means all cylinders are at same 
	 * rotation; multiple numbers mean NW quadrant, then NE 
	 * quadrant, then SE quadrant, then SW quadrant (NOTE: 
	 * decimals are represented by underscores. EX 51.8 = 51_8). 
	 */

	showOSCC_5ToggleOSCCD = new GLMotif::ToggleButton("showOSCC_5Toggle",
			rowColumn, "Show all at 5 deg");
	showOSCC_5ToggleOSCCD->setBorderWidth(0.0f);
	showOSCC_5ToggleOSCCD->setMarginWidth(0.0f);
	showOSCC_5ToggleOSCCD->setHAlignment(GLFont::Left);
	showOSCC_5ToggleOSCCD->setToggle(false);
	showOSCC_5ToggleOSCCD->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	showOSCC_51_8ToggleOSCCD = new GLMotif::ToggleButton("showOSCC_51_8Toggle",
			rowColumn, "Show all at 51.8 deg");
	showOSCC_51_8ToggleOSCCD->setBorderWidth(0.0f);
	showOSCC_51_8ToggleOSCCD->setMarginWidth(0.0f);
	showOSCC_51_8ToggleOSCCD->setHAlignment(GLFont::Left);
	showOSCC_51_8ToggleOSCCD->setToggle(true);
	showOSCC_51_8ToggleOSCCD->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	showOSCC_85ToggleOSCCD = new GLMotif::ToggleButton("showOSCC_85Toggle",
			rowColumn, "Show all at 85 deg");
	showOSCC_85ToggleOSCCD->setBorderWidth(0.0f);
	showOSCC_85ToggleOSCCD->setMarginWidth(0.0f);
	showOSCC_85ToggleOSCCD->setHAlignment(GLFont::Left);
	showOSCC_85ToggleOSCCD->setToggle(false);
	showOSCC_85ToggleOSCCD->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	showOSCC_120ToggleOSCCD = new GLMotif::ToggleButton("showOSCC_120Toggle",
			rowColumn, "Show all at 120 deg");
	showOSCC_120ToggleOSCCD->setBorderWidth(0.0f);
	showOSCC_120ToggleOSCCD->setMarginWidth(0.0f);
	showOSCC_120ToggleOSCCD->setHAlignment(GLFont::Left);
	showOSCC_120ToggleOSCCD->setToggle(false);
	showOSCC_120ToggleOSCCD->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	showOSCC_155ToggleOSCCD = new GLMotif::ToggleButton("showOSCC_155Toggle",
			rowColumn, "Show all at 155 deg");
	showOSCC_155ToggleOSCCD->setBorderWidth(0.0f);
	showOSCC_155ToggleOSCCD->setMarginWidth(0.0f);
	showOSCC_155ToggleOSCCD->setHAlignment(GLFont::Left);
	showOSCC_155ToggleOSCCD->setToggle(false);
	showOSCC_155ToggleOSCCD->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	showOSCC_30_85_85_85ToggleOSCCD = new GLMotif::ToggleButton("showOSCC_30_85_85_85Toggle",
			rowColumn, "Show NW30, NE85, SE85, SW85 deg");
	showOSCC_30_85_85_85ToggleOSCCD->setBorderWidth(0.0f);
	showOSCC_30_85_85_85ToggleOSCCD->setMarginWidth(0.0f);
	showOSCC_30_85_85_85ToggleOSCCD->setHAlignment(GLFont::Left);
	showOSCC_30_85_85_85ToggleOSCCD->setToggle(false);
	showOSCC_30_85_85_85ToggleOSCCD->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	showOSCC_40_85_85_85ToggleOSCCD = new GLMotif::ToggleButton("showOSCC_40_85_85_85Toggle",
			rowColumn, "Show NW40, NE85, SE85, SW85 deg");
	showOSCC_40_85_85_85ToggleOSCCD->setBorderWidth(0.0f);
	showOSCC_40_85_85_85ToggleOSCCD->setMarginWidth(0.0f);
	showOSCC_40_85_85_85ToggleOSCCD->setHAlignment(GLFont::Left);
	showOSCC_40_85_85_85ToggleOSCCD->setToggle(false);
	showOSCC_40_85_85_85ToggleOSCCD->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	showOSCC_50_85_85_85ToggleOSCCD = new GLMotif::ToggleButton("showOSCC_50_85_85_85Toggle",
			rowColumn, "Show NW50, NE85, SE85, SW85 deg");
	showOSCC_50_85_85_85ToggleOSCCD->setBorderWidth(0.0f);
	showOSCC_50_85_85_85ToggleOSCCD->setMarginWidth(0.0f);
	showOSCC_50_85_85_85ToggleOSCCD->setHAlignment(GLFont::Left);
	showOSCC_50_85_85_85ToggleOSCCD->setToggle(false);
	showOSCC_50_85_85_85ToggleOSCCD->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	showOSCC_60_85_85_85ToggleOSCCD = new GLMotif::ToggleButton("showOSCC_60_85_85_85Toggle",
			rowColumn, "Show NW60, NE85, SE85, SW85 deg");
	showOSCC_60_85_85_85ToggleOSCCD->setBorderWidth(0.0f);
	showOSCC_60_85_85_85ToggleOSCCD->setMarginWidth(0.0f);
	showOSCC_60_85_85_85ToggleOSCCD->setHAlignment(GLFont::Left);
	showOSCC_60_85_85_85ToggleOSCCD->setToggle(false);
	showOSCC_60_85_85_85ToggleOSCCD->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	showOSCC_70_85_85_85ToggleOSCCD = new GLMotif::ToggleButton("showOSCC_70_85_85_85Toggle",
			rowColumn, "Show NW70, NE85, SE85, SW85 deg");
	showOSCC_70_85_85_85ToggleOSCCD->setBorderWidth(0.0f);
	showOSCC_70_85_85_85ToggleOSCCD->setMarginWidth(0.0f);
	showOSCC_70_85_85_85ToggleOSCCD->setHAlignment(GLFont::Left);
	showOSCC_70_85_85_85ToggleOSCCD->setToggle(false);
	showOSCC_70_85_85_85ToggleOSCCD->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	showOSCC_80_85_85_85ToggleOSCCD = new GLMotif::ToggleButton("showOSCC_80_85_85_85Toggle",
			rowColumn, "Show NW80, NE85, SE85, SW85 deg");
	showOSCC_80_85_85_85ToggleOSCCD->setBorderWidth(0.0f);
	showOSCC_80_85_85_85ToggleOSCCD->setMarginWidth(0.0f);
	showOSCC_80_85_85_85ToggleOSCCD->setHAlignment(GLFont::Left);
	showOSCC_80_85_85_85ToggleOSCCD->setToggle(false);
	showOSCC_80_85_85_85ToggleOSCCD->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	showOSCC_90_85_85_85ToggleOSCCD = new GLMotif::ToggleButton("showOSCC_90_85_85_85Toggle",
			rowColumn, "Show NW90, NE85, SE85, SW85 deg");
	showOSCC_90_85_85_85ToggleOSCCD->setBorderWidth(0.0f);
	showOSCC_90_85_85_85ToggleOSCCD->setMarginWidth(0.0f);
	showOSCC_90_85_85_85ToggleOSCCD->setHAlignment(GLFont::Left);
	showOSCC_90_85_85_85ToggleOSCCD->setToggle(false);
	showOSCC_90_85_85_85ToggleOSCCD->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	showOSCC_100_85_85_85ToggleOSCCD = new GLMotif::ToggleButton("showOSCC_100_85_85_85Toggle",
			rowColumn, "Show NW100, NE85, SE85, SW85 deg");
	showOSCC_100_85_85_85ToggleOSCCD->setBorderWidth(0.0f);
	showOSCC_100_85_85_85ToggleOSCCD->setMarginWidth(0.0f);
	showOSCC_100_85_85_85ToggleOSCCD->setHAlignment(GLFont::Left);
	showOSCC_100_85_85_85ToggleOSCCD->setToggle(false);
	showOSCC_100_85_85_85ToggleOSCCD->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	showOSCC_110_85_85_85ToggleOSCCD = new GLMotif::ToggleButton("showOSCC_110_85_85_85Toggle",
			rowColumn, "Show NW110, NE85, SE85, SW85 deg");
	showOSCC_110_85_85_85ToggleOSCCD->setBorderWidth(0.0f);
	showOSCC_110_85_85_85ToggleOSCCD->setMarginWidth(0.0f);
	showOSCC_110_85_85_85ToggleOSCCD->setHAlignment(GLFont::Left);
	showOSCC_110_85_85_85ToggleOSCCD->setToggle(false);
	showOSCC_110_85_85_85ToggleOSCCD->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	showOSCC_120_85_85_85ToggleOSCCD = new GLMotif::ToggleButton("showOSCC_120_85_85_85Toggle",
			rowColumn, "Show NW120, NE85, SE85, SW85 deg");
	showOSCC_120_85_85_85ToggleOSCCD->setBorderWidth(0.0f);
	showOSCC_120_85_85_85ToggleOSCCD->setMarginWidth(0.0f);
	showOSCC_120_85_85_85ToggleOSCCD->setHAlignment(GLFont::Left);
	showOSCC_120_85_85_85ToggleOSCCD->setToggle(false);
	showOSCC_120_85_85_85ToggleOSCCD->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	rowColumn->manageChild();

	return osccDialogPopup;
} // end createOSCCDialog()

/*
 * createOSCCTogglesMenu
 *
 * return - GLMotif::Popup *
 */
GLMotif::Popup * VirtualATR::createOSCCTogglesMenu(void) {
	/* Create the submenu's top-level shell: */
	GLMotif::Popup * osccTogglesMenuPopup = new GLMotif::Popup(
			"OSCCTogglesMenuPopup", Vrui::getWidgetManager());

	/* Create the array of oscc toggle buttons inside the top-level shell: */
	GLMotif::SubMenu * osccTogglesMenu = new GLMotif::SubMenu(
			"OSCCTogglesMenu", osccTogglesMenuPopup, false);
	
	/* Create a toggle button to render OSCC at 5: */
	showOSCC_5Toggle = new GLMotif::ToggleButton("showOSCC_5Toggle",
			osccTogglesMenu, "Show all at 5");
	showOSCC_5Toggle->setToggle(false);
	showOSCC_5Toggle->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	/* Create a toggle button to render OSCC at 51.8: */
	showOSCC_51_8Toggle = new GLMotif::ToggleButton("showOSCC_51_8Toggle",
			osccTogglesMenu, "Show all at 51.8");
	showOSCC_51_8Toggle->setToggle(true);
	showOSCC_51_8Toggle->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	/* Create a toggle button to render OSCC at 85: */
	showOSCC_85Toggle = new GLMotif::ToggleButton("showOSCC_85Toggle",
			osccTogglesMenu, "Show all at 85");
	showOSCC_85Toggle->setToggle(false);
	showOSCC_85Toggle->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	/* Create a toggle button to render OSCC at 120: */
	showOSCC_120Toggle = new GLMotif::ToggleButton("showOSCC_120Toggle",
			osccTogglesMenu, "Show all at 120");
	showOSCC_120Toggle->setToggle(false);
	showOSCC_120Toggle->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	/* Create a toggle button to render OSCC at 155: */
	showOSCC_155Toggle = new GLMotif::ToggleButton("showOSCC_155Toggle",
			osccTogglesMenu, "Show all at 155");
	showOSCC_155Toggle->setToggle(false);
	showOSCC_155Toggle->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	/* Create a toggle button to render OSCC at NW30 NE85 SE85 SW85: */
	showOSCC_30_85_85_85Toggle = new GLMotif::ToggleButton("showOSCC_30_85_85_85Toggle",
			osccTogglesMenu, "Show NW30, NE85, SE85, SW85 deg");
	showOSCC_30_85_85_85Toggle->setToggle(false);
	showOSCC_30_85_85_85Toggle->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	/* Create a toggle button to render OSCC at NW40 NE85 SE85 SW85: */
	showOSCC_40_85_85_85Toggle = new GLMotif::ToggleButton("showOSCC_40_85_85_85Toggle",
			osccTogglesMenu, "Show NW40, NE85, SE85, SW85 deg");
	showOSCC_40_85_85_85Toggle->setToggle(false);
	showOSCC_40_85_85_85Toggle->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	/* Create a toggle button to render OSCC at NW50 NE85 SE85 SW85: */
	showOSCC_50_85_85_85Toggle = new GLMotif::ToggleButton("showOSCC_50_85_85_85Toggle",
			osccTogglesMenu, "Show NW50, NE85, SE85, SW85 deg");
	showOSCC_50_85_85_85Toggle->setToggle(false);
	showOSCC_50_85_85_85Toggle->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	/* Create a toggle button to render OSCC at NW60 NE85 SE85 SW85: */
	showOSCC_60_85_85_85Toggle = new GLMotif::ToggleButton("showOSCC_60_85_85_85Toggle",
			osccTogglesMenu, "Show NW60, NE85, SE85, SW85 deg");
	showOSCC_60_85_85_85Toggle->setToggle(false);
	showOSCC_60_85_85_85Toggle->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	/* Create a toggle button to render OSCC at NW70 NE85 SE85 SW85: */
	showOSCC_70_85_85_85Toggle = new GLMotif::ToggleButton("showOSCC_70_85_85_85Toggle",
			osccTogglesMenu, "Show NW70, NE85, SE85, SW85 deg");
	showOSCC_70_85_85_85Toggle->setToggle(false);
	showOSCC_70_85_85_85Toggle->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	/* Create a toggle button to render OSCC at NW80 NE85 SE85 SW85: */
	showOSCC_80_85_85_85Toggle = new GLMotif::ToggleButton("showOSCC_80_85_85_85Toggle",
			osccTogglesMenu, "Show NW80, NE85, SE85, SW85 deg");
	showOSCC_80_85_85_85Toggle->setToggle(false);
	showOSCC_80_85_85_85Toggle->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	/* Create a toggle button to render OSCC at NW90 NE85 SE85 SW85: */
	showOSCC_90_85_85_85Toggle = new GLMotif::ToggleButton("showOSCC_90_85_85_85Toggle",
			osccTogglesMenu, "Show NW90, NE85, SE85, SW85 deg");
	showOSCC_90_85_85_85Toggle->setToggle(false);
	showOSCC_90_85_85_85Toggle->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	/* Create a toggle button to render OSCC at NW100 NE85 SE85 SW85: */
	showOSCC_100_85_85_85Toggle = new GLMotif::ToggleButton("showOSCC_100_85_85_85Toggle",
			osccTogglesMenu, "Show NW100, NE85, SE85, SW85 deg");
	showOSCC_100_85_85_85Toggle->setToggle(false);
	showOSCC_100_85_85_85Toggle->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	/* Create a toggle button to render OSCC at NW110 NE85 SE85 SW85: */
	showOSCC_110_85_85_85Toggle = new GLMotif::ToggleButton("showOSCC_110_85_85_85Toggle",
			osccTogglesMenu, "Show NW110, NE85, SE85, SW85 deg");
	showOSCC_110_85_85_85Toggle->setToggle(false);
	showOSCC_110_85_85_85Toggle->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	/* Create a toggle button to render OSCC at NW120 NE85 SE85 SW85: */
	showOSCC_120_85_85_85Toggle = new GLMotif::ToggleButton("showOSCC_120_85_85_85Toggle",
			osccTogglesMenu, "Show NW120, NE85, SE85, SW85 deg");
	showOSCC_120_85_85_85Toggle->setToggle(false);
	showOSCC_120_85_85_85Toggle->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	/* Calculate the submenu's proper layout: */
	osccTogglesMenu->manageChild();

	/* Return the created top-level shell: */
	return osccTogglesMenuPopup;
} // end createOSCCTogglesMenu

/*
 * createVesselTogglesMenu
 *
 * return - GLMotif::Popup *
 */
GLMotif::Popup * VirtualATR::createVesselTogglesMenu(void) {
	/* Create the submenu's top-level shell: */
	GLMotif::Popup * vesselTogglesMenuPopup = new GLMotif::Popup(
			"VesselTogglesMenuPopup", Vrui::getWidgetManager());

	/* Create the array of vessel toggle buttons inside the top-level shell: */
	GLMotif::SubMenu * vesselTogglesMenu = new GLMotif::SubMenu(
			"VesselTogglesMenu", vesselTogglesMenuPopup, false);

     	/* Create a toggle button to render Vessel: */
	showVesselToggle = new GLMotif::ToggleButton("showVesselToggle",
			vesselTogglesMenu, "Show Vessel");
	showVesselToggle->setToggle(true);
	showVesselToggle->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	/* Create a toggle button to render Clear Vessel: */
	showClear_VesselToggle = new GLMotif::ToggleButton("showClear_VesselToggle",
			vesselTogglesMenu, "Show Clear Vessel");
	showClear_VesselToggle->setToggle(false);
	showClear_VesselToggle->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	/* Calculate the submenu's proper layout: */
	vesselTogglesMenu->manageChild();

	/* Return the created top-level shell: */
	return vesselTogglesMenuPopup;
} // end createVesselTogglesMenu

/*
 * createRenderTogglesMenu
 *
 * return - GLMotif::Popup *
 */
GLMotif::Popup * VirtualATR::createRenderTogglesMenu(void) {
	/* Create the submenu's top-level shell: */
	GLMotif::Popup * renderTogglesMenuPopup = new GLMotif::Popup(
			"RenderTogglesMenuPopup", Vrui::getWidgetManager());

	/* Create the array of render toggle buttons inside the top-level shell: */
	GLMotif::SubMenu * renderTogglesMenu = new GLMotif::SubMenu(
			"RenderTogglesMenu", renderTogglesMenuPopup, false);
	
	/* Create a cascade button to show the "Vessel Models" submenu: */
	GLMotif::CascadeButton * vesselTogglesCascade = new GLMotif::CascadeButton(
			"VesselTogglesCascade", renderTogglesMenu, "Vessel Models");
	vesselTogglesCascade->setPopup(createVesselTogglesMenu());

	/* Create a cascade button to show the "OSCC Models" submenu: */
	GLMotif::CascadeButton * osccTogglesCascade = new GLMotif::CascadeButton(
			"OSCCTogglesCascade", renderTogglesMenu, "Control Cylinders Models");
	osccTogglesCascade->setPopup(createOSCCTogglesMenu());

	/* Create a toggle button to render Wireframe: */
	wireframeToggle = new GLMotif::ToggleButton("wireframeToggle",
			renderTogglesMenu, "Wireframe");
	wireframeToggle->setToggle(false);
	wireframeToggle->getValueChangedCallbacks().add(this,
			&VirtualATR::menuToggleSelectCallback);

	/* Calculate the submenu's proper layout: */
	renderTogglesMenu->manageChild();

	/* Return the created top-level shell: */
	return renderTogglesMenuPopup;
} // end createRenderTogglesMenu

/*
 * createRenderingModesMenu
 * return - GLMotif::Popup *
 */
GLMotif::Popup * VirtualATR::createRenderingModesMenu(void)
	{
	GLMotif::Popup* renderingModesMenuPopup=new GLMotif::Popup("RenderingModesMenuPopup",Vrui::getWidgetManager());

	GLMotif::RadioBox* renderingModes=new GLMotif::RadioBox("RenderingModes",renderingModesMenuPopup,false);
	renderingModes->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);

	int numRenderingModes=dataSetRenderer->getNumRenderingModes();
	for(int i=0;i<numRenderingModes;++i)
		renderingModes->addToggle(dataSetRenderer->getRenderingModeName(i));

	renderingModes->setSelectedToggle(dataSetRenderer->getRenderingMode());
	renderingModes->getValueChangedCallbacks().add(this,&VirtualATR::changeRenderingModeCallback);

	renderingModes->manageChild();

	return renderingModesMenuPopup;
	} // end createRenderingModesMenu()

/*
 * createScalarVariablesMenu
 * return - GLMotif::Popup *
 */
GLMotif::Popup * VirtualATR::createScalarVariablesMenu(void)
	{
	GLMotif::Popup* scalarVariablesMenuPopup=new GLMotif::Popup("ScalarVariablesMenuPopup",Vrui::getWidgetManager());

	GLMotif::RadioBox* scalarVariables=new GLMotif::RadioBox("ScalarVariables",scalarVariablesMenuPopup,false);
	scalarVariables->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);

	for(int i=0;i<variableManager->getNumScalarVariables();++i)
		scalarVariables->addToggle(variableManager->getScalarVariableName(i));

	scalarVariables->setSelectedToggle(variableManager->getCurrentScalarVariable());
	scalarVariables->getValueChangedCallbacks().add(this,&VirtualATR::changeScalarVariableCallback);

	scalarVariables->manageChild();

	return scalarVariablesMenuPopup;
	} // end createScalarVariablesMenu()

/*
 * createStandardLuminancePalettesMenu
 * return - GLMotif::Popup *
 */
GLMotif::Popup * VirtualATR::createStandardLuminancePalettesMenu(void)
	{
	GLMotif::Popup* standardLuminancePalettesMenuPopup=new GLMotif::Popup("StandardLuminancePalettesMenuPopup",Vrui::getWidgetManager());

	/* Create the palette creation menu and add entries for all standard palettes: */
	GLMotif::SubMenu* standardLuminancePalettes=new GLMotif::SubMenu("StandardLuminancePalettes",standardLuminancePalettesMenuPopup,false);

	standardLuminancePalettes->addEntry("Grey");
	standardLuminancePalettes->addEntry("Red");
	standardLuminancePalettes->addEntry("Yellow");
	standardLuminancePalettes->addEntry("Green");
	standardLuminancePalettes->addEntry("Cyan");
	standardLuminancePalettes->addEntry("Blue");
	standardLuminancePalettes->addEntry("Magenta");

	standardLuminancePalettes->getEntrySelectCallbacks().add(this,&VirtualATR::createStandardLuminancePaletteCallback);

	standardLuminancePalettes->manageChild();

	return standardLuminancePalettesMenuPopup;
	} // end createStandardLuminancePalettesMenu()

/*
 * createStandardLuminancePaletteCallback
 * parameter cbData - GLMotif::Menu::EntrySelectCallbackData *
 */
void VirtualATR::createStandardLuminancePaletteCallback(GLMotif::Menu::EntrySelectCallbackData* cbData)
	{
	if(!inLoadPalette)
		variableManager->createPalette(VariableManager::LUMINANCE_GREY+cbData->menu->getEntryIndex(cbData->selectedButton));
	} // end createStandardLuminancePaletteCallback()

/*
 * createStandardSaturationPalettesMenu
 * return - GLMotif::Popup *
 */
GLMotif::Popup * VirtualATR::createStandardSaturationPalettesMenu(void)
	{
	GLMotif::Popup* standardSaturationPalettesMenuPopup=new GLMotif::Popup("StandardSaturationPalettesMenuPopup",Vrui::getWidgetManager());

	/* Create the palette creation menu and add entries for all standard palettes: */
	GLMotif::SubMenu* standardSaturationPalettes=new GLMotif::SubMenu("StandardSaturationPalettes",standardSaturationPalettesMenuPopup,false);

	standardSaturationPalettes->addEntry("Red -> Cyan");
	standardSaturationPalettes->addEntry("Yellow -> Blue");
	standardSaturationPalettes->addEntry("Green -> Magenta");
	standardSaturationPalettes->addEntry("Cyan -> Red");
	standardSaturationPalettes->addEntry("Blue -> Yellow");
	standardSaturationPalettes->addEntry("Magenta -> Green");
	standardSaturationPalettes->addEntry("Rainbow");

	standardSaturationPalettes->getEntrySelectCallbacks().add(this,&VirtualATR::createStandardSaturationPaletteCallback);

	standardSaturationPalettes->manageChild();

	return standardSaturationPalettesMenuPopup;
	} // end createStandardSaturationPalettesMenu()

/*
 * createStandardSaturationPaletteCallback
 * parameter cbData - GLMotif::Menu::EntrySelectCallbackData *
 */
void VirtualATR::createStandardSaturationPaletteCallback(GLMotif::Menu::EntrySelectCallbackData* cbData)
	{
	if(!inLoadPalette)
		variableManager->createPalette(VariableManager::SATURATION_RED_CYAN+cbData->menu->getEntryIndex(cbData->selectedButton));
	} // end createStandardSaturationPaletteCallback()

/*
 * createVectorVariablesMenu
 * return - GLMotif::Popup *
 */
GLMotif::Popup * VirtualATR::createVectorVariablesMenu(void)
	{
	GLMotif::Popup* vectorVariablesMenuPopup=new GLMotif::Popup("VectorVariablesMenuPopup",Vrui::getWidgetManager());

	GLMotif::RadioBox* vectorVariables=new GLMotif::RadioBox("VectorVariables",vectorVariablesMenuPopup,false);
	vectorVariables->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);

	for(int i=0;i<variableManager->getNumVectorVariables();++i)
		vectorVariables->addToggle(variableManager->getVectorVariableName(i));

	vectorVariables->setSelectedToggle(variableManager->getCurrentVectorVariable());
	vectorVariables->getValueChangedCallbacks().add(this,&VirtualATR::changeVectorVariableCallback);

	vectorVariables->manageChild();

	return vectorVariablesMenuPopup;
	} // end createVectorVariablesMenu()
/*
 * display
 *
 * parameter glContextData - GLContextData &
 */
void VirtualATR::display(GLContextData & glContextData) const {

	/* Get context data item: */
	DataItem* dataItem = glContextData.retrieveDataItem<DataItem> (this);

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushAttrib(GL_TRANSFORM_BIT);
	glPushAttrib(GL_VIEWPORT_BIT);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();

	/* Highlight all locators: */
	for(BaseLocatorList::const_iterator blIt=baseLocators.begin();blIt!=baseLocators.end();++blIt)
		(*blIt)->highlightLocator(glContextData);

	/* Enable all cutting planes: */
	int numSupportedCuttingPlanes;
	glGetIntegerv(GL_MAX_CLIP_PLANES,&numSupportedCuttingPlanes);
	int cuttingPlaneIndex=0;
	for(size_t i=0;i<numberOfCuttingPlanes&&cuttingPlaneIndex<numSupportedCuttingPlanes;++i)
		if(cuttingPlanes[i].active)
			{
			/* Enable the cutting plane: */
			glEnable(GL_CLIP_PLANE0+cuttingPlaneIndex);
			GLdouble cuttingPlane[4];
			for(int j=0;j<3;++j)
				cuttingPlane[j]=cuttingPlanes[i].plane.getNormal()[j];
			cuttingPlane[3]=-cuttingPlanes[i].plane.getOffset();
			glClipPlane(GL_CLIP_PLANE0+cuttingPlaneIndex,cuttingPlane);

			/* Go to the next cutting plane: */
			++cuttingPlaneIndex;
			}

	/* Render all opaque visualization elements: */
	elementList->renderElements(glContextData,false);
	for(BaseLocatorList::const_iterator blIt=baseLocators.begin();blIt!=baseLocators.end();++blIt)
		(*blIt)->glRenderAction(glContextData);

	/* Render all transparent visualization elements: */
	elementList->renderElements(glContextData,true);
	for(BaseLocatorList::const_iterator blIt=baseLocators.begin();blIt!=baseLocators.end();++blIt)
		(*blIt)->glRenderActionTransparent(glContextData);

	/* Render the data set: */
	GLfloat lineWidth;
	glGetFloatv(GL_LINE_WIDTH,&lineWidth);
	if(lineWidth!=1.0f)
		glLineWidth(1.0f);
	glColor(dataSetRenderColor);
	dataSetRenderer->glRenderAction(glContextData);
	glLineWidth(lineWidth);

	atr->display(glContextData);

	/* Disable all cutting planes: */
	cuttingPlaneIndex=0;
	for(size_t i=0;i<numberOfCuttingPlanes&&cuttingPlaneIndex<numSupportedCuttingPlanes;++i)
		if(cuttingPlanes[i].active)
			{
			/* Disable the cutting plane: */
			glDisable(GL_CLIP_PLANE0+cuttingPlaneIndex);

			/* Go to the next cutting plane: */
			++cuttingPlaneIndex;
			}

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopClientAttrib();
	glPopAttrib();
	glPopAttrib();
	glPopAttrib();
} // end display()

/*
 * frame
 */
void VirtualATR::frame(void) {
	atr->frame();
} // end frame()

/*
 * getClippingPlanes
 *
 * return - ClippingPlane *
 */
ClippingPlane * VirtualATR::getClippingPlanes(void) {
	return clippingPlanes;
} // end getClippingPlanes()

/*
 * getNumberOfClippingPlanes
 *
 * return - int
 */
int VirtualATR::getNumberOfClippingPlanes(void) {
	return numberOfClippingPlanes;
} // end getNumberOfClippingPlanes()

/*
 * initContext
 *
 * parameter glContextData - GLContextData &
 */
void VirtualATR::initContext(GLContextData & glContextData) const {
	/* Create a new context data item: */
	DataItem* dataItem = new DataItem();

	glContextData.addDataItem(this, dataItem);
} // end initContext()

/*
 * menuToggleSelectCallback
 *
 * parameter elementFileName - const char *
 */
void VirtualATR::loadElements(const char* elementFileName,bool ascii)
	{
	/* Open a pipe for cluster communication: */
	Comm::MulticastPipe* pipe=Vrui::openPipe();

	if(pipe==0||pipe->isMaster())
		{
		/* Open the element file: */
		Misc::File elementFile(elementFileName,ascii?"r":"rb",ascii?Misc::File::DontCare:Misc::File::LittleEndian);

		/* Read all elements from the file: */
		while(true)
			{
			/* Read the next algorithm name: */
			unsigned int nameLength=0;
			char name[256];
			if(ascii)
				{
				/* Skip whitespace: */
				int nextChar;
				while((nextChar=fgetc(elementFile.getFilePtr()))!=EOF&&isspace(nextChar))
					;

				/* Read until end of line: */
				while(nextChar!='\n'&&nextChar!=EOF)
					{
					name[nameLength]=char(nextChar);
					++nameLength;
					nextChar=fgetc(elementFile.getFilePtr());
					}

				if(nextChar==EOF) // Check for end-of-file indicator
					{
					/* Tell the slaves to bail out: */
					if(pipe!=0)
						pipe->write<unsigned int>(0);

					/* Bail out: */
					break;
					}

				/* Remove trailing whitespace: */
				while(isspace(name[nameLength-1]))
					--nameLength;
				}
			else
				{
				nameLength=elementFile.read<unsigned int>();
				if(nameLength==0) // Check for end-of-file indicator
					{
					/* Tell the slaves to bail out: */
					if(pipe!=0)
						pipe->write<unsigned int>(0);

					/* Bail out: */
					break;
					}
				elementFile.read(name,nameLength);
				}
			name[nameLength]='\0';

			if(pipe!=0)
				{
				/* Send the algorithm name to the slaves: */
				pipe->write<unsigned int>(nameLength);
				pipe->write(name,nameLength);
				}

			/* Create an extractor for the given name: */
			Algorithm* algorithm=0;
			for(int i=0;algorithm==0&&i<module->getNumScalarAlgorithms();++i)
				if(strcmp(name,module->getScalarAlgorithmName(i))==0)
					algorithm=module->getScalarAlgorithm(i,variableManager,Vrui::openPipe());
			for(int i=0;algorithm==0&&i<module->getNumVectorAlgorithms();++i)
				if(strcmp(name,module->getVectorAlgorithmName(i))==0)
					algorithm=module->getVectorAlgorithm(i,variableManager,Vrui::openPipe());

			/* Extract an element using the given extractor: */
			if(algorithm!=0)
				{
				std::cout<<"Creating "<<name<<"..."<<std::flush;
				Misc::Timer extractionTimer;

				/* Read the element's extraction parameters from the file: */
				Parameters* parameters=algorithm->cloneParameters();
				parameters->read(elementFile,ascii,variableManager);

				if(pipe!=0)
					{
					/* Send the extraction parameters to the slaves: */
					parameters->write(*pipe,variableManager);
					pipe->finishMessage();
					}

				/* Extract the element: */
				Element* element=algorithm->createElement(parameters);

				/* Store the element: */
				elementList->addElement(element,name);

				/* Destroy the extractor: */
				delete algorithm;

				extractionTimer.elapse();
				std::cout<<" done in "<<extractionTimer.getTime()*1000.0<<" ms"<<std::endl;
				}
			}
		}
	else
		{
		/* Receive all visualization elements from the master: */
		while(true)
			{
			/* Receive the algorithm name from the master: */
			unsigned int nameLength=pipe->read<unsigned int>();
			if(nameLength==0) // Check for end-of-file indicator
				break;
			char name[256];
			pipe->read(name,nameLength);
			name[nameLength]=0;

			/* Create an extractor for the given name: */
			Algorithm* algorithm=0;
			for(int i=0;algorithm==0&&i<module->getNumScalarAlgorithms();++i)
				if(strcmp(name,module->getScalarAlgorithmName(i))==0)
					algorithm=module->getScalarAlgorithm(i,variableManager,Vrui::openPipe());
			for(int i=0;algorithm==0&&i<module->getNumVectorAlgorithms();++i)
				if(strcmp(name,module->getVectorAlgorithmName(i))==0)
					algorithm=module->getVectorAlgorithm(i,variableManager,Vrui::openPipe());

			/* Extract an element using the given extractor: */
			if(algorithm!=0)
				{
				/* Receive the extraction parameters: */
				Parameters* parameters=algorithm->cloneParameters();
				parameters->read(*pipe,variableManager);

				/* Receive the element: */
				Element* element=algorithm->startSlaveElement(parameters);
				algorithm->continueSlaveElement();

				/* Store the element: */
				elementList->addElement(element,name);

				/* Destroy the extractor: */
				delete algorithm;
				}
			}
		}

	if(pipe!=0)
		{
		/* Close the communication pipe: */
		delete pipe;
		}
	}

/*
 * loadElementsCallback
 * parameter cbData - Misc::CallbackData *
 */
void VirtualATR::loadElementsCallback(Misc::CallbackData * cbData)
	{
	if(!inLoadElements)
		{
		/* Create a file selection dialog to select an element file: */
		GLMotif::FileSelectionDialog* fsDialog=new GLMotif::FileSelectionDialog(Vrui::getWidgetManager(),"Load Visualization Elements...",0,".asciielem;.binelem",Vrui::openPipe());
		fsDialog->getOKCallbacks().add(this,&VirtualATR::loadElementsOKCallback);
		fsDialog->getCancelCallbacks().add(this,&VirtualATR::loadElementsCancelCallback);
		Vrui::popupPrimaryWidget(fsDialog,Vrui::getNavigationTransformation().transform(Vrui::getDisplayCenter()));
		inLoadElements=true;
		}
	} // end loadElementsCallback()


/*
 * loadElementsCancelCallback
 * parameter cbData - GLMotif::FileSelectionDialog::CancelCallbackData *
 */
void VirtualATR::loadElementsCancelCallback(GLMotif::FileSelectionDialog::CancelCallbackData* cbData)
	{
	/* Destroy the file selection dialog: */
	Vrui::getWidgetManager()->deleteWidget(cbData->fileSelectionDialog);
	inLoadElements=false;
	} // end loadElementsCancelCallback()

/*
 * loadElementsOKCallback
 * parameter cbData - GLMotif::FileSelectionDialog::OKCallbackData *
 */
void VirtualATR::loadElementsOKCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData)
	{
	try
		{
		/* Determine the type of the element file: */
		if(Misc::hasCaseExtension(cbData->selectedFileName.c_str(),".asciielem"))
			{
			/* Load the ASCII elements file: */
			loadElements(cbData->selectedFileName.c_str(),true);
			}
		else if(Misc::hasCaseExtension(cbData->selectedFileName.c_str(),".binelem"))
			{
			/* Load the binary elements file: */
			loadElements(cbData->selectedFileName.c_str(),false);
			}
		}
	catch(std::runtime_error err)
		{
		std::cerr<<"Caught exception "<<err.what()<<" while loading element file"<<std::endl;
		}

	/* Destroy the file selection dialog: */
	Vrui::getWidgetManager()->deleteWidget(cbData->fileSelectionDialog);
	inLoadElements=false;
	} // end loadElementsOKCallback()

/*
 * loadPaletteCallback
 * parameter cbData - Misc::CallbackData *
 */
void VirtualATR::loadPaletteCallback(Misc::CallbackData * cbData)
	{
	if(!inLoadPalette)
		{
		/* Create a file selection dialog to select a palette file: */
		GLMotif::FileSelectionDialog* fsDialog=new GLMotif::FileSelectionDialog(Vrui::getWidgetManager(),"Load Palette File...",0,".pal",Vrui::openPipe());
		fsDialog->getOKCallbacks().add(this,&VirtualATR::loadPaletteOKCallback);
		fsDialog->getCancelCallbacks().add(this,&VirtualATR::loadPaletteCancelCallback);
		Vrui::popupPrimaryWidget(fsDialog,Vrui::getNavigationTransformation().transform(Vrui::getDisplayCenter()));
		inLoadPalette=true;
		}
	} // end loadPaletteCallback()

/*
 * loadPaletteCancelCallback
 * parameter cbData - GLMotif::FileSelectionDialog::CancelCallbackData *
 */
void VirtualATR::loadPaletteCancelCallback(GLMotif::FileSelectionDialog::CancelCallbackData* cbData)
	{
	/* Destroy the file selection dialog: */
	Vrui::getWidgetManager()->deleteWidget(cbData->fileSelectionDialog);
	inLoadPalette=false;
	} // end loadPaletteCancelCallback()

/*
 * loadPaletteOKCallback
 * parameter cbData - GLMotif::FileSelectionDialog::OKCallbackData *
 */
void VirtualATR::loadPaletteOKCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData)
	{
	try
		{
		/* Load the palette file: */
		variableManager->loadPalette(cbData->selectedFileName.c_str());
		}
	catch(std::runtime_error)
		{
		/* Ignore the error... */
		}

	/* Destroy the file selection dialog: */
	Vrui::getWidgetManager()->deleteWidget(cbData->fileSelectionDialog);
	inLoadPalette=false;
	} // end loadPaletteOKCallback()

/*
 * menuToggleSelectCallback
 *
 * parameter callbackData - GLMotif::ToggleButton::ValueChangedCallbackData *
 */
void VirtualATR::menuToggleSelectCallback(
		GLMotif::ToggleButton::ValueChangedCallbackData * callbackData) {
	/* Adjust program state based on which toggle button changed state: */
        if (strcmp(callbackData->toggle->getName(), "showVesselToggle") == 0) {
		atr->toggleVessel();
		showVesselToggle->setToggle(callbackData->set);
		showVesselToggleRD->setToggle(callbackData->set);
	} else if (strcmp(callbackData->toggle->getName(), "showClear_VesselToggle") == 0) {
		atr->toggleClear_Vessel();
		showClear_VesselToggle->setToggle(callbackData->set);
		showClear_VesselToggleRD->setToggle(callbackData->set);
	} else if (strcmp(callbackData->toggle->getName(), "showOSCC_5Toggle") == 0) {
		atr->toggleOSCC_5();
		showOSCC_5Toggle->setToggle(callbackData->set);
		showOSCC_5ToggleOSCCD->setToggle(callbackData->set);
	} else if (strcmp(callbackData->toggle->getName(), "showOSCC_51_8Toggle") == 0) {
		atr->toggleOSCC_51_8();
		showOSCC_51_8Toggle->setToggle(callbackData->set);
		showOSCC_51_8ToggleOSCCD->setToggle(callbackData->set);
	} else if (strcmp(callbackData->toggle->getName(), "showOSCC_85Toggle") == 0) {
		atr->toggleOSCC_85();
		showOSCC_85Toggle->setToggle(callbackData->set);
		showOSCC_85ToggleOSCCD->setToggle(callbackData->set);
	} else if (strcmp(callbackData->toggle->getName(), "showOSCC_120Toggle") == 0) {
		atr->toggleOSCC_120();
		showOSCC_120Toggle->setToggle(callbackData->set);
		showOSCC_120ToggleOSCCD->setToggle(callbackData->set);
	} else if (strcmp(callbackData->toggle->getName(), "showOSCC_155Toggle") == 0) {
		atr->toggleOSCC_155();
		showOSCC_155Toggle->setToggle(callbackData->set);
		showOSCC_155ToggleOSCCD->setToggle(callbackData->set);
	} else if (strcmp(callbackData->toggle->getName(), "showOSCC_30_85_85_85Toggle") == 0) {
		atr->toggleOSCC_30_85_85_85();
		showOSCC_30_85_85_85Toggle->setToggle(callbackData->set);
		showOSCC_30_85_85_85ToggleOSCCD->setToggle(callbackData->set);
	} else if (strcmp(callbackData->toggle->getName(), "showOSCC_40_85_85_85Toggle") == 0) {
		atr->toggleOSCC_40_85_85_85();
		showOSCC_40_85_85_85Toggle->setToggle(callbackData->set);
		showOSCC_40_85_85_85ToggleOSCCD->setToggle(callbackData->set);
	} else if (strcmp(callbackData->toggle->getName(), "showOSCC_50_85_85_85Toggle") == 0) {
		atr->toggleOSCC_50_85_85_85();
		showOSCC_50_85_85_85Toggle->setToggle(callbackData->set);
		showOSCC_50_85_85_85ToggleOSCCD->setToggle(callbackData->set);
	} else if (strcmp(callbackData->toggle->getName(), "showOSCC_60_85_85_85Toggle") == 0) {
		atr->toggleOSCC_60_85_85_85();
		showOSCC_60_85_85_85Toggle->setToggle(callbackData->set);
		showOSCC_60_85_85_85ToggleOSCCD->setToggle(callbackData->set);
	} else if (strcmp(callbackData->toggle->getName(), "showOSCC_70_85_85_85Toggle") == 0) {
		atr->toggleOSCC_70_85_85_85();
		showOSCC_70_85_85_85Toggle->setToggle(callbackData->set);
		showOSCC_70_85_85_85ToggleOSCCD->setToggle(callbackData->set);
	} else if (strcmp(callbackData->toggle->getName(), "showOSCC_80_85_85_85Toggle") == 0) {
		atr->toggleOSCC_80_85_85_85();
		showOSCC_80_85_85_85Toggle->setToggle(callbackData->set);
		showOSCC_80_85_85_85ToggleOSCCD->setToggle(callbackData->set);
	} else if (strcmp(callbackData->toggle->getName(), "showOSCC_90_85_85_85Toggle") == 0) {
		atr->toggleOSCC_90_85_85_85();
		showOSCC_90_85_85_85Toggle->setToggle(callbackData->set);
		showOSCC_90_85_85_85ToggleOSCCD->setToggle(callbackData->set);
	} else if (strcmp(callbackData->toggle->getName(), "showOSCC_100_85_85_85Toggle") == 0) {
		atr->toggleOSCC_100_85_85_85();
		showOSCC_100_85_85_85Toggle->setToggle(callbackData->set);
		showOSCC_100_85_85_85ToggleOSCCD->setToggle(callbackData->set);
	} else if (strcmp(callbackData->toggle->getName(), "showOSCC_110_85_85_85Toggle") == 0) {
		atr->toggleOSCC_110_85_85_85();
		showOSCC_110_85_85_85Toggle->setToggle(callbackData->set);
		showOSCC_110_85_85_85ToggleOSCCD->setToggle(callbackData->set);
	} else if (strcmp(callbackData->toggle->getName(), "showOSCC_120_85_85_85Toggle") == 0) {
		atr->toggleOSCC_120_85_85_85();
		showOSCC_120_85_85_85Toggle->setToggle(callbackData->set);
		showOSCC_120_85_85_85ToggleOSCCD->setToggle(callbackData->set);
	} else if (strcmp(callbackData->toggle->getName(), "wireframeToggle") == 0) {
		atr->toggleWireframe();
		wireframeToggle->setToggle(callbackData->set);
		wireframeToggleRD->setToggle(callbackData->set);
	} else if (strcmp(callbackData->toggle->getName(), "showRenderDialogToggle")
			== 0) {
		if (callbackData->set) {
			/* Open the render dialog at the same position as the main menu: */
			Vrui::getWidgetManager()->popupPrimaryWidget(
					renderDialog,
					Vrui::getWidgetManager()->calcWidgetTransformation(mainMenu));
		} else {
			/* Close the render dialog: */
			Vrui::popdownPrimaryWidget(renderDialog);
		}
	} else if (strcmp(callbackData->toggle->getName(), "showOSCCDialogToggle")
			== 0) {
		if (callbackData->set) {
			/* Open the oscc dialog at the same position as the main menu: */
			Vrui::getWidgetManager()->popupPrimaryWidget(
					osccDialog,
					Vrui::getWidgetManager()->calcWidgetTransformation(mainMenu));
		} else {
			/* Close the oscc dialog: */
			Vrui::popdownPrimaryWidget(osccDialog);
		}
	}
} // end menuToggleSelectCallback()

/*
 * saveElementsCallback
 * parameter cbData - Misc::CallbackData *
 */
void VirtualATR::saveElementsCallback(Misc::CallbackData * cbData)
	{
	if(Vrui::isMaster())
		{
		#if 1
		/* Create the ASCII element file: */
		char elementFileNameBuffer[256];
		Misc::createNumberedFileName("SavedElements.asciielem",4,elementFileNameBuffer);

		/* Save the visible elements to a binary file: */
		elementList->saveElements(elementFileNameBuffer,true,variableManager);
		#else
		/* Create the binary element file: */
		char elementFileNameBuffer[256];
		Misc::createNumberedFileName("SavedElements.binelem",4,elementFileNameBuffer);

		/* Save the visible elements to a binary file: */
		elementList->saveElements(elementFileNameBuffer,false,variableManager);
		#endif
		}
	} // end saveElementsCallback()

/*
 * showColorBarCallback
 * parameter cbData - GLMotif::ToggleButton::ValueChangedCallbackData *
 */
void VirtualATR::showColorBarCallback(GLMotif::ToggleButton::ValueChangedCallbackData * cbData)
	{
	/* Hide or show color bar dialog based on toggle button state: */
	variableManager->showColorBar(cbData->set);
	} // end showColorBarCallback()

/*
 * showElementListCallback
 * parameter cbData - GLMotif::ToggleButton::ValueChangedCallbackData *
 */
void VirtualATR::showElementListCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	/* Hide or show element list based on toggle button state: */
	if(cbData->set)
		elementList->showElementList(Vrui::getWidgetManager()->calcWidgetTransformation(mainMenu));
	else
		elementList->hideElementList();
	} // end showElementListCallback()

/*
 * showPaletteEditorCallback
 * parameter cbData - GLMotif::ToggleButton::ValueChangedCallbackData *
 */
void VirtualATR::showPaletteEditorCallback(GLMotif::ToggleButton::ValueChangedCallbackData * cbData)
	{
	/* Hide or show palette editor based on toggle button state: */
	variableManager->showPaletteEditor(cbData->set);
	} // end showPaletteEditorCallback()

/*
 * sliderCallback
 *
 * parameter callbackData - GLMotif::Slider::ValueChangedCallbackData *
 */
void VirtualATR::sliderCallback(
		GLMotif::Slider::ValueChangedCallbackData * callbackData) {
	if (strcmp(callbackData->slider->getName(), "SurfaceTransparencySlider")
			== 0) {
		;
	} else if (strcmp(callbackData->slider->getName(), "GridTransparencySlider")
			== 0) {
		;
	}
} // end sliderCallback()

/*
 * toolCreationCallback
 *
 * parameter callbackData - Vrui::ToolManager::ToolCreationCallbackData *
 */
void VirtualATR::toolCreationCallback(
		Vrui::ToolManager::ToolCreationCallbackData * callbackData) {
	/* Check if the new tool is a locator tool: */
	Vrui::LocatorTool* locatorTool=dynamic_cast<Vrui::LocatorTool*>(callbackData->tool);
	if(locatorTool!=0)
		{
		BaseLocator* newLocator;
		if(algorithm==0)
			{
			/* Create a cutting plane locator object and associate it with the new tool: */
			newLocator=new CuttingPlaneLocator(locatorTool,this);
			}
		else if(algorithm<firstScalarAlgorithmIndex)
			{
			/* Create a scalar evaluation locator object and associate it with the new tool: */
			newLocator=new ScalarEvaluationLocator(locatorTool,this);
			}
		else if(algorithm<firstScalarAlgorithmIndex+module->getNumScalarAlgorithms())
			{
			/* Create a data locator object and associate it with the new tool: */
			int algorithmIndex=algorithm-firstScalarAlgorithmIndex;
			Algorithm* extractor=module->getScalarAlgorithm(algorithmIndex,variableManager,Vrui::openPipe());
			newLocator=new ExtractorLocator(locatorTool,this,extractor);
			}
		else if(algorithm<firstVectorAlgorithmIndex)
			{
			/* Create a vector evaluation locator object and associate it with the new tool: */
			newLocator=new VectorEvaluationLocator(locatorTool,this);
			}
		else
			{
			/* Create a data locator object and associate it with the new tool: */
			int algorithmIndex=algorithm-firstVectorAlgorithmIndex;
			Algorithm* extractor=module->getVectorAlgorithm(algorithmIndex,variableManager,Vrui::openPipe());
			newLocator=new ExtractorLocator(locatorTool,this,extractor);
			}

		/* Add new locator to list: */
		baseLocators.push_back(newLocator);
		}
} // end toolCreationCallback()

/*
 * toolDestructionCallback
 *
 * parameter callbackData - Vrui::ToolManager::ToolDestructionCallbackData *
 */
void VirtualATR::toolDestructionCallback(
		Vrui::ToolManager::ToolDestructionCallbackData * callbackData) {
	/* Check if the to-be-destroyed tool is a locator tool: */
	Vrui::LocatorTool* locatorTool=dynamic_cast<Vrui::LocatorTool*>(callbackData->tool);
	if(locatorTool!=0)
		{
		/* Find the data locator associated with the tool in the list: */
		for(BaseLocatorList::iterator blIt=baseLocators.begin();blIt!=baseLocators.end();++blIt)
			if((*blIt)->getTool()==locatorTool)
				{
				/* Remove the locator: */
				delete *blIt;
				baseLocators.erase(blIt);
				break;
				}
		}
} // end toolDestructionCallback()

/*
 * main - The application main method.
 *
 * parameter argc - int
 * parameter argv - char**
 */
int main(int argc, char* argv[]) {
	try {
		/* Create the Vrui application object: */
		char** applicationDefaults = 0;
		VirtualATR application(argc, argv, applicationDefaults);

		/* Run the Vrui application: */
		application.run();

		/* Return to the OS: */
		return 0;
	} catch (std::runtime_error err) {
		/* Print an error message and return to the OS: */
		std::cerr << "Caught exception " << err.what() << std::endl;
		return 1;
	}
} // end main()
