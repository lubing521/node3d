#include "Monster.h"
#include "CSVFile.h"
#include "Audio.h"
#include "RenderNodeMgr.h"
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CMonster::CMonster()
{
	m_nType = 0;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CMonster::~CMonster()
{

}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CMonster::setType(unsigned long uType)
{
	m_uType = uType;
	// ----
	CCsvFile csv(MONSTER_CVS_PATCH);
	// ----
	csv.seek(uType);
	// ----
	const char* szName			= csv.getStr("name","000000");
	const char* szModelFilename	= csv.getStr("model","000000");
	// ----
	float fScale				= csv.getFloat("scale",1.0f);
	// ----
	setName(s2ws(szName).c_str());
	// ----
	CRenderNodeMgr::getInstance().loadRenderNode(szModelFilename,this);
	// ----
	setScale(Vec3D(fScale,fScale,fScale));
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void playMonsterSound(unsigned long uType, const char* szName)
{
	/*for (int i=0; i<10; ++i)
	{
		//CCsvFile csv("Data\\CSV\\MonsterV.csv");
		//csv.seek(uType);
	}*/

	CCsvFile csv(MONSTER_CVS_PATCH);
	// ----
	csv.seek(uType);
	// ----
	GetAudio().playSound(csv.getStr(szName,""));
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CMonster::frameMove(const Matrix& mWorld, double fTime, float fElapsedTime)
{
	float fSoundIdleTime = 10.0f;
	// ----
	if(m_uActionState == WALK)
	{
		// ----
		if(fElapsedTime < fSoundIdleTime)
		{
			if(rand() % (int)(fSoundIdleTime / fElapsedTime) == 0)
			{
				playMonsterSound(m_uType,"soundIdle");
			}
		}
	}
	// ----
	CRole::frameMove(mWorld, fTime, fElapsedTime);
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CMonster::setActionState(unsigned char uActionState)
{
	m_uActionState=uActionState;
	// ----
	switch(m_uActionState)
	{
		case STAND:
		{
			setAnim("0");
		}
		break;

		case WALK:
		{
			setAnim("2");
		}
		break;

		case HIT1:
		{
			playMonsterSound(m_uType,"soundAttack");
			// ----
			setAnim("3");
		}
		break;

		case DIE:
		{
			playMonsterSound(m_uType,"soundDie");
			// ----
			setAnim("6");
		}
		break;
	}
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CMonster::renderFocus()const
{
	C3DMapObj::renderFocus(0xFFFF4040);
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------