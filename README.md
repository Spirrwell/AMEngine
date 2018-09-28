# AMEngine

This is an incredibly work in progress 3D game engine.

The ultimate end goal is to create a 3D game engine inspired heavily by the Source Engine, but is fully open source and HOPEFULLY a lot simpler and easier to use.

The code used should be mostly cross platform friendly, and so there shouldn't be any reason you couldn't set this up to work on Mac. (Would have to use MoltenVK for Vulkan support) However, only Windows and Linux will be officially supported.

# (Windows) Project File Generation
Projects will be generated using Premake. Simply execute the "createprojects.bat" to generate a VS2017 solution & projects.

# (Linux) Project File Generation
Projects will be generated using Premake. You may execute "createprojects.sh" to generate makefile projects. If you would like to generate projects for an IDE like CodeLite, simply edit "createprojects.sh" and replace "gmake" with "codelite"

# Other Notes
This engine will not be supporting 32 bit systems.

Project uses C++17.

IMPORTANT: Please review the licenses of thirdparty libraries and code in their respective folders under the "source/thirdparty" folder.
