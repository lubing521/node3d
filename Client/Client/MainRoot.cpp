#include "ClientNetWork.h"
#include "MainRoot.h"
#include "RenderSystem.h"
#include "TextRender.h"
#include "IniFile.h"
#include "RegData.h"
#include "GlobalFunction.h"
#include "Audio.h"
#include "protocol.h"
#include "FileSystem.h"
#include "RenderNodeMgr.h"
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// # (leo123) i think files patchs and names of files must be in defines

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CMainRoot::CMainRoot()
{
	#ifdef _DEBUG
	if(LoadRenderSystemLibrary("Plugins\\RenderSystemDX9d.dll") == true)
	#else
	if(LoadRenderSystemLibrary("Plugins\\RenderSystemDX9.dll") == true)
	#endif
	{
		// # InitLua
		InitLua(CUICombo::m_pLuaState);
		// ----
		// UI Theme
		std::string strTheme			= IniGetStr("GameUI.cfg", "UIConfig", "theme");
		std::string strThemeFilename	= "Data\\Themes\\" + strTheme + "\\UIStyle.xml";
		// ----
		GetStyleMgr().Create(strThemeFilename.c_str());
		// ----
		// # UI Font
		std::string strLanguage			= IniGetStr("GameUI.cfg", "UIConfig", "language");
		std::string strFont1			= IniGetStr("Font.cfg",strLanguage.c_str(), "font1");
		// ----
		UIGraph::getInstance().initFont(strFont1.c_str(),13);
		// ----
		// # Create UI
		std::string strUIFilename		= IniGetStr("GameUI.cfg", "UIConfig", "file");
		// ----
		m_dlgMain.create(strUIFilename.c_str(), "IDD_MAIN");
		// ----
		// # Load UI Language
		setUILanguage(strLanguage);
		// ----
		// # (leo123) : i think next variables must be in register
		// # Create Render System Window
		int nWidth						= IniGetInt("Game.cfg", "display", "width", 800);
		int nHeight						= IniGetInt("Game.cfg", "display", "height", 500);
		// ----
		CreateRenderWindow(L"MU2", nWidth, nHeight);
		// ----
		// # Create Common Shared Shader
		if(GetRenderSystem()->GetShaderMgr().createSharedShader("EngineRes/fx/shared.fx") == false)
		{
			MessageBoxW(NULL, L"Can't find the shared fx", L"Error", 0);
		}
		// ----
		// # Common Materials
		CRenderNodeMgr::getInstance().loadRenderNode("Data\\Common.mat.csv",NULL);
		CRenderNodeMgr::getInstance().loadRenderNode("EngineRes\\Common.mat.csv",NULL);
		// ----
		// # NetWork
		NETWORK.SetHWND	(m_hWnd);
		NETWORK.SetWinMsgNum(WM_GM_JOIN_CLIENT_MSG_PROC);
		NETWORK.SetProtocolCore(ProtocolCore);
	}
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CMainRoot::~CMainRoot()
{
	// ----
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CDlgMain& CMainRoot::getMainDialog()
{
	return m_dlgMain;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CMainRoot::setUILanguage(const std::string & strLanguage)
{
	std::string strUIFilename			= IniGetStr("GameUI.cfg", "UIConfig", "file");
	std::string strStringFilename		= ChangeExtension(strUIFilename, "String" + strLanguage + ".ini");
	// ----
	m_dlgMain.loadString(strStringFilename.c_str());
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CMainRoot::OnFrameMove(double fTime, float fElapsedTime)
{
	static double s_LiveTime = 0.0f;
	// ----
	s_LiveTime				+= fElapsedTime;
	// ----
	if(s_LiveTime > 10.0f)
	{
		s_LiveTime = 0.0f;
		// ----
		CSLiveClient();
	}
	// ----
	CRoot::OnFrameMove		(fTime, fElapsedTime);
	m_dlgMain.OnFrameMove	(fTime, fElapsedTime);
	// ----
	GetAudio().FrameMove	(fElapsedTime);
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CMainRoot::OnFrameRender(double fTime, float fElapsedTime)
{
	if(m_pRenderSystem->BeginFrame() == true)
	{
		m_pRenderSystem->SetupRenderState();
		m_pRenderSystem->ClearBuffer(true, true, 0x0);
		// ----
		m_dlgMain.OnFrameRender(Matrix::UNIT, fTime, fElapsedTime);
		// ----
		m_pRenderSystem->EndFrame();
	}
	// ----
	CRoot::OnFrameRender(fTime, fElapsedTime);
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

bool CMainRoot::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bool bNoFurtherProcessing = false;
	// -----
	if (uMsg == WM_GM_JOIN_CLIENT_MSG_PROC)
	{
		switch( lParam & 0xFFFF & 0xFFFF)
		{
			case 1:
			{
				NETWORK.DataRecv();
			}
			break;

			case 2:
			{
				NETWORK.FDWRITE_MsgDataSend();
			}
			break;
		}
	}
	// ----
	bNoFurtherProcessing = m_dlgMain.MsgProc(hWnd, uMsg, wParam, lParam);
	// ----
	return bNoFurtherProcessing;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------