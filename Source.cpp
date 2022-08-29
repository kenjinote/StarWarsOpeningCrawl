#pragma comment(lib, "d3d9")
#pragma comment(lib, "d3dx9")

#include <windows.h>
#include <d3dx9.h>
#include <vector>

TCHAR szClassName[] = TEXT("Window");

LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
D3DXMATRIXA16 matWorld, matPosition[4];
FLOAT time = 0;
BOOL active = TRUE;

class TextPool
{
public:
	std::vector<LPD3DXMESH> m_pTextMeshList;
	D3DMATERIAL9 m_TextMeshMaterials;
	BOOL m_bDrawed;
	TextPool()
	{
		ZeroMemory(&m_TextMeshMaterials, sizeof(D3DMATERIAL9));
		m_TextMeshMaterials.Diffuse.r = 1.0f;
		m_TextMeshMaterials.Diffuse.g = 1.0f;
		m_TextMeshMaterials.Diffuse.b = 1.0f;
		m_TextMeshMaterials.Diffuse.a = 1.0f;
		m_TextMeshMaterials.Power = 120.0f;
		m_bDrawed = FALSE;
	}
	~TextPool()
	{
		for (auto v : m_pTextMeshList)
		{
			if (v) v->Release();
		}
	}
	VOID Add(LPCTSTR lpszText)
	{
		LPD3DXMESH pMesh;
		HDC hdc = CreateCompatibleDC(NULL);
		HFONT hFont;
		LOGFONT lf;
		ZeroMemory(&lf, sizeof(lf));
		lf.lfHeight = 16;
		lf.lfStrikeOut = 1;
		lstrcpy(lf.lfFaceName, TEXT("Impact"));
		hFont = CreateFontIndirect(&lf);
		SelectObject(hdc, hFont);
		D3DXCreateText(g_pd3dDevice, hdc, lpszText, 10.0000f, 1.0000f, &pMesh, NULL, NULL);
		DeleteObject(hFont);
		DeleteDC(hdc);
		m_pTextMeshList.push_back(pMesh);
	}
	VOID Draw(DWORD dwTime)
	{
		if (m_bDrawed) return;
		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
		if (SUCCEEDED(g_pd3dDevice->BeginScene()))
		{
			time = (dwTime / 1000.0f - 5.0f);
			//ワールドトランスフォーム（絶対座標変換）
			D3DXMatrixIdentity(&matWorld);
			// ビュートランスフォーム（視点座標変換）
			D3DXVECTOR3 vecEyePt(0.0f, -12.0f, -12.0f); //カメラ（視点）位置
			D3DXVECTOR3 vecLookatPt(0.0f, 0.0f, 0.0f);//注視位置
			D3DXVECTOR3 vecUpVec(0.0f, 1.0f, 0.0f);//上方位置
			D3DXMATRIXA16 matView, matHeading, matCameraPos;
			for (auto pMesh : m_pTextMeshList)
			{
				D3DXMatrixIdentity(&matView);
				D3DXMatrixTranslation(&matHeading, -5.0f, time, 4.0f);
				D3DXMatrixLookAtLH(&matCameraPos, &vecEyePt, &vecLookatPt, &vecUpVec);
				D3DXMatrixMultiply(&matView, &matView, &matHeading);
				D3DXMatrixMultiply(&matView, &matView, &matCameraPos);
				g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);
				D3DXMATRIXA16 matProj;
				D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
				g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
				if (time > 10.0f)
				{
					m_TextMeshMaterials.Diffuse.r = max(0.0f, 1 - (time - 10.0f) / 10.0f);
					m_TextMeshMaterials.Diffuse.g = max(0.0f, 1 - (time - 10.0f) / 10.0f);
					m_TextMeshMaterials.Diffuse.b = max(0.0f, 1 - (time - 10.0f) / 10.0f);
				}
				else
				{
					m_TextMeshMaterials.Diffuse.r = 1;
					m_TextMeshMaterials.Diffuse.g = 1;
					m_TextMeshMaterials.Diffuse.b = 1;
				}
				g_pd3dDevice->SetMaterial(&m_TextMeshMaterials);
				pMesh->DrawSubset(0);
				if (pMesh == m_pTextMeshList[m_pTextMeshList.size() - 1] && max(0.0f, 1 - (time - 10.0) / 10.0f) <= 0.0f)
				{
					m_bDrawed = TRUE;
				}
				else
				{
					time -= 1.5f;
				}
			}
			g_pd3dDevice->EndScene();
			g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
		}
	}
};
TextPool* m_pTextPool;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))return -1;
		D3DPRESENT_PARAMETERS d3dpp;
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.Windowed = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		d3dpp.EnableAutoDepthStencil = TRUE;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		{
			HRESULT hr = E_FAIL;
			DWORD QualityBackBuffer = 0;
			DWORD QualityZBuffer = 0;
			DWORD m = (DWORD)D3DMULTISAMPLE_16_SAMPLES;
			while (m)
			{
				hr = g_pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT,
					D3DDEVTYPE_HAL,
					D3DFMT_X8R8G8B8,
					d3dpp.Windowed,
					(D3DMULTISAMPLE_TYPE)m,
					&QualityBackBuffer);
				if (SUCCEEDED(hr))
				{
					hr = g_pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT,
						D3DDEVTYPE_HAL,
						D3DFMT_X8R8G8B8,
						d3dpp.Windowed,
						(D3DMULTISAMPLE_TYPE)m,
						&QualityZBuffer);
					if (SUCCEEDED(hr))
					{
						d3dpp.MultiSampleType = (D3DMULTISAMPLE_TYPE)m;
						d3dpp.MultiSampleQuality = min(QualityBackBuffer, QualityZBuffer) - 1;
						break;
					}
				}
				m--;
			}
		}
		if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice)))return -1;
		m_pTextPool = new TextPool;
		m_pTextPool->Add(TEXT("Episode I"));
		m_pTextPool->Add(TEXT("THE PHANTOM MENACE"));
		m_pTextPool->Add(TEXT("Turmoil has engulfed the"));
		m_pTextPool->Add(TEXT("Galactic  Republic.  The"));
		m_pTextPool->Add(TEXT("taxation of trade routes"));
		m_pTextPool->Add(TEXT("to outlying star systems"));
		m_pTextPool->Add(TEXT("is  in  dispute."));
		m_pTextPool->Add(TEXT("Hoping  to  resolve  the"));
		m_pTextPool->Add(TEXT("matter  with  a blockade"));
		m_pTextPool->Add(TEXT("of  deadly  battleships,"));
		m_pTextPool->Add(TEXT("the   greedy   Trade"));
		m_pTextPool->Add(TEXT("Federation  has  stopped"));
		m_pTextPool->Add(TEXT("all   shipping   to  the"));
		m_pTextPool->Add(TEXT("small  planet  of  Naboo."));
		m_pTextPool->Add(TEXT("While  the  Congress  of"));
		m_pTextPool->Add(TEXT("the  Republic  endlessly"));
		m_pTextPool->Add(TEXT("debates   this  alarming"));
		m_pTextPool->Add(TEXT("chain   of  events,  the"));
		m_pTextPool->Add(TEXT("Supreme  Chancellor  has"));
		m_pTextPool->Add(TEXT("secretly  dispatched two"));
		m_pTextPool->Add(TEXT("Jedi   Knights,   the"));
		m_pTextPool->Add(TEXT("guardians  of  peace and"));
		m_pTextPool->Add(TEXT("justice  in  the  galaxy,"));
		m_pTextPool->Add(TEXT("to settle the conflict...."));
		{
			g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
			D3DXVECTOR3 vecDirection(0, 0, 1);
			D3DLIGHT9 light;
			ZeroMemory(&light, sizeof(D3DLIGHT9));
			light.Type = D3DLIGHT_DIRECTIONAL;
			light.Diffuse.r = 1.0f;
			light.Diffuse.g = 200.0f / 255.0f;
			light.Diffuse.b = 77.0f / 255.0f;
			D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vecDirection);
			light.Range = 200.0f;
			g_pd3dDevice->SetLight(0, &light);
			g_pd3dDevice->LightEnable(0, TRUE);
		}
		g_pd3dDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
		break;
	case WM_ACTIVATE:
		active = !HIWORD(wParam);
		break;
	case WM_DESTROY:
		delete m_pTextPool;
		if (g_pd3dDevice)g_pd3dDevice->Release();
		if (g_pD3D)g_pD3D->Release();
		PostQuitMessage(0);
		break;
	case WM_SYSCOMMAND:
		switch (wParam)
		{
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			return 0;
		}
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	MSG msg = {};
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		0,
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("Window"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
		);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	BOOL done = FALSE;
	const DWORD dwStartTime = (DWORD)GetTickCount64();
	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				done = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else if (active)
		{
			m_pTextPool->Draw((DWORD)GetTickCount64() - dwStartTime);
		}
	}
	return msg.wParam;
}
