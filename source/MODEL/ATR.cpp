/*
 * ATR.cpp - Methods for ATR facility.
 *
 * Author: Patrick O'Leary
 * Created: May 11, 2010
 * Copyright: 2010
 * Requirements: Delta3D 2.4.0 (Modified) and OSG 2.8.2
 */

/* System headers */
#include <iostream>

/* Application headers */
#include <SYNC/Guard.h>

/* Delta3D headers */
#include <dtCore/camera.h>
#include <dtCore/infinitelight.h>
#include <dtCore/object.h>
#include <dtCore/scene.h>
#include <dtCore/system.h>
#include <dtCore/environment.h>

/* OSG headers */
#include <osg/DisplaySettings>
#include <osgDB/FileUtils>

/* ODE headers */
#include <ode/ode.h>

/* Vrui Headers */
#include <Vrui/Vrui.h>

#include "ATR.h"

static bool dophysics = false;
static double lastTime = 0.0f;
static double *simulationTime;

using namespace std;
using namespace dtCore;
using namespace dtABC;
using namespace dtUtil;

/*****************************************
 Methods of class ATR::DataItem:
 *****************************************/

ATR::DataItem::DataItem(void) {

}

ATR::DataItem::~DataItem(void) {

}

/****************************************************
 Constructors and Destructors of class ATR:
 ****************************************************/
/*
 * ATR constructor
 */
ATR::ATR(void) :
	Application(true), drawMode(true), frameNumber(0) {

	atr = this;

	// Tell osg::Referenced to use thread-safe reference counting. We do
	// this here for all applications and all configurations though it is
	// only strictly necessary when Vrui is configured to use
	// multi-threaded rendering.
	osg::Referenced::setThreadSafeReferenceCounting(true);

	/* Initialize update visitor */
	updateVisitor = new osgUtil::UpdateVisitor();
	frameStamp = new ::osg::FrameStamp();
	updateVisitor->setFrameStamp(frameStamp.get());
} // end ATR()

ATR::~ATR(void) {
}

/*******************************
 Methods of class ATR:
 *******************************/

/*
 * addObjects
 */
void ATR::addObjects(void) {
	GetScene()->AddDrawable(user.get());
	GetScene()->AddDrawable(vessel.get());
	GetScene()->AddDrawable(clear_vessel.get());
	GetScene()->AddDrawable(environment.get());
	// create an infinite light
	globalInfinite = new InfiniteLight(4, "GlobalInfiniteLight");
	GetScene()->AddDrawable(globalInfinite.get());
	globalInfinite->SetEnabled(true);

	/* 
	 * oscc_#: # represents angle at which cylinders are rotated 
	 * from 0 degrees. One number means all cylinders are at same 
	 * rotation; multiple numbers mean NW quadrant, then NE 
	 * quadrant, then SE quadrant, then SW quadrant (NOTE: 
	 * decimals are represented by underscores. EX 51.8 = 51_8). 
	 */
	GetScene()->AddDrawable(oscc_5.get());
	GetScene()->AddDrawable(oscc_51_8.get());
	GetScene()->AddDrawable(oscc_85.get());
	GetScene()->AddDrawable(oscc_120.get());
	GetScene()->AddDrawable(oscc_155.get());
	GetScene()->AddDrawable(oscc_30_85_85_85.get());
	GetScene()->AddDrawable(oscc_40_85_85_85.get());
	GetScene()->AddDrawable(oscc_50_85_85_85.get());
	GetScene()->AddDrawable(oscc_60_85_85_85.get());
	GetScene()->AddDrawable(oscc_70_85_85_85.get());
	GetScene()->AddDrawable(oscc_80_85_85_85.get());
	GetScene()->AddDrawable(oscc_90_85_85_85.get());
	GetScene()->AddDrawable(oscc_100_85_85_85.get());
	GetScene()->AddDrawable(oscc_110_85_85_85.get());
	GetScene()->AddDrawable(oscc_120_85_85_85.get());
} // end addObjects()

/*
 * createEnvironment
 */
void ATR::createEnvironment(void) {
	environment = new dtCore::Environment;
	environment->SetName("Environment");
	environment->SetDateTime(2004, 6, 21, 12, 1, 1);
} // end createEnvironment

