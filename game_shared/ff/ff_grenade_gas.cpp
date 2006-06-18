/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file ff_grenade_gas.cpp
/// @author Shawn Smith (L0ki)
/// @date Apr. 23, 2005
/// @brief gas grenade class
/// 
/// Implementation of the CFFGrenadeGas class. This is the secondary grenade type for spy.
/// 
/// Revisions
/// ---------
/// Apr. 23, 2005	L0ki: Initial Creation

#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"

#define GASGRENADE_MODEL "models/weapons/w_eq_fraggrenade_thrown.mdl"
#define GAS_SOUND "GasGrenade.Explode"
#define GAS_EFFECT "GasCloud"

#ifdef CLIENT_DLL
	#define CFFGrenadeGas C_FFGrenadeGas
	#include "c_te_effect_dispatch.h"
#else
	#include "te_effect_dispatch.h"
#endif

class CFFGrenadeGas : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeGas,CFFGrenadeBase)

	CNetworkVector(m_vInitialVelocity);

	virtual void Precache();
	virtual float GetShakeAmplitude( void ) { return 0.0f; }	// remove the shake
	virtual float GetGrenadeDamage() { return 0.0f; }
	virtual float GetGrenadeRadius() { return 300.0f; }
	virtual const char *GetBounceSound() { return "GasGrenade.Bounce"; }

#ifdef CLIENT_DLL
	CFFGrenadeGas() {}
	CFFGrenadeGas( const CFFGrenadeGas& ) {}
#else
	virtual void Spawn();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);
	virtual void GrenadeThink();

	float m_flNextHurt;
#endif
};

LINK_ENTITY_TO_CLASS( gasgrenade, CFFGrenadeGas );
PRECACHE_WEAPON_REGISTER( gasgrenade );

#ifdef GAME_DLL
	void CFFGrenadeGas::Spawn( void )
	{
		DevMsg("[Grenade Debug] CFFGrenadeGas::Spawn\n");
		SetModel( GASGRENADE_MODEL );
		BaseClass::Spawn();

		m_flNextHurt = 0;
	}

	void CFFGrenadeGas::Explode(trace_t *pTrace, int bitsDamageType)
	{
		DevMsg("[Grenade Debug] CFFGrenadeGas::Explode\n");
		CFFGrenadeBase::PreExplode( pTrace, GAS_SOUND, GAS_EFFECT );

		// TODO: trigger client side hallucination here

		// TODO: Don't for IsInNoGren() check

		//CFFGrenadeBase::PostExplode();
	}

	void CFFGrenadeGas::GrenadeThink()
	{
		// If the grenade is in a no grenade area, kill it
		if( IsInNoGren() && ( gpGlobals->curtime > m_flDetonateTime ) )
		{
			// This will remove the gren
			CFFGrenadeBase::PostExplode();
			return;
		}

		// Been detonated for 10 secs now, so fade out
		if (gpGlobals->curtime > m_flDetonateTime + 10.0f)
		{
			SetThink(&CBaseGrenade::SUB_FadeOut);
			//SetNextThink(gpGlobals->curtime + 10.0f);
		}

		// Don't start until we've stopped moving, for now
		//if (GetAbsVelocity().LengthSqr() > 0 && gpGlobals->curtime > m_flDetonateTime)
		//	m_flDetonateTime = gpGlobals->curtime + 1.0f;

		// Damage people in here
		if (gpGlobals->curtime > m_flDetonateTime && m_flNextHurt < gpGlobals->curtime)
		{
			m_flNextHurt = gpGlobals->curtime + 0.2f;

			BEGIN_ENTITY_SPHERE_QUERY(GetAbsOrigin(), GetGrenadeRadius())
				if (pPlayer && gpGlobals->curtime > pPlayer->m_flLastGassed + 1.0f)
				{
					pPlayer->EmitSound("Player.DrownContinue");	// |-- Mirv: [TODO] Change to something more suitable

					CTakeDamageInfo info(this, GetOwnerEntity(), 1, DMG_DIRECT);
					pPlayer->TakeDamage(info);

					CSingleUserRecipientFilter user((CBasePlayer *)pPlayer);
					user.MakeReliable();

					UserMessageBegin(user, "StatusIconUpdate");
					WRITE_BYTE(FF_ICON_GAS);
					WRITE_FLOAT(15.0);
					MessageEnd();

					// Don't allow this to be accumulative
					pPlayer->m_flLastGassed = gpGlobals->curtime;
				}
			END_ENTITY_SPHERE_QUERY();

			// Just shoving this here for now, Ted can sort out the effect properly.
			EmitSound(GAS_SOUND);

			CEffectData data;
			data.m_vOrigin = GetAbsOrigin();
			data.m_flScale = 1.0f;
			DispatchEffect(GAS_EFFECT, data);
		}

		//BaseClass::GrenadeThink();

		// Next think straight away
		SetNextThink(gpGlobals->curtime);

		CFFGrenadeBase::WaterThink();
	}
#endif

void CFFGrenadeGas::Precache()
{
	DevMsg("[Grenade Debug] CFFGrenadeGas::Precache\n");
	PrecacheModel( GASGRENADE_MODEL );
	PrecacheScriptSound( GAS_SOUND );
	BaseClass::Precache();
}