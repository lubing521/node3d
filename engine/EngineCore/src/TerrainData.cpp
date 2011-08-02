#include "TerrainData.h"
#include <fstream>
#include "interpolation.h"
#include "IORead.h"
#include "Intersect.h"

#define		MAX_PICK_COUNT	200
CTerrainData::CTerrainData()
:m_nWidth(0)
,m_nHeight(0)
,m_nChunkSize(0)
{
}

CTerrainData::~CTerrainData()
{
}

void CTerrainData::clear()
{
	m_nWidth = 0;
	m_nHeight = 0;
	m_nChunkSize = 0;
	m_Cells.clear();
}

void CTerrainData::create(size_t width, size_t height, size_t chunkSize)
{
	clear();
	resize(width,height,chunkSize);
}
template <class T>
inline void resizeVector(std::vector<T>& v, size_t srcWidth, size_t srcHeight,size_t destWidth, size_t destHeight,T val=0)
{
	std::vector<T> srcV=v;
	v.resize(destWidth*destHeight,val);
	if (srcV.empty())
	{
		return;
	}
	size_t width = min(srcWidth,destWidth);
	size_t height = min(srcHeight,destHeight);
	for (size_t y = 0; y < height; ++y)
	{
		for (size_t x = 0; x < width; ++x)
		{
			v[y*destWidth+x] = srcV[y*srcWidth+x];
		}
	}
}

bool CTerrainData::resize(size_t width, size_t height, size_t chunkSize)
{
	if (chunkSize==0)
	{
		chunkSize = 8;
		for (size_t i=8; i<30; ++i)
		{
			if (width%i==0&&width%i==0)
			{
				chunkSize = i;
				break;
			}
		}
	}
	width=(width/chunkSize)*chunkSize;
	height=(height/chunkSize)*chunkSize;

	if (65536<(width+1)*(height+1))
	{
		return false;
	}

	TerrainCell val={0,255,0xFFFFFFFF,0xFFFFFFFF,0.0f,Vec3D(0.0f,1.0f,0.0f),0};
	resizeVector(m_Cells,m_nWidth+1,m_nHeight+1,width+1,height+1,val);

	m_nWidth = width;
	m_nHeight = height;
	m_nChunkSize = chunkSize;

	// ----
	// # Create Octree Root
	// ----
	m_OctreeRoot.clearNodes();
	BBox box;
	float fLength = (float)max(m_nWidth,m_nHeight);
	box.vMin = Vec3D(0.0f,		-fLength*0.5f,	0.0f);
	box.vMax = Vec3D(fLength,	fLength*0.5f,	fLength);
	m_OctreeRoot.create(box, 4);
	// ----
	// # Create Chunks
	// ----
	m_Chunks.clear();
	for (size_t y=0; y<height/chunkSize; ++y)
	{
		for (size_t x=0; x<width/chunkSize; ++x)
		{
			TerrainChunk chunk;
			chunk.box.vMin.x = x*chunkSize;
			chunk.box.vMin.y = 0;
			chunk.box.vMin.z = y*chunkSize;
			chunk.box.vMax.x = x*chunkSize+chunkSize;
			chunk.box.vMax.y = 0;
			chunk.box.vMax.z = y*chunkSize+chunkSize;
			m_Chunks.push_back(chunk);
		}
	}
	for (int i=0; i<m_Chunks.size(); ++i)
	{
		updateChunk(&m_Chunks[i]);
	}
	return true;
}

void CTerrainData::updateChunk(TerrainChunk* pChunk)
{
	m_OctreeRoot.eraseNode(pChunk);
	// ----
	int nBeginX			= pChunk->box.vMin.x;
	int nBeginY			= pChunk->box.vMin.z;
	int nEndX			= pChunk->box.vMax.x;
	int nEndY			= pChunk->box.vMax.z;
	float& fMinHeight	= pChunk->box.vMin.y;
	float& fMaxHeight	= pChunk->box.vMax.y;
	TerrainCell* pCell	= getCell(nBeginX,nBeginY);
	fMinHeight			= pCell->fHeight;
	fMaxHeight			= pCell->fHeight;
	for (size_t y = nBeginY; y<=nEndY; ++y)
	{
		for (size_t x = nBeginX; x<=nEndX; ++x)
		{
			fMinHeight = min(fMinHeight,pCell->fHeight);
			fMaxHeight = max(fMaxHeight,pCell->fHeight);
			pCell++;
		}
		pCell += m_nWidth-nEndX+nBeginX;
	}
	// ----
	m_OctreeRoot.addNode(pChunk->box, pChunk);
}

void CTerrainData::walkOctree(const CFrustum& frustum, std::set<TerrainChunk*>& setNode)
{
	m_OctreeRoot.walkOctree(frustum, setNode);
}

