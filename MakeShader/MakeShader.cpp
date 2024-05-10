/**
* Copyright (C) 2024 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#define INITGUID
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <d3d9.h>
#include "d3dx9.h"
#include "Settings\Settings.h"
#include "Utils\Utils.h"
#include "External\Hooking\Hook.h"

HINSTANCE hModule_dll = nullptr;

CONFIG Config;

bool Wrapper::ValidProcAddress(FARPROC) { return false; }

void* Hook::IATPatch(HMODULE, DWORD, const char*, void*, const char*, void*) { return nullptr; }

bool Hook::UnhookIATPatch(HMODULE, DWORD, const char*, void*, const char*, void*) { return false; }

bool Hook::UnIATPatchAll() { return false; }

// Function to compile HLSL file
bool CompileShader(const wchar_t* hlslFile, LPD3DXBUFFER* compiledShader)
{
    LPD3DXBUFFER errorMessages = nullptr;
    HRESULT hr = D3DXCompileShaderFromFileW(hlslFile, nullptr, nullptr, "main", "ps_2_0", 0, compiledShader, &errorMessages, nullptr);

    if (FAILED(hr))
    {
        if (errorMessages)
        {
            std::cerr << "Shader compilation failed: " << static_cast<const char*>(errorMessages->GetBufferPointer()) << std::endl;
            errorMessages->Release();
        }
        return false;
    }

    return true;
}

// Function to generate header file with shader byte array
bool GenerateHeaderFile(const char* hlslFileName, LPD3DXBUFFER compiledShader, const char* byteArrayName)
{
    // Compute output header file path based on HLSL file name
    std::string hlslFilePath(hlslFileName);
    std::string headerFilePath = hlslFilePath.substr(0, hlslFilePath.find_last_of('.')) + ".h";

    std::ofstream headerFile(headerFilePath);
    if (!headerFile)
    {
        std::cerr << "Failed to create header file: " << headerFilePath << std::endl;
        return false;
    }

    headerFile << "// Automatically generated header file\n\n";
    headerFile << "#pragma once\n\n";

    headerFile << "const unsigned char " << byteArrayName << "[] = {\n";
    const unsigned char* bytes = static_cast<const unsigned char*>(compiledShader->GetBufferPointer());
    const size_t byteCount = compiledShader->GetBufferSize();

    for (size_t i = 0; i < byteCount; ++i)
    {
        headerFile << "0x" << std::hex << static_cast<int>(bytes[i]);
        if (i != byteCount - 1)
            headerFile << ", ";
        if ((i + 1) % 16 == 0)
            headerFile << "\n";
    }

    headerFile << "\n};\n";

    headerFile.close();
    return true;
}

// Main function
int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <HLSL file path> <Byte array variable name>" << std::endl;
        return 1;
    }

    const char* hlslFileName = argv[1];
    const char* byteArrayName = argv[2];

    std::wstring wHlslFileName;
    wHlslFileName.assign(hlslFileName, hlslFileName + strlen(hlslFileName));

    LPD3DXBUFFER compiledShader = nullptr;

    if (!CompileShader(wHlslFileName.c_str(), &compiledShader))
    {
        std::cout << "Failed to compile shader." << std::endl;
        return 1;
    }

    if (!GenerateHeaderFile(hlslFileName, compiledShader, byteArrayName))
    {
        compiledShader->Release();
        std::cout << "Failed to create header." << std::endl;
        return 1;
    }

    compiledShader->Release();

    std::cout << "Shader successfully compiled and header file generated." << std::endl;

    return 0;
}
