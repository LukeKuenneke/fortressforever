//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_weapon_deploydispenser.cpp
//	@author Gavin "Mirvin_Monkey" Bramhill
//	@date May 10, 2005
//	@brief A test dispenser construction slot
//
//	REVISIONS
//	---------
//	May 10, 2005, Mirv: First created
//
//	05/10/05, Mulchman:
//		Took over this class, thanks Mirv. Dispenser now sees if it can be built
//		during weapon idle times. If it can it draws a box. Pressing the attack
//		button will cause the dispenser to be built(if it can be).
//
//	05/11/05, Mulchman:
//		Fixed a bug where the faux dispenser would still be valid even though
//		we had built the thing already
//
//	05/14/05, Mulchman:
//		Optimized as per Mirv's forum suggestion

#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"


#ifdef CLIENT_DLL 
	#define CFFWeaponDeployDispenser C_FFWeaponDeployDispenser
	#include "c_ff_player.h"
	#include "c_ff_buildableobjects.h"
	#include "ff_buildableobjects_shared.h"
#else
	#include "ff_player.h"
	#include "ff_dispenser.h"
#endif

//=============================================================================
// CFFWeaponDeployDispenser
//=============================================================================

class CFFWeaponDeployDispenser : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponDeployDispenser, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponDeployDispenser();
#ifdef CLIENT_DLL 
	~CFFWeaponDeployDispenser() { Cleanup(); }
#endif

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual void WeaponIdle();
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);

	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_DEPLOYDISPENSER; }

private:

	CFFWeaponDeployDispenser(const CFFWeaponDeployDispenser &);

protected:
#ifdef CLIENT_DLL
	C_FFDispenser *pDispenser;
#endif

	// I've stuck the client dll check in here so it doesnt have to be everywhere else
	void Cleanup() 
	{
#ifdef CLIENT_DLL
		if (pDispenser) 
		{
			pDispenser->Remove();
			pDispenser = NULL;
		}
#endif
	}
};

//=============================================================================
// CFFWeaponDeployDispenser tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponDeployDispenser, DT_FFWeaponDeployDispenser) 

BEGIN_NETWORK_TABLE(CFFWeaponDeployDispenser, DT_FFWeaponDeployDispenser) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponDeployDispenser) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_deploydispenser, CFFWeaponDeployDispenser);
PRECACHE_WEAPON_REGISTER(ff_weapon_deploydispenser);

//=============================================================================
// CFFWeaponDeployDispenser implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponDeployDispenser::CFFWeaponDeployDispenser() 
{
#ifdef CLIENT_DLL
	pDispenser = NULL;
#endif
}

//----------------------------------------------------------------------------
// Purpose: Handles whatever should be done when they fire(build, aim, etc) 
//----------------------------------------------------------------------------
void CFFWeaponDeployDispenser::PrimaryAttack() 
{
	if (m_flNextPrimaryAttack < gpGlobals->curtime) 
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

		Cleanup();

#ifdef GAME_DLL
		// Bug #0000265: We've lost the dispenser explosion animation.
		// For now, don't do anything to a built disp with primary attack.
		CFFPlayer *player = GetPlayerOwner();
		CFFDispenser *disp = dynamic_cast<CFFDispenser *> (player->m_hDispenser.Get());

		if (!disp || !disp->IsBuilt()) 
			GetPlayerOwner()->Command_BuildDispenser();
#endif
	}
}

//----------------------------------------------------------------------------
// Purpose: Handles whatever should be done when they scondary fire
//----------------------------------------------------------------------------
void CFFWeaponDeployDispenser::SecondaryAttack() 
{
	if (gpGlobals->curtime > m_flNextSecondaryAttack + 0.1f && gpGlobals->curtime < m_flNextSecondaryAttack + 0.3f)
	{
#ifdef CLIENT_DLL
		engine->ClientCmd("detdismantledispenser");
#endif
	}

	m_flNextSecondaryAttack = gpGlobals->curtime;
}

