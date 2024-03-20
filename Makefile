CXX=g++
SOURCES=app.cpp frame.cpp
OBJECTS=$(subst .cpp,.o,$(SOURCES))

# wx-config --cxxflags --libs
CPPFLAGS=-I/usr/lib/x86_64-linux-gnu/wx/include/gtk3-unicode-3.0 -I/usr/include/wx-3.0 -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__ -pthread
LDFLAGS=-L/usr/lib/x86_64-linux-gnu -pthread   -lwx_gtk3u_xrc-3.0 -lwx_gtk3u_html-3.0 -lwx_gtk3u_qa-3.0 -lwx_gtk3u_adv-3.0 -lwx_gtk3u_core-3.0 -lwx_baseu_xml-3.0 -lwx_baseu_net-3.0 -lwx_baseu-3.0 

CPPFLAGS+= -g

all: t

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $<

t: $(OBJECTS)
	$(CXX) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf t $(OBJECTS)

