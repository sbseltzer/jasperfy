sln:
	cd build/ && premake4.exe --os=windows vs2015

game:
	MSBuild build/windows/vs2015/OrxTemplate.sln -p:Configuration=Release -p:Platform=x64
