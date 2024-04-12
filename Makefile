CC=gcc
CXX=g++
CSOURCES=db.c clib.c expense.c
CPPSOURCES=App.cpp Frame.cpp EditExpenseDialog.cpp
COBJECTS=$(patsubst %.c, %.o, $(CSOURCES))
CPPOBJECTS=$(patsubst %.cpp, %.o, $(CPPSOURCES))
OBJECTS=$(COBJECTS) $(CPPOBJECTS) sqlite3.o

# wx-config --cxxflags --libs
WX_CXXFLAGS=-I/usr/local/lib/wx/include/gtk3-unicode-static-3.3 -I/usr/local/include/wx-3.3 -D_FILE_OFFSET_BITS=64 -D__WXGTK__ -pthread

# wx-config --libs all  (all wx libs)
# wx-config --libs std  (default wx libs - core,base)
#
# wx-config --libs std,propgrid
WX_LIBS=-L/usr/local/lib -pthread   /usr/local/lib/libwx_gtk3u_propgrid-3.3.a /usr/local/lib/libwx_gtk3u_xrc-3.3.a /usr/local/lib/libwx_gtk3u_qa-3.3.a /usr/local/lib/libwx_baseu_net-3.3.a /usr/local/lib/libwx_gtk3u_html-3.3.a /usr/local/lib/libwx_gtk3u_core-3.3.a /usr/local/lib/libwx_baseu_xml-3.3.a /usr/local/lib/libwx_baseu-3.3.a -lX11 -lgthread-2.0 -pthread -lXxf86vm -lSM -lxkbcommon -lgtk-3 -lgdk-3 -lpangocairo-1.0 -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lXtst -lpangoft2-1.0 -lpango-1.0 -lgobject-2.0 -lglib-2.0 -lharfbuzz -lfontconfig -lfreetype -lexpat -lpcre2-32 -lpng -ljpeg -ltiff -ljbig -lz -lcurl -lm 

#CPPFLAGS=-Wall -Werror -g -Wno-deprecated-declarations #-fpermissive -Werror=write-strings

CFLAGS= -std=gnu99 -Wall -Werror -Wno-unused -Wno-deprecated-declarations
CPPFLAGS=-g -Wall -Werror -Wno-deprecated-declarations
CPPFLAGS+= $(WX_CXXFLAGS)
LDFLAGS=$(WX_LIBS)

#.SILENT:
all: t

dep:
	echo no deps

sqlite3.o: sqlite3/sqlite3.c
	$(CC) -c -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.cpp
	$(CXX) -c $(CPPFLAGS) -o $@ $<

t: $(OBJECTS)
	#$(CXX) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf t $(OBJECTS)