//----------------------------------------------------------------------------
// Purpose: Checks validity of ground at this point or whatever
//----------------------------------------------------------------------------
void CFFWeaponDeployDispenser::WeaponIdle() 
{
	if (m_flTimeWeaponIdle < gpGlobals->curtime) 
	{
		//SetWeaponIdleTime(gpGlobals->curtime + 0.1f);

#ifdef CLIENT_DLL 
		C_FFPlayer *pPlayer = GetPlayerOwner();

		if (pPlayer->GetAmmoCount(AMMO_CELLS) < 100) 
		{
			Cleanup();
		}
		// If we haven't built a dispenser...
		else if (!pPlayer->m_hDispenser.Get() && pPlayer->GetAmmoCount(AMMO_CELLS) >= 100) 
		{
			CFFBuildableInfo hBuildInfo(pPlayer, FF_BUILD_DISPENSER, FF_BUILD_DISP_BUILD_DIST, FF_BUILD_DISP_RAISE_VAL);

			if (hBuildInfo.BuildResult() == BUILD_ALLOWED) 
			{
				if (pDispenser) 
				{
					// Dispenser is currently hidden
					if (pDispenser->GetEffects() & EF_NODRAW) 
						pDispenser->RemoveEffects(EF_NODRAW);

					// These were calculated in TryBuild() 
					pDispenser->SetAbsOrigin(hBuildInfo.GetBuildGroundOrigin());
					pDispenser->SetAbsAngles(hBuildInfo.GetBuildGroundAngles());
				}
				else
					pDispenser = C_FFDispenser::CreateClientSideDispenser(hBuildInfo.GetBuildGroundOrigin(), hBuildInfo.GetBuildGroundAngles());
			}
			// Unable to build, so hide buildable
			else if (pDispenser && ! (pDispenser->GetEffects() & EF_NODRAW)) 
				pDispenser->SetEffects(EF_NODRAW);
		}
		// Destroy if we already have one
		else
			Cleanup();
#endif
	}
}

bool CFFWeaponDeployDispenser::Holster(CBaseCombatWeapon *pSwitchingTo) 
{
	Cleanup();

	return BaseClass::Holster();
}

#ifdef GAME_DLL
	//=============================================================================
	// Commands
	//=============================================================================
	CON_COMMAND(dismantledispenser, "Dismantle dispenser")
	{
		CFFPlayer *pPlayer = ToFFPlayer(UTIL_GetCommandClient());

		if (!pPlayer)
			return;

		CFFDispenser *pDispenser = dynamic_cast<CFFDispenser *>(pPlayer->m_hDispenser.Get());

		if (!pDispenser)
			return;

		// Close enough to dismantle
		if ((pPlayer->GetAbsOrigin() - pDispenser->GetAbsOrigin()).LengthSqr() < 6400.0f)
		{
			pPlayer->GiveAmmo(130.0f, AMMO_CELLS, true);
			pDispenser->RemoveQuietly();
		}
		else
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_TOOFARAWAY");
	}

	CON_COMMAND(detdispenser, "Detonates dispenser")
	{
		CFFPlayer *pPlayer = ToFFPlayer(UTIL_GetCommandClient());

		if (!pPlayer)
			return;

		CFFDispenser *pDispenser = dynamic_cast<CFFDispenser *>(pPlayer->m_hDispenser.Get());

		if (!pDispenser)
			return;

		pDispenser->Detonate();
	}

	CON_COMMAND(detdismantledispenser, "Dismantles or detonate dispenser depending on distance")
	{
		CFFPlayer *pPlayer = ToFFPlayer(UTIL_GetCommandClient());

		if (!pPlayer)
			return;

		// Bug #0000333: Buildable Behavior (non build slot) while building
		if( pPlayer->m_bBuilding )
			return;

		CFFDispenser *pDispenser = dynamic_cast<CFFDispenser *>(pPlayer->m_hDispenser.Get());

		if (!pDispenser)
			return;

		// Close enough to dismantle
		if ((pPlayer->GetAbsOrigin() - pDispenser->GetAbsOrigin()).LengthSqr() < 6400.0f)
		{
			pPlayer->GiveAmmo(130.0f, AMMO_CELLS, true);
			pDispenser->RemoveQuietly();
		}
		else
			pDispenser->Detonate();
	}
#endif