TerrainCell* CTerrainData::getCell(int x, int y)
{
	int nCellIndex = y*(m_nWidth+1)+x;
	return &m_Cells[nCellIndex];
}

const TerrainCell* CTerrainData::getCell(int x, int y)const
{
	int nCellIndex = y*(m_nWidth+1)+x;
	return &m_Cells[nCellIndex];
}

void CTerrainData::getVertexByCell(int nCellX, int nCellY, TerrainVertex& vertex)const
{
	const TerrainCell* cell = getCell(nCellX,nCellY);
	vertex.p = Vec3D((float)nCellX, cell->fHeight, (float)nCellY);
	vertex.n = cell->vNormals;

	/*Vec3D vBinormal = vertex.n.cross(Vec3D(0,0,1)).normalize();
	Vec3D vTangent =  Vec3D(1,0,0).cross(vertex.n).normalize();
	Matrix mTangent;
	mTangent.Zero();
	mTangent._11=vBinormal.x;
	mTangent._21=vBinormal.y;
	mTangent._31=vBinormal.z;

	mTangent._12=vertex.n.x;
	mTangent._22=vertex.n.y;
	mTangent._32=vertex.n.z;

	mTangent._13=vTangent.x;
	mTangent._23=vTangent.y;
	mTangent._33=vTangent.z;

	vertex.n = mTangent*GetLightDir();
	vertex.n=vertex.n.normalize();*/

	vertex.c = cell->color;
	//vertex.t = m_EquableTexUV[nCellIndex];
	vertex.t0.x = (float)nCellX;
	vertex.t0.y = (float)nCellY;
	vertex.t1.x = (float)nCellX/(float)getWidth();
	vertex.t1.y = (float)nCellY/(float)getHeight();
}

void CTerrainData::getGrassVertexByCell(int nCellX, int nCellY, TerrainVertex*& vertex)const
{
	const TerrainCell* cell1 = getCell(nCellX,nCellY);
	const TerrainCell* cell2 = getCell(nCellX+1,nCellY+1);
	int	nRand = (((nCellY*(m_nWidth+1)+nCellX+nCellX*nCellY)*214013L+2531011L)>>16)&0x7fff;   
	float fTexU = (nRand%4)*0.25f;
	vertex[0].p = Vec3D((float)nCellX, cell1->fHeight, (float)nCellY);
	//vertex[0].n = Vec3DGetCellNormal(posCell1);
	vertex[0].c = cell1->color;
	vertex[0].t0 = Vec2D(fTexU,1.0f);

	vertex[1].p = vertex[0].p+Vec3D(0.0f,1.5f,0.0f);
	//vertex[1].n = vertex[0].n;
	vertex[1].c = vertex[0].c;
	vertex[1].t0 = Vec2D(fTexU,0.0f);

	vertex[3].p = Vec3D((float)(nCellX+1), cell2->fHeight, (float)(nCellY+1));
	//vertex[3].n = GetCellNormal(posCell2);
	vertex[3].c = cell2->color;
	vertex[3].t0 = Vec2D(fTexU+0.25f,1.0f);

	vertex[2].p = vertex[3].p+Vec3D(0.0f,1.5f,0.0f);
	//vertex[2].n = vertex[3].n;
	vertex[2].c = vertex[3].c;
	vertex[2].t0 = Vec2D(fTexU+0.25f,0.0f);
}

bool CTerrainData::hasGrass(int nCellX, int nCellY)const
{
	const TerrainCell* cell = getCell(nCellX,nCellY);
	const unsigned char uAttribute = getCellAttribute(nCellX,nCellY);
	return 0==cell->uTileID[0]&&255==cell->uTileID[1]&&0==(cell->uAttribute&ATTRIBUTE_GRASS);
}

bool CTerrainData::pickCell(int nCellX, int nCellY, const Vec3D& vRayPos, const Vec3D& vRayDir, Vec3D* pPos)const
{
	if (isCellIn(nCellX,nCellY))
	{
		const TerrainCell* cell = getCell(nCellX,nCellY);
		Vec3D v0((float)(nCellX),	cell->fHeight,					(float)(nCellY));
		Vec3D v1((float)(nCellX),	(cell+m_nWidth+1)->fHeight,		(float)(nCellY+1));
		Vec3D v2((float)(nCellX+1),	(cell+m_nWidth+1+1)->fHeight,	(float)(nCellY+1));
		Vec3D v3((float)(nCellX+1),	(cell+1)->fHeight,				(float)(nCellY));
		Vec3D vOut;
		if(IntersectTri(v1,v2,v0,vRayPos,vRayDir,vOut))
		{
			if (pPos)
			{
				*pPos = vOut;
			}
			return true;
		}
		else if (IntersectTri(v3,v0,v2,vRayPos,vRayDir,vOut))
		{
			if (pPos)
			{
				*pPos = vOut;
			}
			return true;
		}
	}
	return false;
}

