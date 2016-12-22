sln:
	cd build/ && premake4.exe --os=windows vs2015

build:
	MSBuild build/windows/vs2015/OrxTemplate.sln -p:Configuration=Debug -p:Platform=x64

clean:
	MSBuild build/windows/vs2015/OrxTemplate.sln -t:Clean

.PHONY: sln build clean
