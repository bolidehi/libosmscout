bin_PROGRAMS = TravelJinni

TravelJinni_SOURCES = DatabaseTask.h \
                      DatabaseTask.cpp \
                      Configuration.h \
                      Configuration.cpp \
                      LocationSearchDialog.h \
                      LocationSearchDialog.cpp \
                      RouteDialog.h \
                      RouteDialog.cpp \
                      Viewer.cpp

TravelJinni_CXXFLAGS=-I. \
                      -DAPP_DATADIR=\"$(pkgdatadir)\" \
                      $(LIBILLUMINATION_CFLAGS) \
                      $(LIBOSMSCOUT_CFLAGS)

TravelJinni_LDADD=$(LIBILLUMINATION_LIBS) \
                  $(LIBOSMSCOUT_LIBS)

