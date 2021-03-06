#include "Role.h"
#include "MainRoot.h"

CRole::CRole()
{
	m_fHeadRotate = 0.0f;
	m_fCurrentHeadRotate = 0.0f;
	m_uActionState = STAND;
	m_fOneWalkLength = 2.35f;
}

CRole::~CRole()
{
}
#define DIR_COUNT 8
const int DX[DIR_COUNT] = { 0, 1, 1, 1, 0,-1,-1,-1};
const int DY[DIR_COUNT] = {-1,-1, 0, 1, 1, 1, 0,-1};

#define DIR_LENGTH(a) sqrt((float)DX[a]*DX[a]+(float)DY[a]*DY[a])
const float DirLength[DIR_COUNT] = {
	DIR_LENGTH(0),
	DIR_LENGTH(1),
	DIR_LENGTH(2),
	DIR_LENGTH(3),
	DIR_LENGTH(4),
	DIR_LENGTH(5),
	DIR_LENGTH(6),
	DIR_LENGTH(7),
};

#define DIR_NORMALIZE(a) Vec3D((float)DX[a],0.0f,(float)DY[a]).normalize()

const Vec3D DirNormalize[DIR_COUNT] = {
	DIR_NORMALIZE(0),
	DIR_NORMALIZE(1),
	DIR_NORMALIZE(2),
	DIR_NORMALIZE(3),
	DIR_NORMALIZE(4),
	DIR_NORMALIZE(5),
	DIR_NORMALIZE(6),
	DIR_NORMALIZE(7),
};

void CRole::OnFrameMove(float fElapsedTime)
{
	if (!CModelComplex::isCreated())
	{
		CModelComplex::create();
	}
//////////////////////////////////////////////////////////////////////////
	// walk & run
	m_AnimMgr.Tick(int(fElapsedTime*1000));
	if (m_uActionState==WALK)
	{
		float fRate = (float)m_AnimMgr.uFrame/(float)m_AnimMgr.uTotalFrames;
		float fWalkLength=((float)m_AnimMgr.CurLoop+fRate)*m_fOneWalkLength;

		if (m_fWalkLength+DirLength[m_uDir]<=fWalkLength)
		{
			m_fWalkLength+=DirLength[m_uDir];

			m_posCell.x+=DX[m_uDir];
			m_posCell.y+=DY[m_uDir];

			if (m_setPath.size()==0)
			{
				m_uActionState=STAND;
				CModelComplex::SetAnim("1");
			}
			else
			{
				setDir(m_setPath[0]);
				m_setPath.erase(m_setPath.begin());
			}
		}

		m_vPos.x = m_posCell.x+0.5f;
		m_vPos.z = m_posCell.y+0.5f;

		m_vPos += DirNormalize[m_uDir]*(fWalkLength-m_fWalkLength);
		m_vPos.y=CMainRoot::getInstance().getMainDialog().getDisplay().getScene().getTerrain()->GetHeight(Vec2D(m_vPos.x,m_vPos.z));

	}
	else if (m_uActionState==STAND)
	{
		if (m_setPath.size()>0)
		{
			m_uActionState=WALK;
			m_fWalkLength=0;
			setDir(m_setPath[0]);
			m_setPath.erase(m_setPath.begin());
			CModelComplex::SetAnim("15");
		}
	}
//////////////////////////////////////////////////////////////////////////
	//CModelComplex::OnFrameMove(fElapsedTime);
	//CModelObject::OnFrameMove(fElapsedTime);
	Animate(m_strAnimName);
	if (m_pModelData)
	{
		size_t uBoneCount = m_pModelData->m_Skeleton.m_Bones.size();
		for (size_t i=0; i<uBoneCount; ++i)
		{
			if (m_pModelData->m_Skeleton.m_Bones[i].strName=="Bip01 Head")
			{
				if (rand()%53==0)
				{
					m_fHeadRotate = (rand()%100)*0.02f-1.0f;
				}
				m_fCurrentHeadRotate+=(m_fHeadRotate-m_fCurrentHeadRotate)*fElapsedTime;
				Matrix mRotate;
				mRotate.rotate(Vec3D(0.0f,m_fCurrentHeadRotate,0.0f));
				m_setBonesMatrix[i] *= mRotate;
			}
		}
	}
	for (std::map<std::string,CModelObject*>::const_iterator it=m_mapSkinModel.begin();it!=m_mapSkinModel.end();it++)
	{
		const CModelData* pModelData = it->second->getModelData();
		if (pModelData)
		{
			if (pModelData->m_Mesh.m_bSkinMesh)
			{
				pModelData->m_Mesh.skinningMesh(it->second->m_pVB, m_setBonesMatrix);
			}
		}
	}
	for (std::map<std::string,CModelObject*>::const_iterator it=m_mapChildModel.begin();it!=m_mapChildModel.end();it++)
	{
		it->second->OnFrameMove(fElapsedTime);
	}
	CModelComplex::updateEmitters(getWorldMatrix(),fElapsedTime);
}

void CRole::setClass(int nID)
{
	m_nClass=nID;
}

