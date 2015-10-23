if PsyProjectExternalLib( "gpg-cpp-sdk", "C" ) then
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		files { "./gpg-cpp-sdk/android/include/**.h" }
		includedirs { "./gpg-cpp-sdk/android/include" }

end
