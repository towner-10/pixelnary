-- premake5.lua
workspace "ScribbleServer"
    configurations { "Debug", "Release" }

project "ScribbleServer"
    kind "ConsoleApp"
    language "C++"
    architecture "x64"
    targetdir "bin/%{cfg.buildcfg}"

    files { "**.h", "**.hpp", "**.c", "**.cpp" }
    includedirs { "Dependencies/asio-1.28.0/include" }
    links { "pthread" }

    filter { "system:windows", "action:gmake2" }
        links { "ws2_32" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

