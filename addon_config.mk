# All variables and this file are optional, if they are not present the PG and the
# makefiles will try to parse the correct values from the file system.
#
# Variables that specify exclusions can use % as a wildcard to specify that anything in
# that position will match. A partial path can also be specified to, for example, exclude
# a whole folder from the parsed paths from the file system
#
# Variables can be specified using = or +=
# = will clear the contents of that variable both specified from the file or the ones parsed
# from the file system
# += will add the values to the previous ones in the file or the ones parsed from the file
# system
#
# The PG can be used to detect errors in this file, just create a new project with this addon
# and the PG will write to the console the kind of error and in which line it is

meta:
	ADDON_NAME = ofxFft
	ADDON_DESCRIPTION = FFT addon for openFrameworks that wraps FFTW and KISS FFT
	ADDON_AUTHOR =
	ADDON_TAGS = "FFT" "FFTW" "KissFFT"
	ADDON_URL = https://github.com/kylemcdonald/ofxFft

common:
	# dependencies with other addons, a list of them separated by spaces
	# or use += in several lines
	# ADDON_DEPENDENCIES =

	# include search paths, this will be usually parsed from the file system
	# but if the addon or addon libraries need special search paths they can be
	# specified here separated by spaces or one per line using +=
	# ADDON_INCLUDES =

	# any special flag that should be passed to the compiler when using this
	# addon
	# ADDON_CFLAGS =

	# any special flag that should be passed to the linker when using this
	# addon, also used for system libraries with -lname
	# ADDON_LDFLAGS =

	# linux only, any library that should be included in the project using
	# pkg-config
	# ADDON_PKG_CONFIG_LIBRARIES =

	# osx/iOS only, any framework that should be included in the project
	# ADDON_FRAMEWORKS =

	# source files, these will be usually parsed from the file system looking
	# in the src folders in libs and the root of the addon. if your addon needs
	# to include files in different places or a different set of files per platform
	# they can be specified here
	# ADDON_SOURCES =

	# some addons need resources to be copied to the bin/data folder of the project
	# specify here any files that need to be copied, you can use wildcards like * and ?
	# ADDON_DATA =

	# when parsing the file system looking for libraries exclude this for all or
	# a specific platform
	# ADDON_LIBS_EXCLUDE =

	# when parsing the file system looking for sources exclude this for all or
	# a specific platform
	# ADDON_SOURCES_EXCLUDE =

	# when parsing the file system looking for include paths exclude this for all or
	# a specific platform
	# ADDON_INCLUDES_EXCLUDE =

linux64:
	# use fftw by default for Linux 64-bit
	ADDON_PKG_CONFIG_LIBRARIES = fftw3
	ADDON_PKG_CONFIG_LIBRARIES += fftw3f
	ADDON_CFLAGS += -DOFX_FFT_USE_FFTW

linux:
	# use fftw by default for Linux
	ADDON_PKG_CONFIG_LIBRARIES = fftw3
	ADDON_PKG_CONFIG_LIBRARIES += fftw3f
	ADDON_CFLAGS += -DOFX_FFT_USE_FFTW

msys2:
	# use fftw for msys2/mingw
	ADDON_LDFLAGS = -lfftw3
	ADDON_LDFLAGS += -lfftw3f
	ADDON_CFLAGS += -DOFX_FFT_USE_FFTW

vs:
	# use Kiss FFT for Visual Studio as it's easier to setup
	ADDON_CFLAGS += -DOFX_FFT_USE_KISS
	# exclude FFTW files for vs
	ADDON_SOURCES_EXCLUDE = libs/fftw/%

linuxarmv6l:
	# use fftw by default for Raspberry Pi 32-bit
	ADDON_PKG_CONFIG_LIBRARIES = fftw3
	ADDON_PKG_CONFIG_LIBRARIES += fftw3f
	ADDON_CFLAGS += -DOFX_FFT_USE_FFTW

linuxarmv7l:
	# use fftw by default for Raspberry Pi newer models
	ADDON_PKG_CONFIG_LIBRARIES = fftw3
	ADDON_PKG_CONFIG_LIBRARIES += fftw3f
	ADDON_CFLAGS += -DOFX_FFT_USE_FFTW

linuxaarch64:
	# use fftw by default for Raspberry Pi 64-bit (aarch64)
	ADDON_PKG_CONFIG_LIBRARIES = fftw3
	ADDON_PKG_CONFIG_LIBRARIES += fftw3f
	ADDON_CFLAGS += -DOFX_FFT_USE_FFTW

android/armeabi:
	# use Kiss FFT for Android as it's easier to setup
	ADDON_CFLAGS += -DOFX_FFT_USE_KISS
	# exclude FFTW files for Android
	ADDON_SOURCES_EXCLUDE = libs/fftw/%

android/armeabi-v7a:
	# use Kiss FFT for Android as it's easier to setup
	ADDON_CFLAGS += -DOFX_FFT_USE_KISS
	# exclude FFTW files for Android
	ADDON_SOURCES_EXCLUDE = libs/fftw/%

android/arm64-v8a:
	# use Kiss FFT for Android as it's easier to setup
	ADDON_CFLAGS += -DOFX_FFT_USE_KISS
	# exclude FFTW files for Android
	ADDON_SOURCES_EXCLUDE = libs/fftw/%

osx:
	# Configuration for both Intel and Apple Silicon Macs
	# Use Apple's Accelerate framework which works natively on both architectures
	ADDON_FRAMEWORKS = Accelerate
	# Define both flags for backward compatibility
	ADDON_CFLAGS += -DOFX_FFT_USE_APPLE_ACCELERATE -DOFX_FFT_USE_ACCELERATE
	
	# Fix compilation warnings
	ADDON_CFLAGS += -Wno-deprecated-declarations -Wno-format-security
	
	# IMPORTANT - Include KissFFT sources as fallback
	# Many parts of the codebase reference KissFFT even when using Accelerate
	# ADDON_SOURCES_EXCLUDE = libs/kiss/%
	
	# Special build flags for the KissFFT files
	ADDON_CXXFLAGS_KISS = -std=c99
	
	# Make sure we're linking to libm (math library) which KissFFT needs
	ADDON_LDFLAGS += -lm

ios:
	# use Apple's Accelerate framework for iOS (best performance)
	ADDON_FRAMEWORKS = Accelerate
	ADDON_CFLAGS += -DOFX_FFT_USE_APPLE_ACCELERATE
	# exclude both FFTW and KissFFT files for iOS since we're using Accelerate
	ADDON_SOURCES_EXCLUDE = libs/fftw/% libs/kiss/%
