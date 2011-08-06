/***********************************************************************
SimpleMCNPFluxOnly - This class reads an ASCII file which contains the 
results of an MCNP run. The first line of the file is expected to contain 
three integers which correspond to the number of x, y, and z points to 
plot, in that order. The rest of the file must be made up of lines which 
consist of four decimal numbers. The first three are the coordinates 
of a point and the fourth is the result of the flux at that point. 
These numbers are stored and then 3DVisualizer methods are called on these 
stored values. This is a 3DVisualizer Module.

Copyright (c) 2010 Vanessa Gertman

This module was modeled after the ExampleModule from 3D Visualizer.
The ExampleModule is free software under the GNU General Public License.
***********************************************************************/

#ifndef VISUALIZATION_CONCRETE_SIMPLEMCNPFLUXONLY_INCLUDED
#define VISUALIZATION_CONCRETE_SIMPLEMCNPFLUXONLY_INCLUDED

/***********************************************************************
Header files defining basic grid data structures and matching
visualization algorithms:
***********************************************************************/

#include <Wrappers/SlicedCurvilinearIncludes.h>
#include <Wrappers/SlicedScalarVectorDataValue.h>

#include <Wrappers/Module.h>

namespace Visualization {

namespace Concrete {

namespace {

/***********************************************************************
Basic type declarations, can be adapted according to requirements:
***********************************************************************/

typedef float Scalar; // Data set uses 32-bit floats to store vertex positions
typedef float VScalar; // Data set uses 32-bit floats to store vertex values

/***********************************************************************
The following type declarations define how data is represented
internally, and do not have to be changed:
***********************************************************************/

typedef Visualization::Templatized::SlicedCurvilinear<Scalar,3,VScalar> DS; // Templatized data set type
typedef Visualization::Wrappers::SlicedScalarVectorDataValue<DS,VScalar> DataValue; // Type of data value descriptor
typedef Visualization::Wrappers::Module<DS,DataValue> BaseModule; // Module base class type

}

/***********************************************************************
Declaration of the module class:
***********************************************************************/

class SimpleMCNPFluxOnly:public BaseModule
	{
	/* Constructors and destructors: */
	public:
	SimpleMCNPFluxOnly(void); // Default constructor
	
	/*********************************************************************
	Method to load a data set from a file, given a particular command
	line. This method defines the format of the data files read by this
	module class and has to be written.
	*********************************************************************/
	virtual Visualization::Abstract::DataSet* load(const std::vector<std::string>& args, Comm::MulticastPipe* pipe) const; // Method to load a data set from a file
	};

}

}

#endif
