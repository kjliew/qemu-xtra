CFLAGS=-I../build -I../openglide -I. -Werror -msse2 -Os -flto -fomit-frame-pointer
LDFLAGS=-Wl,--disable-auto-image-base -Wl,--dynamicbase -Wl,--nxcompat
LIBS=-L. -lglide2x.dll
TARGET3=glide3x.dll
TARGET1=glide.dll
PEXPORTS=pexports

.PHONY: sanity clean

all: sanity $(TARGET3) $(TARGET1)
	@echo "  CFLAGS   $(CFLAGS)"
	@echo "  LDFLAGS  $(LDFLAGS)"

sanity:
	@if [ "$$MSYSTEM" != "MINGW32" ]; then echo "Error: MSYSTEM == $$MSYSTEM"; exit 1; fi
	$(PEXPORTS) ../build/glide2x.dll | sed "s/^_//" > /tmp/dll.def
	dlltool -U -d /tmp/dll.def -l libglide2x.dll.a

$(TARGET3): gl301wrp.def wr301dll.cpp
	@echo "  CXX  $@"
	@g++ -static -mdll $(CFLAGS) -s -o $@ $^ $(LDFLAGS) $(LIBS)

$(TARGET1): gl211wrp.def wr211dll.cpp
	@echo "  CXX  $@"
	@g++ -static -mdll $(CFLAGS) -s -o $@ $^ $(LDFLAGS) $(LIBS)

clean:
	@rm -f $(TARGET1) $(TARGET3) *.a
