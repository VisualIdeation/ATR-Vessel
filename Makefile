########################################################################
# Makefile for ATR-Vessel
# Copyright (c) 2010 Patrick O'Leary
########################################################################
# Specify the main target
TARGET = ATR-Vessel

# Build type
DEBUG = 0

# Vrui directory
VRUIDIR = /opt/local/Vrui-1.0

# Install directory
INSTALLDIR = /opt/local/ATR-Vessel

# List of unsupported available modules:
UNSUPPORTED_MODULE_NAMES = AnalyzeFile \
                           ByteVolFile \
                           GaleFEMVectorFile \
                           ConvectionFile \
                           ConvectionFileCartesian \
                           CSConvectionFile \
                           DicomImageStack \
                           EarthTomographyGrid \
                           ReifSeismicTomography \
                           SeismicTomographyModel \
                           FloatGridFile \
                           FloatVolFile \
                           VecVolFile \
                           Kollmann0p9File \
                           MagaliSubductionFile \
                           MargareteSubductionFile \
                           VanKekenFile \
                           UnstructuredPlot3DFile

# List of default visualization modules:
MODULE_NAMES = $(UNSUPPORTED_MODULE_NAMES) \
			   SphericalASCIIFile \
               StructuredGridASCII \
               StructuredGridVTK \
               CitcomCUCartesianRawFile \
               CitcomCUSphericalRawFile \
               CitcomSRegionalASCIIFile \
               CitcomSGlobalASCIIFile \
               CitcomtFile \
               MultiCitcomtFile \
               CitcomtVectorFile \
               StructuredHexahedralTecplotASCIIFile \
               UnstructuredHexahedralTecplotASCIIFile \
               ImageStack \
               RealMCNP \
	       SimpleMCNP \
	       SimpleMCNPFluxOnly \
               MultiChannelImageStack

# Flag whether to use GLSL shaders instead of fixed OpenGL functionality
# for some visualization algorithms, especially volume rendering.
USE_SHADERS = 0

# Version number for installation subdirectories.
VERSION =

# Set up destination directories for compilation products:
OBJDIRBASE = object
BINDIRBASE = bin
MODULEDIRBASE = $(TARGET)

# Set resource directory:
RESOURCEDIR = share/$(TARGET)

# Set up additional flags for the C++ compiler:
CFLAGS = 

# Create debug or fully optimized versions of the software:
ifneq ($(DEBUG),0)
  # Include the debug version of the Vrui application makefile fragment:
  include $(VRUIDIR)/etc/Vrui.debug.makeinclude
  # Enable debugging and disable optimization:
  CFLAGS += -g3 -O0
  # Set destination directories for created objects:
  OBJDIR = $(OBJDIRBASE)/debug
  BINDIR = $(BINDIRBASE)/debug
  MODULEDIR = $(MODULEDIRBASE)/debug
else
  # Include the release version of the Vrui application makefile fragment:
  include $(VRUIDIR)/etc/Vrui.makeinclude
  # Disable debugging and enable optimization:
  CFLAGS += -g0 -O3 -DNDEBUG
  # Set destination directories for created objects:
  OBJDIR = $(OBJDIRBASE)
  BINDIR = $(BINDIRBASE)
  MODULEDIR = $(MODULEDIRBASE)
endif

# Add base directory to include path:
CFLAGS += -I./source

# Pattern rule to compile C++ sources:
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)/$(*D)
	@echo Compiling $<...
	@g++ -c -o $@ $(VRUI_CFLAGS) $(CFLAGS) $<

# System-default destination directory for dynamic libraries:
ifeq (`uname -m`,x86_64)
  LIBDIR = lib64
else
  LIBDIR = lib
endif

# Which libraries are linked
LIBS = GLU dtABC

# Add directories to the include and library paths
INCPATH = /usr/local/include
LIBPATH = /usr/local/lib /usr/local/lib64

# Function to generate full plug-in names:
PLUGINNAME = $(LIBDIR)/$(MODULEDIR)/lib$(1).$(VRUI_PLUGINFILEEXT)

# Pattern rule to link visualization module plug-ins:
$(call PLUGINNAME,%): CFLAGS += $(VRUI_PLUGINCFLAGS)
ifneq ($(USE_SHADERS),0)
  $(call PLUGINNAME,%): CFLAGS += -DVIRTUALATR_USE_SHADERS
endif
$(call PLUGINNAME,%): CFLAGS += $(VRUI_PLUGINCFLAGS)
$(call PLUGINNAME,%): $(OBJDIR)/source/Concrete/%.o
	@mkdir -p $(LIBDIR)/$(MODULEDIR)
	@echo Linking $@...
	@g++ $(VRUI_PLUGINLINKFLAGS) -o $@ $^