void CRole::setSkeleton()
{
	load("Data\\Player\\player.bmd");
	BBox box;
	box.vMin=Vec3D(-0.5f,0.0f,-0.5f);
	box.vMax=Vec3D(0.5f,2.0f,0.5f);
	CModelObject::setBBox(box);
}

void CRole::setEquip(const char* szPart, int nEquipID)
{
	char szFilename[256];
	if (nEquipID<0x0F)
	{
		sprintf_s(szFilename,"Data\\Player\\%sMale%02d.bmd",szPart,nEquipID+1);
	}
	else
	{
		sprintf_s(szFilename,"Data\\Player\\%sClass%02d.bmd",szPart,(m_nClass>>5)+1);
	}
	loadSkinModel(szPart, szFilename);
}

void CRole::setHelm(int nID)
{
	setEquip("helm",nID);
}

void CRole::setArmor(int nID)
{
	setEquip("armor",nID);
}

void CRole::setGlove(int nID)
{
	setEquip("glove", nID);
}

void CRole::setPant(int nID)
{
	setEquip("pant", nID);
}

void CRole::setBoot(int nID)
{
	setEquip("boot", nID);
}

void CRole::setDir(unsigned char uDir)
{
	m_uDir = uDir;
	setRotate(Vec3D(0,uDir*PI*0.25f,0));
}

void CRole::setName(const wchar_t* wcsName)
{
	m_wstrName = wcsName;
}

const wchar_t* CRole::getName()
{
	return m_wstrName.c_str();
}

void CRole::setLevel(int nLevel)
{
	m_nLevel=nLevel;
}

#include "RPGSkyUIGraph.h"
#include "RenderSystem.h"
void CRole::drawName()const
{
	CRenderSystem& R = GetRenderSystem();
	R.SetLightingEnabled(false);
	R.SetTextureColorOP(0,TBOP_MODULATE,TBS_TEXTURE,TBS_DIFFUSE);
	R.SetDepthBufferFunc(false,false);
	CRect<int> rcViewport;
	R.getViewport(rcViewport);
	Matrix mView;
	R.getViewMatrix(mView);
	mView.Invert();
	Matrix mProjection;
	R.getProjectionMatrix(mProjection);
	//////////////////////////////////////////////////////////////////////////
	{
		float fZ = 1;
		Matrix mNewWorld=mView*Matrix::newTranslation(Vec3D(-fZ/mProjection._11,fZ/mProjection._22,fZ))*
			Matrix::newScale(Vec3D(fZ/mProjection._11/rcViewport.right*2.0f,-fZ/mProjection._22/rcViewport.bottom*2.0f,-fZ/mProjection._33/rcViewport.bottom*2.0f));
		R.setWorldMatrix(mNewWorld);
	}

	if (!m_wstrName.empty())
	{
		std::wstring wstrText = L"[color=255,255,0]Level"+i2ws(m_nLevel)+
			L"[/color] [color=128,255,255]"+m_wstrName+L"[/color]";

		BBox box = C3DMapObj::getBBox();
		Vec3D vPos = (box.vMin+box.vMax)*0.5f;
		vPos.y = box.vMax.y;
		Pos2D pos(0,0);
		GetRenderSystem().world2Screen(vPos,pos);
		RECT rc={pos.x-100,pos.y-30,pos.x+100,pos.y};
		RPGSkyUIGraph::getInstance().drawText(m_wstrName,-1,rc,ALIGN_TYPE_CENTER);
	}
}

void CRole::render(int flag)const
{
	//C3DMapObj::render(flag);
	Matrix mWorld = C3DMapObj::getWorldMatrix();
	if (m_uActionState==WALK)
	{
		// Move the model pos, bcoz the walk animate move in the model space.
		float fRate = (float)m_AnimMgr.uFrame/(float)m_AnimMgr.uTotalFrames;
		mWorld=Matrix::newTranslation(-DirNormalize[m_uDir]*fRate*m_fOneWalkLength)*mWorld;
	}
	GetRenderSystem().setWorldMatrix(mWorld);
	CModelComplex::render((E_MATERIAL_RENDER_TYPE)flag,(E_MATERIAL_RENDER_TYPE)flag);
	drawName();
}

void CRole::walk(unsigned char uDir)
{
	m_vPos.x+=DX[uDir];
	m_vPos.z+=DY[uDir];
	float fRate = (float)m_AnimMgr.uFrame/(float)m_AnimMgr.uTotalFrames;
	float fX=m_vPos.x+fRate;
	float fZ=m_vPos.z+fRate;
	m_vPos.y=CMainRoot::getInstance().getMainDialog().getDisplay().getScene().getTerrain()->GetHeight(Vec2D(fX,fZ));
	setDir(uDir);
}

void CRole::setCellPos(int x, int y)
{
	m_posCell.x = x;
	m_posCell.y = y;
	m_vPos.x = x+0.5f;
	m_vPos.z = y+0.5f;
	m_vPos.y=CMainRoot::getInstance().getMainDialog().getDisplay().getScene().getTerrain()->GetHeight(Vec2D(m_vPos.x,m_vPos.z));
}
