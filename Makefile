# Optimizer on
OPT = -Os

# Project Name
TARGET = FFT-Example

# Sources
CPP_SOURCES = PhaseVoc.cpp  Smoother.cpp util.cpp

# Library Locations
LIBDAISY_DIR = ../DaisyExamples/libDaisy
DAISYSP_DIR = ../DaisyExamples/DaisySP/

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