# Rule to build all ATR-Vessel components:
MODULES = $(MODULE_NAMES:%=$(call PLUGINNAME,%))
ALL = $(BINDIR)/$(TARGET) \
      $(MODULES)
.PHONY: all
all: $(ALL)

# Rule to remove build results:
clean:
	-rm -f $(OBJDIR)/source/*.o $(OBJDIR)/source/Abstract/*.o $(OBJDIR)/source/ANALYSIS/*.o $(OBJDIR)/source/Concrete/*.o $(OBJDIR)/source/MODEL/*.o $(OBJDIR)/source/SYNC/*.o $(OBJDIR)/source/Templatized/*.o $(OBJDIR)/source/UTIL/*.o $(OBJDIR)/source/Wrappers/*.o
	-rmdir $(OBJDIR)/source/Abstract $(OBJDIR)/source/ANALYSIS $(OBJDIR)/source/Concrete $(OBJDIR)/source/MODEL $(OBJDIR)/source/SYNC $(OBJDIR)/source/Templatized $(OBJDIR)/source/UTIL $(OBJDIR)/source/Wrappers
	-rmdir $(OBJDIR)/source
	-rm -f $(ALL)

# Rule to clean the source directory for packaging:
distclean:
	-rm -rf $(OBJDIRBASE)
	-rm -rf $(BINDIRBASE)
	-rm -rf lib lib64

# List of required source files:
ABSTRACT_SOURCES = source/Abstract/ScalarExtractor.cpp \
                   source/Abstract/VectorExtractor.cpp \
                   source/Abstract/DataSet.cpp \
                   source/Abstract/VariableManager.cpp \
                   source/Abstract/DataSetRenderer.cpp \
                   source/Abstract/Algorithm.cpp \
                   source/Abstract/Element.cpp \
                   source/Abstract/CoordinateTransformer.cpp \
                   source/Abstract/Module.cpp

ANALYSIS_SOURCES = source/ANALYSIS/BaseLocator.cpp \
                   source/ANALYSIS/ClippingPlane.cpp \
                   source/ANALYSIS/ClippingPlaneLocator.cpp \
                   source/ANALYSIS/CuttingPlaneLocator.cpp \
                   source/ANALYSIS/EvaluationLocator.cpp \
                   source/ANALYSIS/Extractor.cpp \
                   source/ANALYSIS/ExtractorLocator.cpp \
                   source/ANALYSIS/ScalarEvaluationLocator.cpp \
                   source/ANALYSIS/VectorEvaluationLocator.cpp

MODEL_SOURCES = source/MODEL/ATR.cpp

SYNC_SOURCES = source/SYNC/DeadlockException.cpp \
               source/SYNC/LockException.cpp \
               source/SYNC/MutexPosix.cpp
                   
TEMPLATIZED_SOURCES = source/Templatized/Simplex.cpp \
                      source/Templatized/Tesseract.cpp \
                      source/Templatized/SliceCaseTableSimplex.cpp \
                      source/Templatized/SliceCaseTableTesseract.cpp \
                      source/Templatized/IsosurfaceCaseTableSimplex.cpp \
                      source/Templatized/IsosurfaceCaseTableTesseract.cpp

UTIL_SOURCES = source/UTIL/Exception.cpp \
               source/UTIL/ResourceException.cpp \
               source/UTIL/SystemBase.cpp \
               source/UTIL/SystemPosix.cpp

WRAPPERS_SOURCES = source/Wrappers/ParametersIOHelper.cpp \
                   source/Wrappers/RenderArrow.cpp \
                   source/Wrappers/CartesianCoordinateTransformer.cpp \
                   source/Wrappers/SlicedScalarVectorDataValue.cpp

CONCRETE_SOURCES = source/Concrete/SphericalCoordinateTransformer.cpp \
                   source/Concrete/EarthRenderer.cpp \
                   source/Concrete/PointSet.cpp

VIRTUALATR_SOURCES = $(ABSTRACT_SOURCES) \
					 $(ANALYSIS_SOURCES) \
					 $(CONCRETE_SOURCES) \
					 $(MODEL_SOURCES) \
					 $(SYNC_SOURCES) \
                     $(TEMPLATIZED_SOURCES) \
                     $(UTIL_SOURCES) \
                     $(WRAPPERS_SOURCES) \
                     source/ElementList.cpp \
                     source/ColorBar.cpp \
                     source/ColorMap.cpp \
                     source/PaletteEditor.cpp \
                     source/VirtualATR.cpp
ifneq ($(USE_SHADERS),0)
  VIRTUALATR_SOURCES += source/Polyhedron.cpp \
                        source/Raycaster.cpp \
                        source/SingleChannelRaycaster.cpp \
                        source/TripleChannelRaycaster.cpp
else
  VIRTUALATR_SOURCES += source/VolumeRenderer.cpp \
                        source/PaletteRenderer.cpp
endif

# List of required shaders:
SHADERDIR = $(RESOURCEDIR)/Shaders
SHADERS = SingleChannelRaycaster.vs \
          SingleChannelRaycaster.fs \
          TripleChannelRaycaster.vs \
          TripleChannelRaycaster.fs

# Per-source compiler flags:
$(OBJDIR)/source/Concrete/EarthRenderer.o: CFLAGS += -DEARTHRENDERER_IMAGEDIR='"$(INSTALLDIR)/$(RESOURCEDIR)"'
$(OBJDIR)/source/MODEL/ATR.o: CFLAGS += $(foreach INC,$(INCPATH),-I$(INC))
$(OBJDIR)/source/SingleChannelRaycaster.o: CFLAGS += -DVIRTUALATR_SHADERDIR='"$(INSTALLDIR)/$(SHADERDIR)"'
$(OBJDIR)/source/TripleChannelRaycaster.o: CFLAGS += -DVIRTUALATR_SHADERDIR='"$(INSTALLDIR)/$(SHADERDIR)"'
$(OBJDIR)/source/VirtualATR.o: CFLAGS += -DVIRTUALATR_MODULENAMETEMPLATE='"$(INSTALLDIR)/$(call PLUGINNAME,%s)"' $(foreach INC,$(INCPATH),-I$(INC))

#
# Rule to build VirtualATR main program
#

$(BINDIR)/$(TARGET): $(VIRTUALATR_SOURCES:%.cpp=$(OBJDIR)/%.o)
	@mkdir -p $(BINDIR)
	@echo Linking $@...
	@g++ -o $@ $^ $(VRUI_LINKFLAGS) $(VRUI_PLUGINHOSTLINKFLAGS) $(foreach LIB,$(LIBPATH),-L$(LIB)) $(foreach LIBRARY, $(LIBS),-l$(LIBRARY))
.PHONY: $(TARGET)
$(TARGET): $(BINDIR)/$(TARGET)

# Dependencies and special flags for visualization modules:
$(call PLUGINNAME,CitcomSRegionalASCIIFile): $(OBJDIR)/source/Concrete/CitcomSRegionalASCIIFile.o \
                                             $(OBJDIR)/source/Concrete/CitcomSCfgFileParser.o

$(call PLUGINNAME,CitcomSGlobalASCIIFile): $(OBJDIR)/source/Concrete/CitcomSGlobalASCIIFile.o \
                                           $(OBJDIR)/source/Concrete/CitcomSCfgFileParser.o

$(call PLUGINNAME,StructuredHexahedralTecplotASCIIFile): $(OBJDIR)/source/Concrete/TecplotASCIIFileHeaderParser.o \
                                                         $(OBJDIR)/source/Concrete/StructuredHexahedralTecplotASCIIFile.o

$(call PLUGINNAME,UnstructuredHexahedralTecplotASCIIFile): $(OBJDIR)/source/Concrete/TecplotASCIIFileHeaderParser.o \
                                                           $(OBJDIR)/source/Concrete/UnstructuredHexahedralTecplotASCIIFile.o

$(call PLUGINNAME,MultiChannelImageStack): PACKAGES += MYIMAGES

$(call PLUGINNAME,DicomImageStack): $(OBJDIR)/source/Concrete/DicomImageStack.o \
                                    $(OBJDIR)/source/Concrete/DicomImageFile.o

# Keep module object files around after building:
.SECONDARY: $(MODULE_NAMES:%=$(OBJDIR)/source/Concrete/%.o)

#
# Rule to install ATR-Vessel in a destination directory
#

install: $(ALL)
	@echo Installing ATR-Vessel in $(INSTALLDIR)...
	@install -d $(INSTALLDIR)
	@install -d $(INSTALLDIR)/bin
	@install $(BINDIR)/$(TARGET) $(INSTALLDIR)/bin
	@install -d $(INSTALLDIR)/$(LIBDIR)/$(MODULEDIR)
	@install $(MODULES) $(INSTALLDIR)/$(LIBDIR)/$(MODULEDIR)
	@install -d $(INSTALLDIR)/$(RESOURCEDIR)
ifneq ($(USE_SHADERS),0)
	@install -d $(INSTALLDIR)/$(SHADERDIR)
	@install $(SHADERS:%=$(SHADERDIR)/%) $(INSTALLDIR)/$(SHADERDIR)
endif
