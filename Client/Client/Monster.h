//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once /* Monster.h */
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "Role.h"
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

class CMonster : public CRole
{
private:
	unsigned long	m_uType;
public:
	CMonster();
	~CMonster();
	// ----
	void			setType			(unsigned long uType);
	// ----
	virtual void	frameMoveRole	(const Matrix& mWorld, double fTime, float fElapsedTime);
	virtual void	setActionState	(unsigned char uActionState);
};
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------