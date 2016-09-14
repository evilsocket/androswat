TARGET    = androswat
MAIN_SRCS = $(wildcard src/*.cpp) $(wildcard src/*/*.cpp)
MAIN_OBJS = $(MAIN_SRCS:.cpp=.o)

# you can override this
ifndef ANDROID_SYSROOT
SYSROOT = /Users/evilsocket/android/ndk/platforms/android-18/arch-arm/
else
SYSROOT = $(ANDROID_SYSROOT)
endif

ifndef ANDROID_STLPORT_INCLUDES
STLPORT_INC = /Users/evilsocket/Library/Android/ndk/sources/cxx-stl/stlport/stlport/
else
STLPORT_INC = $(ANDROID_STLPORT_INCLUDES)
endif

ifndef ANDROID_STLPORT_LIBS
STLPORT_LIBS = /Users/evilsocket/Library/Android/ndk/sources/cxx-stl/stlport/libs/armeabi/
else
STLPORT_LIBS = $(ANDROID_STLPORT_LIBS)
endif

PREFIX   = arm-linux-androideabi-
CXX		 = $(PREFIX)g++
CXXFLAGS = -I. -Iinclude -I$(STLPORT_INC) -L$(STLPORT_LIBS) -fPIC -fPIE -pie --sysroot $(SYSROOT)
LDFLAGS  = -llog -lstlport_static

all: $(MAIN_OBJS)
	@$(CXX) $(CXXFLAGS) -o $(TARGET) $(MAIN_OBJS) $(LDFLAGS)

install: all
	@adb push $(TARGET) /data/local/tmp/
	@adb shell chmod 777 /data/local/tmp/$(TARGET)

help: install
	@clear
	@adb shell su -c /data/local/tmp/$(TARGET) --help

show: install
	@clear
	@adb shell su -c /data/local/tmp/$(TARGET) -n "com.android.calculator2" --show

read: install
	@clear
	@adb shell su -c /data/local/tmp/$(TARGET) -n "com.android.calculator2" --read 74f53000 --size 1024

dump: install
	@clear
	@adb shell su -c /data/local/tmp/$(TARGET) -n "com.android.calculator2" --dump 400e8000 --output /data/local/tmp/test.dump

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	@rm -f src/*.o $(TARGET)
