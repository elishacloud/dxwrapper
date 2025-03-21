#include "ddraw-testing.h"

std::ostream& operator<<(std::ostream& os, REFIID riid)
{
#define CHECK_REFIID(riidPrefix, riidName) \
	if (riid == riidPrefix ## _ ## riidName) \
	{ \
		return os << #riidPrefix << "_" << #riidName; \
	}

	CHECK_REFIID(IID, IUnknown);
	CHECK_REFIID(IID, IClassFactory);
	// ddraw
	CHECK_REFIID(CLSID, DirectDraw);
	CHECK_REFIID(CLSID, DirectDraw7);
	CHECK_REFIID(CLSID, DirectDrawClipper);
	CHECK_REFIID(IID, IDirectDraw);
	CHECK_REFIID(IID, IDirectDraw2);
	CHECK_REFIID(IID, IDirectDraw4);
	CHECK_REFIID(IID, IDirectDraw7);
	CHECK_REFIID(IID, IDirectDrawSurface);
	CHECK_REFIID(IID, IDirectDrawSurface2);
	CHECK_REFIID(IID, IDirectDrawSurface3);
	CHECK_REFIID(IID, IDirectDrawSurface4);
	CHECK_REFIID(IID, IDirectDrawSurface7);
	CHECK_REFIID(IID, IDirectDrawPalette);
	CHECK_REFIID(IID, IDirectDrawClipper);
	CHECK_REFIID(IID, IDirectDrawColorControl);
	CHECK_REFIID(IID, IDirectDrawGammaControl);
	// ddrawex
	CHECK_REFIID(IID, IDirectDraw3);
	CHECK_REFIID(CLSID, DirectDrawFactory);
	CHECK_REFIID(IID, IDirectDrawFactory);
	// d3d
	CHECK_REFIID(IID, IDirect3D);
	CHECK_REFIID(IID, IDirect3D2);
	CHECK_REFIID(IID, IDirect3D3);
	CHECK_REFIID(IID, IDirect3D7);
	CHECK_REFIID(IID, IDirect3DRampDevice);
	CHECK_REFIID(IID, IDirect3DRGBDevice);
	CHECK_REFIID(IID, IDirect3DHALDevice);
	CHECK_REFIID(IID, IDirect3DMMXDevice);
	CHECK_REFIID(IID, IDirect3DRefDevice);
	CHECK_REFIID(IID, IDirect3DNullDevice);
	CHECK_REFIID(IID, IDirect3DTnLHalDevice);
	CHECK_REFIID(IID, IDirect3DDevice);
	CHECK_REFIID(IID, IDirect3DDevice2);
	CHECK_REFIID(IID, IDirect3DDevice3);
	CHECK_REFIID(IID, IDirect3DDevice7);
	CHECK_REFIID(IID, IDirect3DTexture);
	CHECK_REFIID(IID, IDirect3DTexture2);
	CHECK_REFIID(IID, IDirect3DLight);
	CHECK_REFIID(IID, IDirect3DMaterial);
	CHECK_REFIID(IID, IDirect3DMaterial2);
	CHECK_REFIID(IID, IDirect3DMaterial3);
	CHECK_REFIID(IID, IDirect3DExecuteBuffer);
	CHECK_REFIID(IID, IDirect3DViewport);
	CHECK_REFIID(IID, IDirect3DViewport2);
	CHECK_REFIID(IID, IDirect3DViewport3);
	CHECK_REFIID(IID, IDirect3DVertexBuffer);
	CHECK_REFIID(IID, IDirect3DVertexBuffer7);

	UINT x = 0;
	char buffer[(sizeof(IID) * 2) + 5] = { '\0' };
	for (size_t j : {3, 2, 1, 0, 0xFF, 5, 4, 0xFF, 7, 6, 0xFF, 8, 9, 0xFF, 10, 11, 12, 13, 14, 15})
	{
		if (j == 0xFF)
		{
			buffer[x] = '-';
		}
		else
		{
			sprintf_s(buffer + x, 3, "%02X", ((byte*)&riid)[j]);
			x++;
		}
		x++;
	}

	return Logging::LogStruct(os) << buffer;
}

std::ostream& operator<<(std::ostream& os, const DDERR& ErrCode)
{
#define VISIT_DDERR_CODES(visit) \
	visit(DD_OK) \
	visit(DDERR_ALREADYINITIALIZED) \
	visit(DDERR_CANNOTATTACHSURFACE) \
	visit(DDERR_CANNOTDETACHSURFACE) \
	visit(DDERR_CURRENTLYNOTAVAIL) \
	visit(DDERR_EXCEPTION) \
	visit(DDERR_GENERIC) \
	visit(DDERR_HEIGHTALIGN) \
	visit(DDERR_INCOMPATIBLEPRIMARY) \
	visit(DDERR_INVALIDCAPS) \
	visit(DDERR_INVALIDCLIPLIST) \
	visit(DDERR_INVALIDMODE) \
	visit(DDERR_INVALIDOBJECT) \
	visit(DDERR_INVALIDPARAMS) \
	visit(DDERR_INVALIDPIXELFORMAT) \
	visit(DDERR_INVALIDRECT) \
	visit(DDERR_LOCKEDSURFACES) \
	visit(DDERR_NO3D) \
	visit(DDERR_NOALPHAHW) \
	visit(DDERR_NOSTEREOHARDWARE) \
	visit(DDERR_NOSURFACELEFT) \
	visit(DDERR_NOCLIPLIST) \
	visit(DDERR_NOCOLORCONVHW) \
	visit(DDERR_NOCOOPERATIVELEVELSET) \
	visit(DDERR_NOCOLORKEY) \
	visit(DDERR_NOCOLORKEYHW) \
	visit(DDERR_NODIRECTDRAWSUPPORT) \
	visit(DDERR_NOEXCLUSIVEMODE) \
	visit(DDERR_NOFLIPHW) \
	visit(DDERR_NOGDI) \
	visit(DDERR_NOMIRRORHW) \
	visit(DDERR_NOTFOUND) \
	visit(DDERR_NOOVERLAYHW) \
	visit(DDERR_OVERLAPPINGRECTS) \
	visit(DDERR_NORASTEROPHW) \
	visit(DDERR_NOROTATIONHW) \
	visit(DDERR_NOSTRETCHHW) \
	visit(DDERR_NOT4BITCOLOR) \
	visit(DDERR_NOT4BITCOLORINDEX) \
	visit(DDERR_NOT8BITCOLOR) \
	visit(DDERR_NOTEXTUREHW) \
	visit(DDERR_NOVSYNCHW) \
	visit(DDERR_NOZBUFFERHW) \
	visit(DDERR_NOZOVERLAYHW) \
	visit(DDERR_OUTOFCAPS) \
	visit(DDERR_OUTOFMEMORY) \
	visit(DDERR_OUTOFVIDEOMEMORY) \
	visit(DDERR_OVERLAYCANTCLIP) \
	visit(DDERR_OVERLAYCOLORKEYONLYONEACTIVE) \
	visit(DDERR_PALETTEBUSY) \
	visit(DDERR_COLORKEYNOTSET) \
	visit(DDERR_SURFACEALREADYATTACHED) \
	visit(DDERR_SURFACEALREADYDEPENDENT) \
	visit(DDERR_SURFACEBUSY) \
	visit(DDERR_CANTLOCKSURFACE) \
	visit(DDERR_SURFACEISOBSCURED) \
	visit(DDERR_SURFACELOST) \
	visit(DDERR_SURFACENOTATTACHED) \
	visit(DDERR_TOOBIGHEIGHT) \
	visit(DDERR_TOOBIGSIZE) \
	visit(DDERR_TOOBIGWIDTH) \
	visit(DDERR_UNSUPPORTED) \
	visit(DDERR_UNSUPPORTEDFORMAT) \
	visit(DDERR_UNSUPPORTEDMASK) \
	visit(DDERR_INVALIDSTREAM) \
	visit(DDERR_VERTICALBLANKINPROGRESS) \
	visit(DDERR_WASSTILLDRAWING) \
	visit(DDERR_DDSCAPSCOMPLEXREQUIRED) \
	visit(DDERR_XALIGN) \
	visit(DDERR_INVALIDDIRECTDRAWGUID) \
	visit(DDERR_DIRECTDRAWALREADYCREATED) \
	visit(DDERR_NODIRECTDRAWHW) \
	visit(DDERR_PRIMARYSURFACEALREADYEXISTS) \
	visit(DDERR_NOEMULATION) \
	visit(DDERR_REGIONTOOSMALL) \
	visit(DDERR_CLIPPERISUSINGHWND) \
	visit(DDERR_NOCLIPPERATTACHED) \
	visit(DDERR_NOHWND) \
	visit(DDERR_HWNDSUBCLASSED) \
	visit(DDERR_HWNDALREADYSET) \
	visit(DDERR_NOPALETTEATTACHED) \
	visit(DDERR_NOPALETTEHW) \
	visit(DDERR_BLTFASTCANTCLIP) \
	visit(DDERR_NOBLTHW) \
	visit(DDERR_NODDROPSHW) \
	visit(DDERR_OVERLAYNOTVISIBLE) \
	visit(DDERR_NOOVERLAYDEST) \
	visit(DDERR_INVALIDPOSITION) \
	visit(DDERR_NOTAOVERLAYSURFACE) \
	visit(DDERR_EXCLUSIVEMODEALREADYSET) \
	visit(DDERR_NOTFLIPPABLE) \
	visit(DDERR_CANTDUPLICATE) \
	visit(DDERR_NOTLOCKED) \
	visit(DDERR_CANTCREATEDC) \
	visit(DDERR_NODC) \
	visit(DDERR_WRONGMODE) \
	visit(DDERR_IMPLICITLYCREATED) \
	visit(DDERR_NOTPALETTIZED) \
	visit(DDERR_UNSUPPORTEDMODE) \
	visit(DDERR_NOMIPMAPHW) \
	visit(DDERR_INVALIDSURFACETYPE) \
	visit(DDERR_NOOPTIMIZEHW) \
	visit(DDERR_NOTLOADED) \
	visit(DDERR_NOFOCUSWINDOW) \
	visit(DDERR_NOTONMIPMAPSUBLEVEL) \
	visit(DDERR_DCALREADYCREATED) \
	visit(DDERR_NONONLOCALVIDMEM) \
	visit(DDERR_CANTPAGELOCK) \
	visit(DDERR_CANTPAGEUNLOCK) \
	visit(DDERR_NOTPAGELOCKED) \
	visit(DDERR_MOREDATA) \
	visit(DDERR_EXPIRED) \
	visit(DDERR_TESTFINISHED) \
	visit(DDERR_NEWMODE) \
	visit(DDERR_D3DNOTINITIALIZED) \
	visit(DDERR_VIDEONOTACTIVE) \
	visit(DDERR_NOMONITORINFORMATION) \
	visit(DDERR_NODRIVERSUPPORT) \
	visit(DDERR_DEVICEDOESNTOWNSURFACE) \
	visit(DDERR_NOTINITIALIZED) \
	visit(DDERR_LOADFAILED) \
	visit(DDERR_BADVERSIONINFO) \
	visit(DDERR_BADPROCADDRESS) \
	visit(DDERR_LEGACYUSAGE) \
	visit(D3DERR_BADMAJORVERSION) \
	visit(D3DERR_BADMINORVERSION) \
	visit(D3DERR_INVALID_DEVICE) \
	visit(D3DERR_INITFAILED) \
	visit(D3DERR_DEVICEAGGREGATED) \
	visit(D3DERR_EXECUTE_CREATE_FAILED) \
	visit(D3DERR_EXECUTE_DESTROY_FAILED) \
	visit(D3DERR_EXECUTE_LOCK_FAILED) \
	visit(D3DERR_EXECUTE_UNLOCK_FAILED) \
	visit(D3DERR_EXECUTE_LOCKED) \
	visit(D3DERR_EXECUTE_NOT_LOCKED) \
	visit(D3DERR_EXECUTE_FAILED) \
	visit(D3DERR_EXECUTE_CLIPPED_FAILED) \
	visit(D3DERR_TEXTURE_NO_SUPPORT) \
	visit(D3DERR_TEXTURE_CREATE_FAILED) \
	visit(D3DERR_TEXTURE_DESTROY_FAILED) \
	visit(D3DERR_TEXTURE_LOCK_FAILED) \
	visit(D3DERR_TEXTURE_UNLOCK_FAILED) \
	visit(D3DERR_TEXTURE_LOAD_FAILED) \
	visit(D3DERR_TEXTURE_SWAP_FAILED) \
	visit(D3DERR_TEXTURE_LOCKED) \
	visit(D3DERR_TEXTURE_NOT_LOCKED) \
	visit(D3DERR_TEXTURE_GETSURF_FAILED) \
	visit(D3DERR_MATRIX_CREATE_FAILED) \
	visit(D3DERR_MATRIX_DESTROY_FAILED) \
	visit(D3DERR_MATRIX_SETDATA_FAILED) \
	visit(D3DERR_MATRIX_GETDATA_FAILED) \
	visit(D3DERR_SETVIEWPORTDATA_FAILED) \
	visit(D3DERR_INVALIDCURRENTVIEWPORT) \
	visit(D3DERR_INVALIDPRIMITIVETYPE) \
	visit(D3DERR_INVALIDVERTEXTYPE) \
	visit(D3DERR_TEXTURE_BADSIZE) \
	visit(D3DERR_INVALIDRAMPTEXTURE) \
	visit(D3DERR_MATERIAL_CREATE_FAILED) \
	visit(D3DERR_MATERIAL_DESTROY_FAILED) \
	visit(D3DERR_MATERIAL_SETDATA_FAILED) \
	visit(D3DERR_MATERIAL_GETDATA_FAILED) \
	visit(D3DERR_INVALIDPALETTE) \
	visit(D3DERR_ZBUFF_NEEDS_SYSTEMMEMORY) \
	visit(D3DERR_ZBUFF_NEEDS_VIDEOMEMORY) \
	visit(D3DERR_SURFACENOTINVIDMEM) \
	visit(D3DERR_LIGHT_SET_FAILED) \
	visit(D3DERR_LIGHTHASVIEWPORT) \
	visit(D3DERR_LIGHTNOTINTHISVIEWPORT) \
	visit(D3DERR_SCENE_IN_SCENE) \
	visit(D3DERR_SCENE_NOT_IN_SCENE) \
	visit(D3DERR_SCENE_BEGIN_FAILED) \
	visit(D3DERR_SCENE_END_FAILED) \
	visit(D3DERR_INBEGIN) \
	visit(D3DERR_NOTINBEGIN) \
	visit(D3DERR_NOVIEWPORTS) \
	visit(D3DERR_VIEWPORTDATANOTSET) \
	visit(D3DERR_VIEWPORTHASNODEVICE) \
	visit(D3DERR_NOCURRENTVIEWPORT) \
	visit(D3DERR_INVALIDVERTEXFORMAT) \
	visit(D3DERR_COLORKEYATTACHED) \
	visit(D3DERR_VERTEXBUFFEROPTIMIZED) \
	visit(D3DERR_VBUF_CREATE_FAILED) \
	visit(D3DERR_VERTEXBUFFERLOCKED) \
	visit(D3DERR_VERTEXBUFFERUNLOCKFAILED) \
	visit(D3DERR_ZBUFFER_NOTPRESENT) \
	visit(D3DERR_STENCILBUFFER_NOTPRESENT) \
	visit(D3DERR_WRONGTEXTUREFORMAT) \
	visit(D3DERR_UNSUPPORTEDCOLOROPERATION) \
	visit(D3DERR_UNSUPPORTEDCOLORARG) \
	visit(D3DERR_UNSUPPORTEDALPHAOPERATION) \
	visit(D3DERR_UNSUPPORTEDALPHAARG) \
	visit(D3DERR_TOOMANYOPERATIONS) \
	visit(D3DERR_CONFLICTINGTEXTUREFILTER) \
	visit(D3DERR_UNSUPPORTEDFACTORVALUE) \
	visit(D3DERR_CONFLICTINGRENDERSTATE) \
	visit(D3DERR_UNSUPPORTEDTEXTUREFILTER) \
	visit(D3DERR_TOOMANYPRIMITIVES) \
	visit(D3DERR_INVALIDMATRIX) \
	visit(D3DERR_TOOMANYVERTICES) \
	visit(D3DERR_CONFLICTINGTEXTUREPALETTE) \
	visit(D3DERR_CONFLICTINGTEXTUREPALETTE) \
	visit(D3DERR_INVALIDSTATEBLOCK) \
	visit(D3DERR_INBEGINSTATEBLOCK) \
	visit(D3DERR_NOTINBEGINSTATEBLOCK) \
	visit(D3DERR_COMMAND_UNPARSED) \
	visit(E_NOINTERFACE) \
	visit(E_POINTER) \
	visit(D3D_OK)

#define VISIT_DDERR_RETURN(x) \
	if (ErrCode == x) \
	{ \
		return os << #x; \
	}

	VISIT_DDERR_CODES(VISIT_DDERR_RETURN);

	return os << Logging::hex((DWORD)ErrCode);
}
