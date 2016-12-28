sbuild:
	../orx/code/build/premake4 --file=build/premake4.lua

sclean:
	rm -rf build/windows

build:
ifeq ($(OS),Windows_NT)
	MSBuild build/windows/vs2015/Game.sln -p:Configuration=Debug -p:Platform=x64
else
	$(MAKE) --no-print-directory -C build/linux/gmake -f Game.make
endif

clean:
ifeq ($(OS),Windows_NT)
	MSBuild build/windows/vs2015/Game.sln -t:Clean
else
	$(MAKE) --no-print-directory -C build/linux/gmake -f Game.make clean
endif

dox:
	cd doc ; doxygen

.PHONY: sbuild sclean build clean dox
