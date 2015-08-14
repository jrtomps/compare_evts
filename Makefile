INSTDIR=/usr/devopt/nscldaq/11.1-conv


#
#  Add your own compilation/link flags here:

USERCXXFLAGS=-std=c++11 -g -I. -I$(INSTDIR)/include
USERCCFLAGS=$(USERCCFLAGS)
USERLDFLAGS=-L$(INSTDIR)/lib -ldaqio -ldaqformatio -ldataformatv11 -lException -Wl,-rpath=$(INSTDIR)/lib

#
#  Add the names of objects you need here if you modified the name of the driver file, 
#  this should also reflect thtat.
#
OBJECTS = main.o FragmentIndex.o

#
#  Modify the line below to provide the name of the library you are trying to build
#  it must be of the form libsomethingorother.so
#

USERFILTER = UserFilter

%.o : %.cpp
	$(CXX) $(USERCXXFLAGS) -c -o $@ $<


$(USERFILTER): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(USERLDFLAGS) $(LDFLAGS)






