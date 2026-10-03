cmake_minimum_required(VERSION 3.15)
project(MyApp LANGUAGES CXX)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Add Defines
add_compile_definitions(
    RELEASE
)

# Check for Main Files
file(GLOB MAIN_FILES
    src/*.cpp
)

# Executable Files
add_executable(MyApp
	
	# Main
	${MAIN_FILES}
	
)

# Include Directories
target_include_directories(MyApp PRIVATE 
    src 
	OGRE/include/OGRE
	OGRE/include/OGRE/Bites
	OGRE/include/OGRE/Bullet
	OGRE/include/OGRE/MeshLodGenerator
	OGRE/include/OGRE/Overlay
	OGRE/include/OGRE/Paging
	OGRE/include/OGRE/Plugins
	OGRE/include/OGRE/Property
	OGRE/include/OGRE/RenderSystems
	OGRE/include/OGRE/RTShaderSystem
	OGRE/include/OGRE/Terrain
	OGRE/include/OGRE/Threading
	OGRE/include/OGRE/Volume
)

# Link Directories
target_link_directories(MyApp PRIVATE 

	src
	OGRE/lib
	
)

# Link Libraries
target_link_libraries(MyApp PRIVATE 
    # Windows System Libraries
    opengl32
    User32
    Shell32
    Gdi32
    Kernel32
    Advapi32
    Ole32
    Oleaut32
    uuid
    odbc32
    odbccp32
    
    # OGRE Libraries
    OgreMain
    OgreBites
    OgreBitesQt
    OgreBullet
    OgreMeshLodGenerator
    OgreOverlay
    OgrePaging
    OgreProperty
    OgreRTShaderSystem
    OgreTerrain
    OgreVolume
)


