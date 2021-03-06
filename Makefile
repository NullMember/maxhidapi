OSXOPTIONS := -it --rm -v $(CURDIR):/workspace -w /workspace -e CROSS_TRIPLE=osx64 multiarch/crossbuild:dev
OSXCMD := cc -std=c99 -arch i386 -arch x86_64 -shared src/maxhidapi.c libs/hidapi/mac/hid.c -DMAC_VERSION \
-I libs/max/c74support/max-includes -I libs/max/c74support/max-includes/x64 -I libs/hidapi/hidapi \
-F libs/max/c74support/max-includes @libs/max/c74support/max-includes/c74_linker_flags.txt \
-framework MaxAPI -framework IOKit -framework CoreFoundation -o externals/hidapi.mxo/hidapi

WIN32OPTIONS := -it --rm -v $(CURDIR):/workspace -w /workspace -e CROSS_TRIPLE=win32 multiarch/crossbuild:dev
WIN32CMD := cc -shared src/maxhidapi.c libs/hidapi/windows/hid.c -DWIN_VERSION -DWIN_EXT_VERSION \
-I libs/max/c74support/max-includes -I libs/max/c74support/max-includes -I libs/hidapi/hidapi \
-L libs/max/c74support/max-includes -l MaxAPI -l setupapi -o externals/hidapi.mxe

WIN64OPTIONS := -it --rm -v $(CURDIR):/workspace -w /workspace -e CROSS_TRIPLE=win64 multiarch/crossbuild:dev
WIN64CMD := cc -shared src/maxhidapi.c libs/hidapi/windows/hid.c -DWIN_VERSION -DWIN_EXT_VERSION \
-I libs/max/c74support/max-includes -I libs/max/c74support/max-includes/x64 -I libs/hidapi/hidapi \
-L libs/max/c74support/max-includes/x64 -l MaxAPI -l setupapi -o externals/hidapi.mxe64

all: build

build:
	docker run $(OSXOPTIONS) $(OSXCMD)
	docker run $(WIN32OPTIONS) $(WIN32CMD)
	docker run $(WIN64OPTIONS) $(WIN64CMD)