/*
 * createUser
 *
 * param mass - dMass &
 */
void ATR::createUser(dMass & mass) {
	user = new Object("User");
	//	user->SetCollisionCappedCylinder(1.0f, 1.0f);
	user->RenderCollisionGeometry(true);
	dMassSetBox(&mass, 1.0, 1.0, 1.0, 1.0);
	user->SetMass(&mass);
	user->EnableDynamics();
} // end createUser()

void ATR::createVessel(void) {
	vessel = new Object("Vessel");
	vessel->LoadFile("models/SReactor_Vessel.osg");
}

void ATR::createClear_Vessel(void) {
	clear_vessel = new Object("Clear_Vessel");
	clear_vessel->LoadFile("models/zw/Clear_Vessel.osg");
}

void ATR::createOSCC_5(void) {
	oscc_5 = new Object("OSCC_5");
	oscc_5->LoadFile("models/zw/OSCC/OSCC_NW5_NE5_SE5_SW5.osg");
}

void ATR::createOSCC_51_8(void) {
	oscc_51_8 = new Object("OSCC_51_8");
	oscc_51_8->LoadFile("models/zw/OSCC/OSCC_NW51_8_NE51_8_SE51_8_SW51_8.osg");
}

void ATR::createOSCC_85(void) {
	oscc_85 = new Object("OSCC_85");
	oscc_85->LoadFile("models/zw/OSCC/OSCC_NW85_NE85_SE85_SW85.osg");
}

void ATR::createOSCC_120(void) {
	oscc_120 = new Object("OSCC_120");
	oscc_120->LoadFile("models/zw/OSCC/OSCC_NW120_NE120_SE120_SW120.osg");
}

void ATR::createOSCC_155(void) {
	oscc_155 = new Object("OSCC_155");
	oscc_155->LoadFile("models/zw/OSCC/OSCC_NW155_NE155_SE155_SW155.osg");
}

void ATR::createOSCC_30_85_85_85(void) {
	oscc_30_85_85_85 = new Object("OSCC_30_85_85_85");
	oscc_30_85_85_85->LoadFile("models/zw/OSCC/OSCC_NW30_NE85_SE85_SW85.osg");
}

void ATR::createOSCC_40_85_85_85(void) {
	oscc_40_85_85_85 = new Object("OSCC_40_85_85_85");
	oscc_40_85_85_85->LoadFile("models/zw/OSCC/OSCC_NW40_NE85_SE85_SW85.osg");
}

void ATR::createOSCC_50_85_85_85(void) {
	oscc_50_85_85_85 = new Object("OSCC_50_85_85_85");
	oscc_50_85_85_85->LoadFile("models/zw/OSCC/OSCC_NW50_NE85_SE85_SW85.osg");
}

void ATR::createOSCC_60_85_85_85(void) {
	oscc_60_85_85_85 = new Object("OSCC_60_85_85_85");
	oscc_60_85_85_85->LoadFile("models/zw/OSCC/OSCC_NW60_NE85_SE85_SW85.osg");
}

void ATR::createOSCC_70_85_85_85(void) {
	oscc_70_85_85_85 = new Object("OSCC_70_85_85_85");
	oscc_70_85_85_85->LoadFile("models/zw/OSCC/OSCC_NW70_NE85_SE85_SW85.osg");
}

void ATR::createOSCC_80_85_85_85(void) {
	oscc_80_85_85_85 = new Object("OSCC_80_85_85_85");
	oscc_80_85_85_85->LoadFile("models/zw/OSCC/OSCC_NW80_NE85_SE85_SW85.osg");
}

void ATR::createOSCC_90_85_85_85(void) {
	oscc_90_85_85_85 = new Object("OSCC_90_85_85_85");
	oscc_90_85_85_85->LoadFile("models/zw/OSCC/OSCC_NW90_NE85_SE85_SW85.osg");
}

void ATR::createOSCC_100_85_85_85(void) {
	oscc_100_85_85_85 = new Object("OSCC_100_85_85_85");
	oscc_100_85_85_85->LoadFile("models/zw/OSCC/OSCC_NW100_NE85_SE85_SW85.osg");
}

