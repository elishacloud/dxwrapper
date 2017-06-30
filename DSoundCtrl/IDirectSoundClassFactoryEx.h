/**
* Copyright (c) 2003-2007, Arne Bockholdt, github@bockholdt.info
*
* This file is part of Direct Sound Control.
*
* Direct Sound Control is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Direct Sound Control is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with Direct Sound Control.  If not, see <http://www.gnu.org/licenses/>.
*
* Updated 2017 by Elisha Riedlinger
*/

#pragma once

#define IDIRECTSOUNDCLASSFACTORYEX_CLASS_NAME	"IDirectSoundClassFactoryEx"

/**
* Copyright (c) 2003-2007, Arne Bockholdt, Ammerbachstr. 87, 64372 Ober-Ramstadt, Germany
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/

#ifdef _DEBUG
extern void LogMessage(const char* szClassName, void* pInstance, char* szMessage);
extern bool	g_bLogSystem;
#endif // _DEBUG


#ifdef __cplusplus
extern "C" {  // only need to export C interface if
			  // used by C++ source code
#endif

	extern DirectSoundCreatefunc g_pDirectSoundCreate;
	extern DirectSoundCreate8func g_pDirectSoundCreate8;

#ifdef __cplusplus
}
#endif

class IDirectSoundClassFactoryEx : public IClassFactory
{
public:
	IDirectSoundClassFactoryEx(void);
	virtual ~IDirectSoundClassFactoryEx(void);

	virtual HRESULT __stdcall QueryInterface(REFIID, LPVOID *);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	virtual HRESULT __stdcall CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject);
	virtual HRESULT __stdcall LockServer(BOOL fLock);

	LPCLASSFACTORY m_lpClassFactory;

protected:
#ifdef _DEBUG
	const char* m_cszClassName;
#endif // _DEBUG
};