bool CTerrainData::pick(const Vec3D& vRayPos, const Vec3D& vRayDir, Vec3D* pPos)const
{
	// 格子地图的快速PICK
	int nIncX = vRayDir.x>0?1:-1;
	int nIncY = vRayDir.z>0?1:-1;
	float fK = vRayDir.z/vRayDir.x;
	float fB = vRayPos.z - fK*vRayPos.x;
	int nX = int(vRayPos.x)-nIncX;
	int nY = int(vRayPos.z)-nIncY;

	if (fK<1&&fK>-1)
	{
		for (int i = 0; i < MAX_PICK_COUNT; i++)
		{
			int nCellX = nX + nIncX;
			int nCellY = (int)(fK*(float)nCellX+fB);
			if (nCellY!=nY)
			{
				nY = nCellY;
				nCellX = nX - (nIncX<0);
				nCellY = nY - nIncY*(nIncX<0);
			}
			else
			{
				nX = nCellX;
			}
			if (pickCell(nCellX, nCellY, vRayPos, vRayDir, pPos))
			{
				return true;
			}
		}
	}
	else
	{
		for (int i = 0; i < MAX_PICK_COUNT; i++)
		{
			int nCellY = nY + nIncY;
			int nCellX = (int)(((float)nCellY-fB)/fK);
			if (nCellX!=nX)
			{
				nX = nCellX;
				nCellX = nX - nIncX*(nIncY<0);
				nCellY = nY - (nIncY<0);
			}
			else
			{
				nY = nCellY;
			}
			if (pickCell(nCellX, nCellY, vRayPos, vRayDir, pPos))
			{
				return true;
			}
		}
	}
	return false;
}

unsigned long CTerrainData::getVertexIndex(int nCellX, int nCellY)const
{
	return nCellY*(m_nWidth+1) + nCellX;
}

int CTerrainData::getCellXByVertexIndex(unsigned long uVertexIndex)const
{
	return uVertexIndex%(m_nWidth+1);
}

int CTerrainData::getCellYByVertexIndex(unsigned long uVertexIndex)const
{ 
	return uVertexIndex/(m_nWidth+1);
}

bool CTerrainData::isCellIn(int x, int y)const
{
	return x>=0 && x<m_nWidth && y>=0 && y<m_nHeight;
}

bool CTerrainData::isPointIn(int x, int y)const
{
	return x>=0 && x<=m_nWidth && y>=0 && y<=m_nHeight;
}

unsigned char CTerrainData::getCellTileID(int x, int y, size_t layer)const
{
	if (isCellIn(x,y))
	{
		if (2>layer)
		{
			return getCell(x,y)->uTileID[layer];
		}
	}
	return 0;
}

void CTerrainData::setCellTileID(int x, int y, unsigned char uTileID, size_t layer)
{
	if (isCellIn(x,y))
	{
		if (2>layer)
		{
			getCell(x,y)->uTileID[layer] = uTileID;
		}
	}
}

unsigned char CTerrainData::getCellAttribute(int x, int y)const
{
	if (isCellIn(x,y))
	{
		return getCell(x,y)->uAttribute;
	}
	return 0;
}

void CTerrainData::setCellAttribute(int x, int y, unsigned char uAtt)
{
	if (isCellIn(x,y))
	{
		getCell(x,y)->uAttribute = uAtt;
	}
}

float CTerrainData::getVertexHeight(int x, int y)const
{
	if (isPointIn(x,y))
	{
		return getCell(x,y)->fHeight;
	}
	return 0.0f;
}

void CTerrainData::setVertexHeight(int x, int y, float fHeight)
{
	if (isPointIn(x,y))
	{
		getCell(x,y)->fHeight = fHeight;
	}
}

Vec3D CTerrainData::getVertexNormal(int x, int y)const
{
	float a = getVertexHeight(x,	y);
	float b = getVertexHeight(x,	y+1);
	float c = getVertexHeight(x+1,	y);
	Vec3D vVector0(0,(b-a),1);
	Vec3D vVector1(1,(c-a),0);
	Vec3D vN = vVector0.cross(vVector1);
	return vN.normalize();
}

Color32 CTerrainData::getVertexColor(int x, int y)const
{
	if (isPointIn(x,y))
	{
		return getCell(x,y)->color;
	}
	return 0xFFFFFFFF;
}

void CTerrainData::setVertexColor(int x, int y, Color32 color)
{
	if (isPointIn(x,y))
	{
		getCell(x,y)->color = color;
	}
}

