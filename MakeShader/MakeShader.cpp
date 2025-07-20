/**
* Copyright (C) 2025 Elisha Riedlinger
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
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <d3d9.h>
#include "d3dx9.h"
#include "Settings\Settings.h"
#include "Utils\Utils.h"
#include "External\Hooking\Hook.h"

HINSTANCE hModule_dll = nullptr;

CONFIG Config;

bool Wrapper::ValidProcAddress(FARPROC) { return false; }

// Function to compile HLSL file
bool CompileShader(const wchar_t* hlslFile, LPD3DXBUFFER* compiledShader, LPCSTR pFunctionName, LPCSTR pProfile)
{
    LPD3DXBUFFER errorMessages = nullptr;
    HRESULT hr = D3DXCompileShaderFromFileW(hlslFile, nullptr, nullptr, pFunctionName, pProfile, 0, compiledShader, &errorMessages, nullptr);

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

// Function to check if a file exists
bool FileExists(const std::string& filePath)
{
    struct stat buffer;
    return (stat(filePath.c_str(), &buffer) == 0);
}

// Function to read the contents of a file
std::string ReadFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file)
    {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Function to generate the header content
std::string GenerateHeaderContent(const char* byteArrayName, LPD3DXBUFFER compiledShader)
{
    std::stringstream headerContent;
    headerContent << "// Automatically generated header file\n\n";
    headerContent << "#pragma once\n\n";

    headerContent << "const unsigned char " << byteArrayName << "[] = {\n";
    const unsigned char* bytes = static_cast<const unsigned char*>(compiledShader->GetBufferPointer());
    const size_t byteCount = compiledShader->GetBufferSize();

    for (size_t i = 0; i < byteCount; ++i)
    {
        headerContent << "0x" << std::hex << static_cast<int>(bytes[i]);
        if (i != byteCount - 1)
            headerContent << ", ";
        if ((i + 1) % 16 == 0)
            headerContent << "\n";
    }

    headerContent << "\n};\n";
    return headerContent.str();
}

// Function to generate header file with shader byte array
bool GenerateHeaderFile(const char* hlslFileName, LPD3DXBUFFER compiledShader, const char* byteArrayName)
{
    // Compute output header file path based on HLSL file name
    std::string hlslFilePath(hlslFileName);
    std::string headerFilePath = hlslFilePath.substr(0, hlslFilePath.find_last_of('.')) + ".h";

    // Generate new header content
    std::string newHeaderContent = GenerateHeaderContent(byteArrayName, compiledShader);

    // Check if the file already exists
    if (FileExists(headerFilePath))
    {
        // Read the existing content
        std::string existingHeaderContent = ReadFile(headerFilePath);

        // Compare the new content with the existing content
        if (newHeaderContent == existingHeaderContent)
        {
            // Contents are the same, no need to overwrite
            return true;
        }
    }

    // Write the new content to the file
    std::ofstream headerFile(headerFilePath);
    if (!headerFile)
    {
        std::cerr << "Failed to create header file: " << headerFilePath << std::endl;
        return false;
    }

    headerFile << newHeaderContent;
    headerFile.close();
    return true;
}

// Main function
int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        std::cerr << "Usage: " << argv[0] << " <HLSL file path> <entry point> <shader model> <Byte array variable name>" << std::endl;
        return 1;
    }

    const char* hlslFileName = argv[1];
    const char* entryPoint = argv[2];
    const char* shaderModel = argv[3];
    const char* byteArrayName = argv[4];

    std::wstring wHlslFileName;
    wHlslFileName.assign(hlslFileName, hlslFileName + strlen(hlslFileName));

    LPD3DXBUFFER compiledShader = nullptr;

    if (!CompileShader(wHlslFileName.c_str(), &compiledShader, entryPoint, shaderModel))
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
