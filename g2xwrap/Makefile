CFLAGS=-I../openglide -I. -Werror -msse2 -Os -flto -fomit-frame-pointer
LDFLAGS=-Wl,--disable-auto-image-base -Wl,--dynamicbase -Wl,--nxcompat
LIBS=-L../../qemu-3dfx/wrapper/build -lglide2x.dll
TARGET3=glide3x.dll
TARGET1=glide.dll

all: $(TARGET3) $(TARGET1)
	@echo "  CFLAGS   $(CFLAGS)"
	@echo "  LDFLAGS  $(LDFLAGS)"

$(TARGET3): gl301wrp.def wr301dll.cpp
	@if [ "$$MSYSTEM" != "MINGW32" ]; then echo "Error: MSYSTEM == $$MSYSTEM"; exit 1; fi
	@echo "  CXX  $@"
	@g++ -static -mdll $(CFLAGS) -s -o $@ $^ $(LDFLAGS) $(LIBS)

$(TARGET1): gl211wrp.def wr211dll.cpp
	@if [ "$$MSYSTEM" != "MINGW32" ]; then echo "Error: MSYSTEM == $$MSYSTEM"; exit 1; fi
	@echo "  CXX  $@"
	@g++ -static -mdll $(CFLAGS) -s -o $@ $^ $(LDFLAGS) $(LIBS)

clean:
	@rm -f $(TARGET1) $(TARGET3)
