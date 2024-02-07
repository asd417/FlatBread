workspace "FlatBread"
    architecture "x64"
    configurations { 
        "Debug", 
        "Release",
        "Dist"
    }
    language "C++"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "FlatBread"
    language "C++"
    files { "src/**.hpp", "src/**.cpp" }
    kind "ConsoleApp"
    targetdir("build/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.h",
        "src/**.cpp",
    }

    includedirs
    {
        "$(ProjectDir)/dependencies/glfw-3.3.8.bin.WIN64/include",
        "$(ProjectDir)/dependencies/glm",
        "C:/VulkanSDK/1.3.261.1/Include"
    }

    libdirs{
        "$(ProjectDir)/dependencies/glfw-3.3.8.bin.WIN64/lib-vc2022",
        "C:/VulkanSDK/1.3.261.1/Lib"
    }

    links{
        "vulkan-1.lib",
        "glfw3.lib",
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines
        {

        }

        
        filter "configurations:Debug"
            defines ""
            optimize "On"

        filter "configurations:Release"
            defines ""
            optimize "On"

        filter "configurations:Dist"
            defines ""
            optimize "On"
        
        filter {"system:windows","configurations:Debug"}
            buildoptions "/MDd"

        filter {"system:windows","configurations:Release"}
            buildoptions "/MD"

    