void ATR::createOSCC_110_85_85_85(void) {
	oscc_110_85_85_85 = new Object("OSCC_110_85_85_85");
	oscc_110_85_85_85->LoadFile("models/zw/OSCC/OSCC_NW110_NE85_SE85_SW85.osg");
}

void ATR::createOSCC_120_85_85_85(void) {
	oscc_120_85_85_85 = new Object("OSCC_120_85_85_85");
	oscc_120_85_85_85->LoadFile("models/zw/OSCC/OSCC_NW120_NE85_SE85_SW85.osg");
}

/*
 * config
 */
void ATR::config(void) {
	dMass mass;

	/* Load the model files */
	createEnvironment();
	createUser(mass);
	createVessel();
	createClear_Vessel();
	createOSCC_5();
	createOSCC_51_8();
	createOSCC_85();
	createOSCC_120();
	createOSCC_155();
	createOSCC_30_85_85_85();
	createOSCC_40_85_85_85();
	createOSCC_50_85_85_85();
	createOSCC_60_85_85_85();
	createOSCC_70_85_85_85();
	createOSCC_80_85_85_85();
	createOSCC_90_85_85_85();
	createOSCC_100_85_85_85();
	createOSCC_110_85_85_85();
	createOSCC_120_85_85_85();

	addObjects();
} // end config()

/*
 * display
 *
 * parameter glContextData - GLContextData &
 */
void ATR::display(GLContextData & glContextData) const {
	/* Get context data item: */
	DataItem* dataItem = glContextData.retrieveDataItem<DataItem> (this);

	dataItem->viewer->advance(lastFrameTime);
	if (!dataItem->viewer->done()) {
		dataItem->viewer->updateTraversal();
	}

	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	GLdouble mv[16], p[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, mv);
	glGetDoublev(GL_PROJECTION_MATRIX, p);

	dataItem->viewer->getCamera()->setViewport(vp[0], vp[1], vp[2], vp[3]);
	dataItem->viewer->getCamera()->setProjectionMatrix(osg::Matrix(p));
	dataItem->viewer->getCamera()->setViewMatrix(osg::Matrix(mv));

	/* Render all opaque surfaces: */
	dataItem->viewer->renderingTraversals();
} // end display()

/*
 * frame
 */
void ATR::frame(void) {
	/* Get the current application time: */
	double newFrameTime = Vrui::getApplicationTime();

	++frameNumber;

	// Update the frame stamp with information from this frame.
	frameStamp->setFrameNumber(frameNumber);
	frameStamp->setReferenceTime(newFrameTime);
	frameStamp->setSimulationTime(newFrameTime);

	// Set up the time and frame number so time-dependent things (animations,
	// particle system) function correctly.
	updateVisitor->setTraversalNumber(frameNumber);

	lastFrameTime = newFrameTime;
} // end frame()

/*
 * initContext
 *
 * parameter glContextData - GLContextData &
 */
void ATR::initContext(GLContextData & glContextData) const {
	/* Create a new context data item: */
	DataItem* dataItem = new DataItem();

	// Create the osgViewer instance
	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;
	viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);

	// Set up osgViewer::GraphicsWindowEmbedded for this context
	osg::ref_ptr<osg::GraphicsContext::Traits> traits =
			new ::osg::GraphicsContext::Traits;
	osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> graphicsWindow =
			new osgViewer::GraphicsWindowEmbedded(traits.get());
	viewer->getCamera()->setGraphicsContext(graphicsWindow.get());
	viewer->getCamera()->getGraphicsContext()->getState()->setContextID(
			osg::GraphicsContext::createNewContextID());

	viewer->getCamera()->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);

	viewer->getCamera()->setClearColor(::osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
	viewer->getCamera()->setClearMask(0);

	osg::Group * root = new osg::Group();
	dataItem->root = root;
	root->setName("Root");
	root->addChild(atr->GetRootNode());

	// Add the tree to the viewer and set properties
	Guard<MutexPosix> viewerGuard(dataItem->viewerLock);
	viewer->setSceneData(root);

	dataItem->viewer = viewer;

	glContextData.addDataItem(this, dataItem);
} // end initContext()

