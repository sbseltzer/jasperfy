-- This premake script should be used with orx-customized version of premake4.
-- Its Hg repository can be found at https://bitbucket.org/orx/premake-stable.
-- A copy, including binaries, can also be found in the extern/premake folder.

--
-- Globals
--

-- This is where I define all of the convenience template project configuration.
local templateVars = require ("project")

function islinux64 ()
    local pipe    = io.popen ("uname -m")
    local content = pipe:read ('*a')
    pipe:close ()

    local t64 =
    {
        'x86_64',
        'ia64',
        'amd64',
        'powerpc64',
        'sparc64'
    }

    for i, v in ipairs (t64) do
        if content:find (v) then
            return true
        end
    end

    return false
end

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
        if string.lower(_ACTION) == "vs2013"
        or string.lower(_ACTION) == "vs2015" then
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
    elseif os.is ("linux") then
        if islinux64 () then
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
    elseif os.is ("macosx") then
        if string.find(string.lower(_ACTION), "xcode") then
            return
            {
                "Universal"
            }
        else
            return
            {
                "x32", "x64"
            }
        end
    end
end

function defaultaction (name, action)
   if os.is (name) then
      _ACTION = _ACTION or action
   end
end

defaultaction ("windows", "vs2015")
defaultaction ("linux", "gmake")
defaultaction ("macosx", "gmake")

newoption
{
    trigger = "to",
    value   = "path",
    description = "Set the output location for the generated files"
}

if os.is ("macosx") then
    osname = "mac"
else
    osname = os.get()
end

destination = _OPTIONS["to"] or "./" .. osname .. "/" .. _ACTION
copybase = path.rebase ("..", os.getcwd (), os.getcwd () .. "/" .. destination)
print(destination, os.getcwd(), copybase)

--
-- Solution
--

solution (templateVars.projectname)

    language (templateVars.language)

    location (destination)

    kind (templateVars.console and "ConsoleApp" or "WindowedApp")

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
        "../include/orx",
        "$(ORX)/include",
        "../../orx/code/include/"
    }

    configuration {"not macosx"}
        libdirs {"../lib"}
    configuration {}

    libdirs
    {
      "../lib",
      "../lib/dynamic",
      "$(ORX)/lib",
      "$(ORX)/lib/dynamic",
      "../../orx/code/lib/dynamic"
    }

    targetdir ("../" .. templateVars.targetdir)

    flags
    {
        "NoPCH",
        "NoManifest",
        "FloatFast",
        "NoNativeWChar",
        "NoExceptions",
        "NoIncrementalLink",
        "NoEditAndContinue",
        "NoMinimalRebuild",
        "Symbols",
        "StaticRuntime"
    }

    configuration {"not vs2013", "not vs2015"}
        flags {"EnableSSE2"}

    configuration {"not x64"}
        flags {"EnableSSE2"}

    configuration {"not windows"}
        flags {"Unicode"}

    configuration {"*Debug*"}
        defines {"__orxDEBUG__"}
        links {"orxd"}

    configuration {"*Profile*"}
        defines {"__orxPROFILER__"}
        flags {"Optimize", "NoRTTI"}
        links {"orxp"}

    configuration {"*Release*"}
        flags {"Optimize", "NoRTTI"}
        links {"orx"}


-- Linux

    configuration {"linux"}
        linkoptions {"-Wl,-rpath ./", "-Wl,--export-dynamic"}
        links
        {
            "dl",
            "m",
            "rt"
        }

    -- This prevents an optimization bug from happening with some versions of gcc on linux
    configuration {"linux", "not *Debug*"}
        buildoptions {"-fschedule-insns"}


-- Mac OS X

    configuration {"macosx"}
        buildoptions
        {
            "-mmacosx-version-min=10.6",
            "-gdwarf-2",
            "-Wno-write-strings"
        }
        links
        {
            "Foundation.framework",
            "AppKit.framework"
        }
        linkoptions
        {
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
-- Project
--

project (templateVars.projectname)

    files
    {
      "../src/**.c",
      "../src/**.cpp",
      "../src/**.inl",
      "../include/**.h",
      "../include/**.hpp"
    }

    -- Make some variables for convenience.
    local libDir = copybase .. "/../orx/code/lib/dynamic"
    local binDir = copybase .. "/" .. templateVars.targetdir

-- Linux

    configuration {"linux"}
    postbuildcommands {"$(shell cp -f " .. libDir .. "/liborx*.so " .. binDir .. ")"}


-- Mac OS X

    configuration {"macosx", "xcode*"}
    postbuildcommands {"$(cp -f " .. libDir .. "/liborx*.dylib " .. binDir .. ")"}

    configuration {"macosx", "not xcode*"}
        postbuildcommands {"$(shell cp -f " .. libDir .. "/liborx*.dylib " .. binDir .. ")"}


-- Windows

    configuration {"windows"}
        postbuildcommands {"cmd /c copy /Y " .. path.translate(libDir, "\\") .. "\\orx*.dll " .. path.translate(binDir, "\\")}

