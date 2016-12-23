sbuild:
	../orx/code/build/premake4 --file=build/premake4.lua

sclean:
	rm -rf build/windows

build:
	MSBuild build/windows/vs2015/Game.sln -p:Configuration=Debug -p:Platform=x64

clean:
	MSBuild build/windows/vs2015/Game.sln -t:Clean

.PHONY: sbuild sclean build clean
