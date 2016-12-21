sln:
	cd build/ && `pwd`/premake4.exe --os=windows vs2015

game:
	MSBuild build/windows/vs2015/OrxTemplate.sln