void ATR::toggleLight(void) {
	globalInfinite->SetEnabled(!globalInfinite->GetEnabled());
}

void ATR::toggleVessel(void) {
	vessel.get()->DeltaDrawable::SetActive(
			!vessel.get()->DeltaDrawable::GetActive());
}

void ATR::toggleClear_Vessel(void) {
	clear_vessel.get()->DeltaDrawable::SetActive(
			!clear_vessel.get()->DeltaDrawable::GetActive());
}

void ATR::toggleOSCC_5(void) {
	oscc_5.get()->DeltaDrawable::SetActive(
			!oscc_5.get()->DeltaDrawable::GetActive());
}

void ATR::toggleOSCC_51_8(void) {
	oscc_51_8.get()->DeltaDrawable::SetActive(
			!oscc_51_8.get()->DeltaDrawable::GetActive());
}

void ATR::toggleOSCC_85(void) {
	oscc_85.get()->DeltaDrawable::SetActive(
			!oscc_85.get()->DeltaDrawable::GetActive());
}

void ATR::toggleOSCC_120(void) {
	oscc_120.get()->DeltaDrawable::SetActive(
			!oscc_120.get()->DeltaDrawable::GetActive());
}

void ATR::toggleOSCC_155(void) {
	oscc_155.get()->DeltaDrawable::SetActive(
			!oscc_155.get()->DeltaDrawable::GetActive());
}

void ATR::toggleOSCC_30_85_85_85(void) {
	oscc_30_85_85_85.get()->DeltaDrawable::SetActive(
			!oscc_30_85_85_85.get()->DeltaDrawable::GetActive());
}

void ATR::toggleOSCC_40_85_85_85(void) {
	oscc_40_85_85_85.get()->DeltaDrawable::SetActive(
			!oscc_40_85_85_85.get()->DeltaDrawable::GetActive());
}

void ATR::toggleOSCC_50_85_85_85(void) {
	oscc_50_85_85_85.get()->DeltaDrawable::SetActive(
			!oscc_50_85_85_85.get()->DeltaDrawable::GetActive());
}

void ATR::toggleOSCC_60_85_85_85(void) {
	oscc_60_85_85_85.get()->DeltaDrawable::SetActive(
			!oscc_60_85_85_85.get()->DeltaDrawable::GetActive());
}

void ATR::toggleOSCC_70_85_85_85(void) {
	oscc_70_85_85_85.get()->DeltaDrawable::SetActive(
			!oscc_70_85_85_85.get()->DeltaDrawable::GetActive());
}

void ATR::toggleOSCC_80_85_85_85(void) {
	oscc_80_85_85_85.get()->DeltaDrawable::SetActive(
			!oscc_80_85_85_85.get()->DeltaDrawable::GetActive());
}

void ATR::toggleOSCC_90_85_85_85(void) {
	oscc_90_85_85_85.get()->DeltaDrawable::SetActive(
			!oscc_90_85_85_85.get()->DeltaDrawable::GetActive());
}

void ATR::toggleOSCC_100_85_85_85(void) {
	oscc_100_85_85_85.get()->DeltaDrawable::SetActive(
			!oscc_100_85_85_85.get()->DeltaDrawable::GetActive());
}

void ATR::toggleOSCC_110_85_85_85(void) {
	oscc_110_85_85_85.get()->DeltaDrawable::SetActive(
			!oscc_110_85_85_85.get()->DeltaDrawable::GetActive());
}

void ATR::toggleOSCC_120_85_85_85(void) {
	oscc_120_85_85_85.get()->DeltaDrawable::SetActive(
			!oscc_120_85_85_85.get()->DeltaDrawable::GetActive());
}

void ATR::toggleWireframe(void) {
	if (drawMode) {
		GetScene()->SetRenderState(dtCore::Scene::FRONT_AND_BACK,
				dtCore::Scene::LINE);
		drawMode = false;
	} else {
		GetScene()->SetRenderState(dtCore::Scene::FRONT, dtCore::Scene::FILL);
		drawMode = true;
	}
}
