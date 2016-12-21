-- This premake script should be used with the orx-customized version of premake4.
-- Its Mercurial repository can be found at https://bitbucket.org/orx/premake-stable.
-- A copy, including binaries, can also be found in the extern/premake folder of any orx distributions.

--
-- Globals
--



function initconfigurations ()
    return
    {
        "Debug",
        "Profile",
        "Release"
    }
end

function initplatforms ()
    if os.is ("windows") then
        if string.lower(_ACTION) == "vs2013" then
            return
            {
                "x64",
                "x32"
            }
        else
            return
            {
                "Native"
            }
        end
    elseif os.is64bit () then
        return
        {
            "x64",
            "x32"
        }
    else
        return
        {
            "x32",
            "x64"
        }
    end
end

function defaultaction (name, action)
   if os.is (name) then
      _ACTION = _ACTION or action
   end
end

defaultaction ("windows", "vs2013")
defaultaction ("linux", "gmake")
defaultaction ("macosx", "gmake")

if os.is ("macosx") then
    osname = "mac"
else
    osname = os.get()
end

destination = "./" .. osname .. "/" .. _ACTION
copybase = path.rebase ("..", os.getcwd (), os.getcwd () .. "/" .. destination)


--
-- Solution: OrxTemplate
--

solution "OrxTemplate"

    language ("C++")

    location (destination)

    kind ("ConsoleApp")

    configurations
    {
        initconfigurations ()
    }

    platforms
    {
        initplatforms ()
    }

    includedirs
    {
        "../include",
        "../include/orx"
    }

    flags
    {
        "NoPCH",
        "NoManifest",
        "EnableSSE2",
        "FloatFast",
        "NoNativeWChar",
        "NoExceptions",
        "Symbols",
        "StaticRuntime"
    }

    configuration {"not vs2013"}
        flags {"EnableSSE2"}

    configuration {"not x64"}
        flags {"EnableSSE2"}

    configuration {"not windows"}
        flags {"Unicode"}

    configuration {"*Debug*"}
        defines {"__orxDEBUG__"}
        links {"orxd"}
        targetsuffix ("d")

    configuration {"*Profile*"}
        defines {"__orxPROFILER__"}
        flags {"Optimize", "NoRTTI"}
        links {"orxp"}
        targetsuffix ("p")

    configuration {"*Release*"}
        flags {"Optimize", "NoRTTI"}
        links {"orx"}


-- Linux

    -- This prevents an optimization bug from happening with some versions of gcc on linux
    configuration {"linux", "not *Debug*"}
        buildoptions {"-fschedule-insns"}


-- Mac OS X

    configuration {"macosx"}
        buildoptions
        {
            "-isysroot /Developer/SDKs/MacOSX10.6.sdk",
            "-mmacosx-version-min=10.6",
            "-gdwarf-2",
            "-Wno-write-strings"
        }
        linkoptions
        {
            "-isysroot /Developer/SDKs/MacOSX10.6.sdk",
            "-mmacosx-version-min=10.6",
            "-dead_strip"
        }

    configuration {"macosx", "x32"}
        buildoptions
        {
            "-mfix-and-continue"
        }


-- Windows


--
-- Project: OrxTemplate
--

project "OrxTemplate"

    files
    {
        "../src/**.c",
        "../src/**.cpp",
        "../include/**.h",
        "../include/**.hpp"
    }
    targetname ("OrxTemplate")


-- Linux

    configuration {"linux"}
        linkoptions {"-Wl,-rpath ./", "-Wl,--export-dynamic"}
        links
        {
            "dl",
            "m",
            "rt"
        }

    configuration {"linux", "x32"}
        libdirs {"../lib/linux32"}
        targetdir ("../bin/linux32")

    configuration {"linux", "x64"}
        libdirs {"../lib/linux64"}
        targetdir ("../bin/linux64")


-- Mac OS X

    configuration {"macosx"}
        links
        {
            "Foundation.framework",
            "AppKit.framework"
        }
        libdirs {"../lib/mac"}
        targetdir ("../bin/mac")


-- Windows

    configuration {"windows"}
        links
        {
            "winmm"
        }

    configuration {"windows", "x64"}
        libdirs {"../lib/windows/64"}
        targetdir ("../bin/windows/64")

    configuration {"windows", "not x64"}
        libdirs {"../lib/windows/32"}
        targetdir ("../bin/windows/32")
