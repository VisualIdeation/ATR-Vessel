/*
 * ATR.h - Class for ATR facility.
 *
 * Author: Patrick O'Leary
 * Created: May 11, 2010
 * Copyright: 2010
 */

#ifndef ATR_H_
#define ATR_H_

/* Delta3D includes */
#include <dtABC/application.h>

/* Vrui includes */
#include <GL/GLContextData.h>
#include <GL/GLObject.h>

/* osg includes */
#include <osg/Version>
#include <osg/Vec3>
#include <osg/Matrix>
#include <osg/Transform>
#include <osg/Group>
#include <osg/Camera>

#include <osgUtil/UpdateVisitor>

#include <osgViewer/Viewer>

#include <SYNC/MutexPosix.h>
#include <SYNC/NullMutex.h>

using namespace std;
using namespace dtCore;
using namespace dtABC;
using namespace dtUtil;

namespace dtCore {
class Environment;
class Object;
}
class dMass;

class ATR: public Application , public GLObject {
public:
	ATR(void);
protected:
	virtual ~ATR(void);
private:
	struct DataItem: public GLObject::DataItem {
	public:
		/* Elements: */
		int data;
		osg::Group * root;
		osg::ref_ptr<osgViewer::Viewer> viewer;
		MutexPosix viewerLock;

		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
	};
public:
	virtual void addObjects(void);
	virtual void config(void);
	virtual void display(GLContextData& contextData) const;
	void frame(void);
	virtual void initContext(GLContextData& contextData) const;
	void toggleVessel(void);
	void toggleClear_Vessel(void);
	void toggleWireframe(void);

	/* 
	 * OSCC_#: # represents angle at which cylinders are rotated 
	 * from 0 degrees. One number means all cylinders are at same 
	 * rotation; multiple numbers mean NW quadrant, then NE 
	 * quadrant, then SE quadrant, then SW quadrant (NOTE: 
	 * decimals are represented by underscores. EX 51.8 = 51_8). 
	 */
	void toggleOSCC_5(void);
	void toggleOSCC_51_8(void);
	void toggleOSCC_85(void);
	void toggleOSCC_120(void);
	void toggleOSCC_155(void);
	void toggleOSCC_30_85_85_85(void);
	void toggleOSCC_40_85_85_85(void);
	void toggleOSCC_50_85_85_85(void);
	void toggleOSCC_60_85_85_85(void);
	void toggleOSCC_70_85_85_85(void);
	void toggleOSCC_80_85_85_85(void);
	void toggleOSCC_90_85_85_85(void);
	void toggleOSCC_100_85_85_85(void);
	void toggleOSCC_110_85_85_85(void);
	void toggleOSCC_120_85_85_85(void);
	ATR * atr;
	bool drawMode;
	RefPtr<Environment> environment;
	int frameNumber;
	osg::ref_ptr<osg::FrameStamp> frameStamp;
	double lastFrameTime;
	osg::ref_ptr<osg::NodeVisitor> updateVisitor;
	RefPtr<Object> user;
	RefPtr<Object> vessel;
	RefPtr<Object> clear_vessel;
	RefPtr<Object> oscc_5;
	RefPtr<Object> oscc_51_8;
	RefPtr<Object> oscc_85;
	RefPtr<Object> oscc_120;
	RefPtr<Object> oscc_155;
	RefPtr<Object> oscc_30_85_85_85;
	RefPtr<Object> oscc_40_85_85_85;
	RefPtr<Object> oscc_50_85_85_85;
	RefPtr<Object> oscc_60_85_85_85;
	RefPtr<Object> oscc_70_85_85_85;
	RefPtr<Object> oscc_80_85_85_85;
	RefPtr<Object> oscc_90_85_85_85;
	RefPtr<Object> oscc_100_85_85_85;
	RefPtr<Object> oscc_110_85_85_85;
	RefPtr<Object> oscc_120_85_85_85;
	RefPtr<Object> wandinstrument;
private:
	void createEnvironment(void);
	void createUser(dMass & mass);
	void createVessel(void);
	void createClear_Vessel(void);
	void createOSCC_5(void);
	void createOSCC_51_8(void);
	void createOSCC_85(void);
	void createOSCC_120(void);
	void createOSCC_155(void);
	void createOSCC_30_85_85_85(void);
	void createOSCC_40_85_85_85(void);
	void createOSCC_50_85_85_85(void);
	void createOSCC_60_85_85_85(void);
	void createOSCC_70_85_85_85(void);
	void createOSCC_80_85_85_85(void);
	void createOSCC_90_85_85_85(void);
	void createOSCC_100_85_85_85(void);
	void createOSCC_110_85_85_85(void);
	void createOSCC_120_85_85_85(void);
};

#endif
