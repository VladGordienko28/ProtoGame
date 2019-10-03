//-----------------------------------------------------------------------------
//	Test_File.cpp: File Utils tests
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Tests.h"

namespace flu
{
namespace tests
{
	void test_File()
	{
		enter_unit( File );

		// fm::getFilePath
		{
			check( fm::getFilePath( L"" ) == L""  );
			check( fm::getFilePath( L"final.hlsl" ) == L"" );
			check( fm::getFilePath( L"C:\\Program Files\\Flourine\\Launcher.exe" ) == L"C:\\Program Files\\Flourine" );
			check( fm::getFilePath( L"..\\Packages\\System.fpg" ) == L"..\\Packages" );
		}

		// fm::getFileExt
		{
			check( fm::getFileExt( L"" ) == L"" );
			check( fm::getFileExt( L"Palette" ) == L"" );
			check( fm::getFileExt( L"Fire.png" ) == L"png" );
			check( fm::getFileExt( L"..\\Packages\\Weapon.fpg" ) == L"fpg" );
		}

		// fm::getFileName
		{
			check( fm::getFileName( L"" ) == L"" );
			check( fm::getFileName( L"Lemon" ) == L"Lemon" );
			check( fm::getFileName( L"Bzzz.wav" ) == L"Bzzz" );
			check( fm::getFileName( L"C:\\Fluorine" ) == L"" );
			check( fm::getFileName( L"C:\\Fluorine\\MakeBuild.bat" ) == L"MakeBuild" );
			check( fm::getFileName( L"..\\Packages\\System.fpg" ) == L"System" );
		}

		// fm::getFileNameExt
		{
			check( fm::getFileNameExt( L"" ) == L"" );
			check( fm::getFileNameExt( L"Lemon" ) == L"Lemon" );
			check( fm::getFileNameExt( L"Bzzz.wav" ) == L"Bzzz.wav" );
			check( fm::getFileNameExt( L"C:\\Fluorine" ) == L"" );
			check( fm::getFileNameExt( L"C:\\Fluorine\\MakeBuild.bat" ) == L"MakeBuild.bat" );
			check( fm::getFileNameExt( L"..\\Packages\\System.fpg" ) == L"System.fpg" );
		}

		// fm::isAbsoluteFileName
		{
			check( fm::isAbsoluteFileName( L"" ) == false );
			check( fm::isAbsoluteFileName( L"Tralka" ) == false );
			check( fm::isAbsoluteFileName( L"Lalka.png" ) == false );
			check( fm::isAbsoluteFileName( L"C:\\Temp\\b89a.cache" ) == true );
		}

		// fm::normalizeFileName
		{
			check( fm::normalizeFileName( L"" ) == L""  );
			check( fm::normalizeFileName( L"C:\\Fluorine\\Bin\\flu_shell.exe" ) == L"C:\\Fluorine\\Bin\\flu_shell.exe"  );
			check( fm::normalizeFileName( L"C:\\Fluorine/Packages/Movies.fpg" ) == L"C:\\Fluorine\\Packages\\Movies.fpg"  );
		}

		// fm::resolveFileName
		// we can cover only absolute paths in unit tests
		{
			check( fm::resolveFileName( L"", fm::EPathBase::Absolute ) == L"\\" ); // what??
			check( fm::resolveFileName( L"F:\\Fluorine\\Build.bat", fm::EPathBase::Absolute ) == L"F:\\Fluorine\\Build.bat" );
			check( fm::resolveFileName( L"F:\\Fluorine/Bin/Editor.ini", fm::EPathBase::Absolute ) == L"F:\\Fluorine\\Bin\\Editor.ini" );
			check( fm::resolveFileName( L"F:\\Fluorine\\Bin\\..\\Packages\\System.fpg", fm::EPathBase::Absolute ) == L"F:\\Fluorine\\Packages\\System.fpg" );
			check( fm::resolveFileName( L"F:\\Fluorine/Bin/..\\Packages/System.fpg", fm::EPathBase::Absolute ) == L"F:\\Fluorine\\Packages\\System.fpg" );
		}

		leave_unit;
	}
}
}