#pragma once
#include "Vec4D.h"
#include "Vec2D.h"
#include "Color.h"
#include "Pos2D.h"
#include "Material.h"
#include "vector"

//////////////////////////////////////////////////////////////////////////
// SceneData
//////////////////////////////////////////////////////////////////////////

struct TerrainCell
{
	unsigned char	uTileID[2];
	Color32			color;
	float			fHeight;
	unsigned char	uAttribute;
	long			SearchedFlag;
};

//////////////////////////////////////////////////////////////////////////
// Scene
//////////////////////////////////////////////////////////////////////////
#include "RenderSystemCommon.h"
#include "Frustum.h"

class iSceneData
{
public:
	iSceneData(){};
	virtual ~iSceneData(){};
	//
	virtual void			clear()=0;
	virtual void			create(size_t width, size_t height)=0;
	virtual bool			resize(size_t width, size_t height)=0;
	//
	virtual int				getWidth()const=0;
	virtual int				getHeight()const=0;
	virtual int				getVertexCount()const=0;
	//
	virtual bool			isCellIn(int x, int y)const=0;
	virtual bool			isPointIn(int x, int y)const=0;
	//
	virtual unsigned char	getCellTileID(int x, int y, size_t layer = 0)const=0;
	virtual void			setCellTileID(int x, int y, unsigned char uTileID, size_t layer = 0)=0;
	//
	virtual unsigned long	getVertexIndex(int nCellX, int nCellY)const=0;
	virtual int				getCellXByVertexIndex(unsigned long uVertexIndex)const=0;
	virtual int				getCellYByVertexIndex(unsigned long uVertexIndex)const=0;
	//
	virtual float			getVertexHeight(int x, int y)const=0;
	virtual void			setVertexHeight(int x, int y, float fHeight)=0;
	//
	virtual Vec3D			getVertexNormal(int x, int y)const=0;
	//
	virtual unsigned char	getCellAttribute(int x, int y)const=0;
	virtual void			setCellAttribute(int x, int y, unsigned char uAtt)=0;
	//
	virtual Color32			getVertexColor(int x, int y)const=0;
	virtual void			setVertexColor(int x, int y, Color32 color)=0;
	//
	virtual float			getHeight(float fX, float fY)const=0;
	virtual Vec4D			getColor(float fX, float fY)const=0;
	//
	virtual bool			pickCell(int nCellX, int nCellY, const Vec3D& vRayPos, const Vec3D& vRayDir, Vec3D* pPos = NULL)const=0;
	virtual bool			pick(const Vec3D& vRayPos, const Vec3D& vRayDir, Vec3D* pPos = NULL)const=0;
	//
	virtual const			std::string& getFilename()const=0;
	//
	virtual bool			hasGrass(int nCellX, int nCellY)const=0;

	virtual std::vector<TerrainCell>& getCells()=0;

	virtual const char*		getLightMap()const=0;
	virtual void			setLightMap(const char* szFilename)=0;
public:
	virtual void					setFog					(const Fog& fog)=0;
	virtual void					setLight				(const DirectionalLight& light)=0;
	virtual const Fog&				getFog					()const=0;
	virtual const DirectionalLight&	getLight				()const=0;
};