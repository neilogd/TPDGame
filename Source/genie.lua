PsyProjectGameLib( solution().name )
	files { "./AutoGenRegisterReflection.cpp", "./**.h", "./**.inl", "./**.cpp" }
	includedirs { "../External/gpg-cpp-sdk/android/include" }

PsyProjectGameExe( solution().name )
	configuration "android-gcc-arm"
		libdirs {
			"../External/gpg-cpp-sdk/android/lib/c++/armeabi-v7a"
		}
		links {
			
			"gpg",
			"z",
			"atomic",
			"log"
		}

PsyProjectImporterExe( solution().name )
