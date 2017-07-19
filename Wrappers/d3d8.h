#pragma once

#define VISIT_D3D8_PROCS(visit) \
	visit(Direct3DCreate8) \
	visit(ValidateVertexShader) \
	visit(ValidatePixelShader) \
	visit(DebugSetMute)

class d3d8_dll
{
public:
	d3d8_dll() { };
	~d3d8_dll() { };

	void Load();

	void Set_DebugSetMute(FARPROC ProcAddress)
	{
		DebugSetMute = ProcAddress;
	}

	HMODULE dll = nullptr;

private:
	VISIT_D3D8_PROCS(ADD_FARPROC_MEMBER);
};

extern d3d8_dll d3d8;
