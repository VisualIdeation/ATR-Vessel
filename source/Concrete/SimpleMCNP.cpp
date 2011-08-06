/***********************************************************************
SimpleMCNP - This class reads an ASCII file which contains the results
of an MCNP run. The first line of the file is expected to contain three
integers which correspond to the number of x, y, and z points to plot, 
in that order. The rest of the file must be made up of lines which 
consist of five decimal numbers. The first three are the coordinates 
of a point, the fourth is the result of the flux, and the fifth is the
relative error at that point. These numbers are stored and then 
3DVisualizer methods are called on these stored values. This is a 
3DVisualizer Module.

Copyright (c) 2010 Vanessa Gertman

This module was modeled after the ExampleModule from 3D Visualizer.
The ExampleModule is free software under the GNU General Public License.
***********************************************************************/
#include <math.h>
#include <stdio.h>
#include <Plugins/FactoryManager.h>

#include <Concrete/SimpleMCNP.h>
#define PI 3.1415926535897932384626
namespace Visualization {

namespace Concrete {

/******************************
Methods of class SimpleMCNP:
******************************/

/***********************************************************************
Constructor for SimpleMCNP class. Contains no code except the
definition of its own name.
***********************************************************************/

SimpleMCNP::SimpleMCNP(void)
	:BaseModule("SimpleMCNP")
	{
	}

/***********************************************************************
Method to load a data set from a file, given a particular command line.
This method defines the format of the data files read by this module
class and has to be written.
***********************************************************************/

  Visualization::Abstract::DataSet* SimpleMCNP::load(const std::vector<std::string>& args, Comm::MulticastPipe* pipe) const
	{
	/* Create the result data set: */
	DataSet* result=new DataSet;
	
	/* Open the input file: */
	FILE* file=fopen(args[0].c_str(),"rt"); // args[0] is the first module command line parameter
	
	int status;
	/* Read the input file's header: */
	DS::Index numVertices; // DS::Index is a helper type containing three integers NI, NJ, NK
	status = fscanf(file,"%d %d %d",&numVertices[0],&numVertices[1],&numVertices[2]);
	
	/* Define the result data set's grid layout: */
	DS& dataSet=result->getDs(); // Get the internal data representation from the result data set
	dataSet.setGrid(numVertices); // Set the data set's number of vertices
	dataSet.addSlice(); // Add a single scalar variable to the data set's grid, need as many calls as variables
	dataSet.addSlice(); // Add a single scalar variable to the data set's grid
	
	/* Define the result data set's variables as they are selected in 3D Visualizer's menus: */
	DataValue& dataValue=result->getDataValue(); // Get the internal representations of the data set's value space
	dataValue.initialize(&dataSet); // Initialize the value space for the data set
	dataValue.setScalarVariableName(0,"Flux"); // Set the name of the first scalar variable
	dataValue.setScalarVariableName(1,"Relative Error"); // Set the name of the first scalar variable,
	
	/* Read all vertex positions and temperature values: */
	DS::Index index; // Index counting variable containing three integers I, J, K
	for(index[0]=0;index[0]<numVertices[0];++index[0]) // I varies most slowly
		for(index[1]=0;index[1]<numVertices[1];++index[1])
			for(index[2]=0;index[2]<numVertices[2];++index[2]) // K varies fastest
				{
				/* Read the vertex position and flux and relative error values: */
				  double pos[3],flux, relative_err;
				  status = fscanf(file,"%lf %lf %lf %lf %lf",&pos[0],&pos[1],&pos[2],&flux, &relative_err);
			        /*Rotating and resizing the data to fit correctly in the core*/
				  double temp_x= pos[0];
				  pos[0]= ((pos[0]*cos(PI/4)) - (pos[1]*sin(PI/4)))*0.01;
				  pos[1]= ((temp_x*sin(PI/4)) + (pos[1]*cos(PI/4)))*0.01;
				  pos[2]= (pos[2])*0.01;

				/* Store the position and value in the data set: */
				dataSet.getVertexPosition(index)=DS::Point(pos); // Store the vertex' position
				dataSet.getVertexValue(0,index)=DS::ValueScalar(flux); // Store the vertex' flux
				dataSet.getVertexValue(1,index)=DS::ValueScalar(relative_err); // Store the vertex' relative error
				}
	
	/* Close the input file: */
	fclose(file);
	
	/* Finalize the data set's grid structure (required): */
	dataSet.finalizeGrid();
	
	/* Return the result data set: */
	return result;
	}

}

}

/***********************************************************************
Plug-in interface functions. These allow loading dynamically loading
modules into 3D Visualizer at run-time, and do not have to be changed
except for the name of the generated module class.
***********************************************************************/

extern "C" Visualization::Abstract::Module* createFactory(Plugins::FactoryManager<Visualization::Abstract::Module>& manager)
	{
	/* Create module object and insert it into class hierarchy: */
	Visualization::Concrete::SimpleMCNP* module=new Visualization::Concrete::SimpleMCNP();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
