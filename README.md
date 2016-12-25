# Orx Template Project

This project is a template project that I'm developing as I follow the orx tutorial series and reflects an evolution of my development preferences. Perhaps you'll like it and perhaps adapt it to your own preferences.

# Usage

## File Locations

Each directory contains, at the very least, a README.md file to explain its purpose and how/when to use it. 

I use orx's bootstrap registration to set a custom default path for config files. I prefer keeping versioned game content outside of the bin folder, which means keeping config files outside the bin folder. This is why `Game.ini` is in the base directory of this project. You can, of course, move it to the bin directory and it will still work.

## Notes on .gitignore and .hgignore

I've set this project up to ignore premake-generated files in the `build` folder, non-config files in the `bin` folder, everything but the readme in the `lib` folder, and the `include/orx` folder.

## Project Setup

### Running Premake

Start by running the premake file in the build folder.

It uses a premake file that has been tweaked to my liking.

### Orx Libraries/Includes

Ensure orx's libaries and includes can be found during compilation.

There are 3 ways to tell this project how to find the orx libraries/includes.

1. Download or clone orx's source code repository and place it as a sibling directory to this one. Ensure that `../orx/code/` is a valid path from this template project folder. Make sure you build the libraries so that the `../orx/code/lib` directory has content.
2. Put things wherever you want and define an environment variable called `ORX` that points to a folder that contains `include/` and `lib/`. In this case it's feasible to download prebuilts. For example, `export ORX=c:/custom/path/to/orx/code` or `export ORX=../my_orx_prebuilts/orx-nightly-2016-12-21/dev-vs2015-64`.
3. Manually copy and paste the relevant orx's libraries to this project's `lib` folder, and copy orx's includes to this project's `include/orx` folder.

### Compiling

There is a Makefile at the root of this project. You are welcome to use whatever you like. Premake will likely configure an IDE for you to use. I use Spacemacs which has a feature that integrates with Makefiles, which is why I created it. It simply adds conveniences for my development, particularly on Windows.

If you want to use that Makefile, you'll need to modify your environment and install some programs. On Windows I use Git Bash for scripting and Visual Studio 2015 for its compiler. This Makefile assumes you are using a Visual Studio 2015 project config and have Git Bash, GNU Make, and MSBuild in your path.