float CTerrainData::getHeight(float fX, float fY)const
{
	int nCellX = fX;
	int nCellY = fY;
	float u = fX - nCellX;
	float v = fY - nCellY;
	float a = getVertexHeight(nCellX,	nCellY);
	float b = getVertexHeight(nCellX+1,	nCellY);
	float c = getVertexHeight(nCellX,	nCellY+1);
	float d = getVertexHeight(nCellX+1,	nCellY+1);
	return bilinearInterpolation(a,b,c,d,u,v);
}

Vec4D CTerrainData::getColor(float fX, float fY)const
{
	int nCellX = fX;
	int nCellY = fY;
	float u = fX - nCellX;
	float v = fY - nCellY;
	Vec4D a = getVertexColor(nCellX,	nCellY);
	Vec4D b = getVertexColor(nCellX+1,	nCellY);
	Vec4D c = getVertexColor(nCellX,	nCellY+1);
	Vec4D d = getVertexColor(nCellX+1,	nCellY+1);
	return bilinearInterpolation(a,b,c,d,u,v);
}

#define DIR_COUNT 8
#define DIR_INVALID 0xFF
const int DX[DIR_COUNT] = { 0, 1, 1, 1, 0,-1,-1,-1};
const int DY[DIR_COUNT] = {-1,-1, 0, 1, 1, 1, 0,-1};
//const int DX[16] = { 0, 1, 1, 2, 1, 2, 1, 1, 0,-1,-1,-2,-1,-2,-1,-1};
//const int DY[16] = {-1,-2,-1,-1, 0, 1, 1, 2, 1, 2, 1, 1, 0,-1,-1,-2};

unsigned char CTerrainData::getPath(int sx,int sy,int tx,int ty, std::vector<unsigned char>& path)
{
	m_Searched.clear();
	m_Searched.resize(getVertexCount(),false);
	m_nNodeCount=0;
	m_nAllNodeCount=0;

	int level = 0;

	AddNode(sx,sy,tx,ty,DIR_INVALID,0,0);

	for(;m_nNodeCount>0;)
	{
		Node a = GetNode(); // 取出一个节点
		if ((a.x == tx) && (a.y == ty)) // 目标
		{
			m_nAllNodeCount=a.n;
			break;
		}
		for (int i = 0; i<DIR_COUNT; i++)
		{
			level=a.level+((i%2)?7:5);
			AddNode(a.x+DX[i], a.y+DY[i], tx, ty, i, level, a.n); // 扩展此节点
			if (m_nAllNodeCount>=MAX_ALLNODE)
			{
				return 0;
			}
		}
	}
	//m_nAllNodeCount--;
	if (m_nAllNodeCount<=0)
	{
		return 0;
	}
	path.clear();
	int nNodeIndex = m_nAllNodeCount;
	while(m_allnode[nNodeIndex].dir != DIR_INVALID)
	{
		path.insert(path.begin(),m_allnode[nNodeIndex].dir);
		nNodeIndex = m_allnode[nNodeIndex].father;
	}
	return m_allnode[m_nAllNodeCount].dir;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void CTerrainData::AddNode(int x,int y,int tx,int ty,unsigned char dir,int level,int father)
{
	if (!isCellIn(x,y))
	{
		return;
	}
	//if (((x!=tx) || (y!=ty))/* && (dir!=DIR_INVALID)*/)
	{
		if (getCell(x,y)->uAttribute&TERRAIN_ATT_TYPE_BALK)
		{
			return;
		}
		if (m_Searched[getVertexIndex(x,y)])// 可检查节点是否访问过
		{
			return;
		}
	}

	m_nNodeCount++;
	int p = m_nNodeCount;
	int f = level + (7*abs(x-tx)+7*abs(y-ty)+3*abs(abs(x-tx)-abs(y-ty))) / 2;//启发函数定义
	while( p > 1 )
	{
		int q = p >> 1;
		if( f < node[q].f )
			node[p] = node[q];
		else
			break;
		p = q;
	}
	node[p].x = x;
	node[p].y = y;
	node[p].f = f;
	node[p].level = level;
	node[p].n = m_nAllNodeCount;

	assert(m_nAllNodeCount<MAX_ALLNODE);

	m_allnode[m_nAllNodeCount].dir = dir;
	m_allnode[m_nAllNodeCount].father = father;
	m_nAllNodeCount++;

	m_Searched[getVertexIndex(x,y)]=true;
}

CTerrainData::Node CTerrainData::GetNode()
{
	Node Top = node[1];
	int p,q;
	Node a = node[m_nNodeCount];
	m_nNodeCount--;
	p = 1;
	q = p * 2;
	while( q <= m_nNodeCount )
	{
		if( (node[q+1].f < node[q].f) && (q < m_nNodeCount) )
			q++;
		if( node[q].f < a.f )
			node[p] = node[q];
		else
			break;
		p = q;
		q = p * 2;
	}
	node[p] = a;	
	return Top;
}