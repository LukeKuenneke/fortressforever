/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_assaultcannon.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF Assault Cannon code.
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation logged


#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"
#include "in_buttons.h"
#include "soundenvelope.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponAssaultCannon C_FFWeaponAssaultCannon
	#include "c_ff_player.h"
#else
	#include "ff_player.h"
#endif

// Please keep all values exposed to cvars so non programmers can be tweaking, even if the code isn't final.
ConVar ffdev_ac_maxchargetime("ffdev_ac_maxchargetime", "2.0", FCVAR_REPLICATED, "Assault Cannon Max Charge Time");
ConVar ffdev_ac_chargeuptime("ffdev_ac_chargeuptime", "0.0", FCVAR_REPLICATED, "Assault Cannon Chargeup Time");
ConVar ffdev_ac_overheatdelay("ffdev_ac_overheatdelay", "1.0", FCVAR_REPLICATED, "Assault Cannon Overheat delay");

// You pretty much only hear this high rev sound when you overheat, so it should probably be loud
ConVar ffdev_ac_revsound_volume_high("ffdev_ac_revsound_volume_high", "1.4", FCVAR_REPLICATED, "Assault Cannon Rev Sound High Volume");
ConVar ffdev_ac_revsound_volume_low("ffdev_ac_revsound_volume_low", "0.5", FCVAR_REPLICATED, "Assault Cannon Rev Sound Low Volume");
ConVar ffdev_ac_revsound_pitch_high("ffdev_ac_revsound_pitch_high", "150", FCVAR_REPLICATED, "Assault Cannon Rev Sound High Pitch");
ConVar ffdev_ac_revsound_pitch_low("ffdev_ac_revsound_pitch_low", "50", FCVAR_REPLICATED, "Assault Cannon Rev Sound Low Pitch");

ConVar ffdev_ac_minspread("ffdev_ac_minspread", "0.01", FCVAR_REPLICATED, "Assault Cannon Minimum spread");
ConVar ffdev_ac_maxspread("ffdev_ac_maxspread", "0.10", FCVAR_REPLICATED, "Assault Cannon Maximum spread");

ConVar ffdev_ac_maxcycletime("ffdev_ac_maxcycletime", "0.12", FCVAR_REPLICATED, "Assault Cannon Maximum cycle time");
ConVar ffdev_ac_mincycletime("ffdev_ac_mincycletime", "0.06", FCVAR_REPLICATED, "Assault Cannon Minimum cycle time");

ConVar ffdev_ac_loopshotsound_rate_max("ffdev_ac_loopshotsound_rate_max", "0.12", FCVAR_REPLICATED, "Loop shot sound starts fading in at this rate of fire.");
ConVar ffdev_ac_loopshotsound_rate_min("ffdev_ac_loopshotsound_rate_min", "0.06", FCVAR_REPLICATED, "Loop shot sound is at max volume at this rate of fire.");
ConVar ffdev_ac_loopshotsound_pitch_high("ffdev_ac_loopshotsound_pitch_high", "150", FCVAR_REPLICATED, "How high the pitch of the loop shot sound can get (coincides with ffdev_ac_maxcycletime).");
ConVar ffdev_ac_loopshotsound_pitch_low("ffdev_ac_loopshotsound_pitch_low", "50", FCVAR_REPLICATED, "How low the pitch of the loop shot sound can get (coincides with ffdev_ac_mincycletime).");
ConVar ffdev_ac_loopshotsound_volume_high("ffdev_ac_loopshotsound_volume_high", "1.4", FCVAR_REPLICATED, "How high the volume of the loop shot sound can get (coincides with ffdev_ac_loopshotsound_rate_max).");
ConVar ffdev_ac_loopshotsound_volume_low("ffdev_ac_loopshotsound_volume_low", "0.5", FCVAR_REPLICATED, "How low the volume of the loop shot sound can get (coincides with ffdev_ac_loopshotsound_rate_min).");

ConVar ffdev_ac_speedeffect_max("ffdev_ac_speedeffect_max", "0.5", FCVAR_REPLICATED, "Speed effect at ffdev_ac_maxcycletime (slower shooting = faster walking).");
ConVar ffdev_ac_speedeffect_min("ffdev_ac_speedeffect_min", "0.2", FCVAR_REPLICATED, "Speed effect at ffdev_ac_mincycletime (faster shooting = slower walking).");

#ifdef CLIENT_DLL

ConVar ffdev_ac_barrelrotation_speed_min("ffdev_ac_barrelrotation_speed_min", "3.0", 0, "Minimum speed the ac barrel will rotate.");
ConVar ffdev_ac_barrelrotation_speed_max("ffdev_ac_barrelrotation_speed_max", "9.0", 0, "Maximum speed the ac barrel will rotate.");

#endif

//=============================================================================
// CFFWeaponAssaultCannon
//=============================================================================

class CFFWeaponAssaultCannon : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponAssaultCannon, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponAssaultCannon();
	~CFFWeaponAssaultCannon();

	virtual void PrimaryAttack();
	//virtual void WeaponIdle();
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);
	virtual bool Deploy();

	virtual void ItemPostFrame();
	
	void UpdateChargeTime();

	virtual void Precache();

	virtual void Fire();

	virtual void GetHeatLevel(int _firemode, float &_current, float &_max) 
	{
		_current = m_flChargeTime; 
		_max = ffdev_ac_maxchargetime.GetFloat(); 
	}
private:
	virtual float GetFireRate();
	Vector GetFireSpread();

#ifdef CLIENT_DLL
	void UpdateBarrelSpin();
#endif

	void PlayLoopShotSound();
	void StopLoopShotSound();

	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_ASSAULTCANNON; }
	//const char *GetTracerType() { return "ACTracer"; }

private:

	CFFWeaponAssaultCannon(const CFFWeaponAssaultCannon &);

public:	// temp while i expose m_flChargeTime to global function

	float m_flLastTick;
	float m_flDeployTick;
	float m_flChargeTime;
	CNetworkVar(float, m_flTriggerPressed);
	CNetworkVar(float, m_flTriggerReleased);

	bool	m_bFiring;
	bool	m_bPlayLoopShotSound;

#ifdef CLIENT_DLL
	CSoundPatch *m_pRevSound;

	float		m_flRotationValue;
	float		m_flChargeTimeClient;
	int			m_iBarrelRotation;
#endif
};

//=============================================================================
// CFFWeaponAssaultCannon tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponAssaultCannon, DT_FFWeaponAssaultCannon) 

BEGIN_NETWORK_TABLE(CFFWeaponAssaultCannon, DT_FFWeaponAssaultCannon) 
#ifdef GAME_DLL
	SendPropFloat(SENDINFO(m_flTriggerPressed)),
	SendPropFloat(SENDINFO(m_flTriggerReleased)),
#else
	RecvPropFloat(RECVINFO(m_flTriggerPressed)),
	RecvPropFloat(RECVINFO(m_flTriggerReleased)),
#endif
END_NETWORK_TABLE() 

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CFFWeaponAssaultCannon) 
	DEFINE_PRED_FIELD_TOL(m_flTriggerPressed, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, 1.0f),
	DEFINE_PRED_FIELD_TOL(m_flTriggerReleased, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, 1.0f),
END_PREDICTION_DATA() 
#endif

LINK_ENTITY_TO_CLASS(ff_weapon_assaultcannon, CFFWeaponAssaultCannon);
PRECACHE_WEAPON_REGISTER(ff_weapon_assaultcannon);

//=============================================================================
// CFFWeaponAssaultCannon implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponAssaultCannon::CFFWeaponAssaultCannon() 
{
	m_flNextSecondaryAttack = gpGlobals->curtime;

	m_flTriggerReleased = 1.0f;
	m_flTriggerPressed = 0.0f;

	m_bFiring = false;
	m_bPlayLoopShotSound = false;

#ifdef CLIENT_DLL
	m_pRevSound = NULL;

	m_flRotationValue = 0.0f;
	m_flChargeTimeClient = 0.0f;
	m_iBarrelRotation = -1;
#endif
}

//----------------------------------------------------------------------------
// Purpose: Deconstructor
//----------------------------------------------------------------------------
CFFWeaponAssaultCannon::~CFFWeaponAssaultCannon() 
{
	m_flNextSecondaryAttack = gpGlobals->curtime;

#ifdef CLIENT_DLL

	if(m_pRevSound)
	{
		CSoundEnvelopeController::GetController().SoundDestroy(m_pRevSound);
		m_pRevSound = NULL;
	}

	m_flRotationValue = 0.0f;
	m_flChargeTimeClient = 0.0f;
	m_iBarrelRotation = -1;
#endif
}

//----------------------------------------------------------------------------
// Purpose: When holstered we need to stop any sounds + remove speed effects
//----------------------------------------------------------------------------
bool CFFWeaponAssaultCannon::Holster(CBaseCombatWeapon *pSwitchingTo) 
{
	// Bug #0000499: Oddity with assault cannon
	// Moved this up to here so it gets called and remove it if its set
#ifdef GAME_DLL
	CFFPlayer *pPlayer = GetPlayerOwner();

	// Add a temp slowdown as we unwind
	//if(pPlayer->IsSpeedEffectSet(SE_ASSAULTCANNON))
	//	pPlayer->AddSpeedEffect( SE_ASSAULTCANNON, 0.5f, ffdev_ac_slowdownspeed.GetFloat(), SEM_BOOLEAN );

	// uh....why slow the player down when they're holstering it?
	pPlayer->RemoveSpeedEffect(SE_ASSAULTCANNON);
#endif

	// Also start the rev sound for the client
#ifdef CLIENT_DLL
	if (m_pRevSound) // is this really needed? ----> && GetPlayerOwner() == CBasePlayer::GetLocalPlayer())
	{
		CSoundEnvelopeController::GetController().SoundDestroy(m_pRevSound);
		m_pRevSound = NULL;
	}
#endif

	//if (!m_fFireState) 
	//	return BaseClass::Holster(pSwitchingTo);

	//StopSound("Assaultcannon.loop_shot");	// Possibly not needed
	//EmitSound("Assaultcannon.Winddown");

	// WindDown
#ifdef CLIENT_DLL
	if (GetPlayerOwner() == CBasePlayer::GetLocalPlayer())
#endif
	{
		if( (m_flLastTick - m_flDeployTick) > 0.5 )
			WeaponSoundLocal(SPECIAL2);
		//else
		//	WeaponSoundLocal(STOP);
	}

	StopLoopShotSound();

	//m_fFireState = 0;

	CFFPlayer *pOwner = ToFFPlayer(GetOwner());
	if (pOwner)
		if ( pOwner->m_nButtons & IN_ATTACK || pOwner->m_afButtonPressed & IN_ATTACK )
			m_flTriggerReleased = gpGlobals->curtime; // we are essentially releasing trigger by holstering

	return BaseClass::Holster(pSwitchingTo);
}

//----------------------------------------------------------------------------
// Purpose: Reset state
//----------------------------------------------------------------------------
bool CFFWeaponAssaultCannon::Deploy()
{
	m_flChargeTime = 0;
	m_flLastTick = gpGlobals->curtime;
	m_flDeployTick = gpGlobals->curtime;

	m_bFiring = false;
	m_bPlayLoopShotSound = false;

	CFFPlayer *pOwner = ToFFPlayer(GetOwner());
	if (pOwner)
		if ( pOwner->m_nButtons & IN_ATTACK || pOwner->m_afButtonPressed & IN_ATTACK )
			m_flTriggerPressed = gpGlobals->curtime; // set this if we come into this while +attacking
	
#ifdef CLIENT_DLL
	// play the rev sound
	CPASAttenuationFilter filter(this);

	// Bring up the looping rev looping sound
	if (!m_pRevSound)
	{
		m_pRevSound = CSoundEnvelopeController::GetController().SoundCreate(filter, entindex(), "assaultcannon.rotate");
		CSoundEnvelopeController::GetController().Play(m_pRevSound, 0.0, ffdev_ac_revsound_pitch_low.GetFloat());
		CSoundEnvelopeController::GetController().SoundChangeVolume(m_pRevSound, ffdev_ac_revsound_volume_low.GetFloat(), 1.0f);
	}

	m_flRotationValue = 0;
#endif

	return BaseClass::Deploy();
}

//----------------------------------------------------------------------------
// Purpose: Fires bullets
//----------------------------------------------------------------------------
void CFFWeaponAssaultCannon::Fire() 
{
	Assert(0);
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	Vector vecForward;
	AngleVectors(pPlayer->EyeAngles(), &vecForward);

	FireBulletsInfo_t info(pWeaponInfo.m_iBullets, pPlayer->Weapon_ShootPosition(), vecForward, GetFireSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType);
	info.m_pAttacker = pPlayer;
	info.m_iDamage = pWeaponInfo.m_iDamage;

	Vector vecTest = info.m_vecSrc;

#ifdef CLIENT_DLL
	QAngle tmpAngle;
	pPlayer->GetViewModel()->GetAttachment(1, info.m_vecSrc, tmpAngle);
#endif

	pPlayer->FireBullets(info);

}

void CFFWeaponAssaultCannon::UpdateChargeTime()
{
	CFFPlayer *pOwner = ToFFPlayer(GetOwner());

	if (!pOwner)
		return;

	m_fFireDuration = (pOwner->m_nButtons & IN_ATTACK) ? (m_fFireDuration + gpGlobals->frametime) : 0.0f;

	// Firing has started
	if (pOwner->m_afButtonPressed & IN_ATTACK && m_flTriggerPressed < gpGlobals->curtime)
	{
		// We've logged the trigger having being released recently.
		// If the trigger was held longer than the time since the release then there
		// must be charge left on the AC.
		// We will subtract this remaining charge from the curtime for the new charge.
		if (m_flTriggerReleased > m_flTriggerPressed)
		{
			float flTimeSinceRelease = gpGlobals->curtime - m_flTriggerReleased;
			float flTimeHeld = m_flTriggerReleased - m_flTriggerPressed;
			float flTimeLeft = flTimeHeld - flTimeSinceRelease;

			if (flTimeLeft > 0 && flTimeLeft < ffdev_ac_maxchargetime.GetFloat())
			{
				m_flTriggerPressed = gpGlobals->curtime - flTimeLeft;
			}
			else
				m_flTriggerPressed = gpGlobals->curtime;
		}

		// Sometimes m_afButtonPressed seems to be set for 2 frames in a row.
		// Therefore only allow 
		else if (m_flTriggerReleased > 0)
			m_flTriggerPressed = gpGlobals->curtime;

		// Reset the trigger released
		m_flTriggerReleased = 0;
	}

	// If they have released the fire button, catch the release trigger time
	if (pOwner->m_afButtonReleased & IN_ATTACK)
	{
		m_flTriggerReleased = gpGlobals->curtime;
	}

	// If we're currently firing then the charge time is simply the time since the
	// trigger was pressed (assuming trigger actually was pressed).
	if (pOwner->m_nButtons & IN_ATTACK && m_flTriggerPressed)
	{
		m_flChargeTime = gpGlobals->curtime - m_flTriggerPressed;
	}
	// Otherwise the charge time is the amount it was held minus the amount of time
	// that has elapsed since it was released.
	else
	{
		float flTimeSinceRelease = gpGlobals->curtime - m_flTriggerReleased;
		float flTimeHeld = m_flTriggerReleased - m_flTriggerPressed;

		m_flChargeTime = flTimeHeld - flTimeSinceRelease;

		if (m_flChargeTime < 0)
			m_flChargeTime = 0;
	}

	// They might have overheated.
	// Manufacture a smooth chargetime reduction
	if (m_flNextSecondaryAttack > gpGlobals->curtime)
	{
		m_flChargeTime = ffdev_ac_maxchargetime.GetFloat() * ((m_flNextSecondaryAttack - gpGlobals->curtime) / ffdev_ac_overheatdelay.GetFloat());
	}
}

void CFFWeaponAssaultCannon::ItemPostFrame()
{
	CFFPlayer *pOwner = ToFFPlayer(GetOwner());

	if (!pOwner)
		return;

	// weapon was JUST deployed
	if (m_flLastTick == m_flDeployTick)
		m_flNextSecondaryAttack = m_flDeployTick;

	// The time since we last thought
	float flTimeDelta = gpGlobals->curtime - m_flLastTick;
	m_flLastTick = gpGlobals->curtime;

	// Keep track of fire duration for anywhere else it may be needed
	UpdateChargeTime();

#ifdef CLIENT_DLL
	UpdateBarrelSpin();
#endif

	// update pitch and volume of the loop shot sound (might make it only update the volume)
	PlayLoopShotSound();

	float flTimeSinceRelease = gpGlobals->curtime - m_flTriggerReleased;

	// Player is holding down fire. Don't allow it if we're still recovering from an overheat though
	if ((flTimeSinceRelease <= 0.5f || pOwner->m_nButtons & IN_ATTACK) && m_flNextSecondaryAttack <= gpGlobals->curtime)
	{
		// Oh no...
		if (m_flChargeTime > ffdev_ac_maxchargetime.GetFloat())
		{
			// Freeze for 5s, reduce to max rev sound so it falls away instantly
			m_flNextSecondaryAttack = gpGlobals->curtime + ffdev_ac_overheatdelay.GetFloat();
			m_flTriggerPressed = gpGlobals->curtime + ffdev_ac_overheatdelay.GetFloat();
			m_flTriggerReleased = 0; //gpGlobals->curtime;
			
			// Play the overheat sound
			WeaponSound(SPECIAL3);

			StopLoopShotSound();

#ifdef GAME_DLL
			// Remember to reset the speed soon
			pOwner->AddSpeedEffect(SE_ASSAULTCANNON, 0.5f, ffdev_ac_speedeffect_min.GetFloat(), SEM_BOOLEAN);
#endif

			m_bFiring = false;
		}

		// Time for the next real fire think
		else if (m_flChargeTime >= ffdev_ac_chargeuptime.GetFloat() && m_flNextPrimaryAttack <= gpGlobals->curtime)
		{
			// Out of ammo
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
			{
				//WeaponSound(STOP);
				StopLoopShotSound();

				HandleFireOnEmpty();
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.2f;

#ifdef GAME_DLL
				pOwner->RemoveSpeedEffect(SE_ASSAULTCANNON);
#endif

				m_bFiring = false;
			}
			// Weapon should be firing now
			else
			{
				// If the firing button was just pressed, reset the firing time
				if (pOwner && pOwner->m_afButtonPressed & IN_ATTACK)
					m_flNextPrimaryAttack = gpGlobals->curtime;

#ifdef GAME_DLL
				// base the speed effect on how charged the ac is
				float flSpeed = ffdev_ac_speedeffect_max.GetFloat() - ( (ffdev_ac_speedeffect_max.GetFloat() - ffdev_ac_speedeffect_min.GetFloat()) * (m_flChargeTime / ffdev_ac_maxchargetime.GetFloat()) );
				CFFPlayer *pPlayer = GetPlayerOwner();
				pPlayer->AddSpeedEffect(SE_ASSAULTCANNON, 0.5f, flSpeed, SEM_BOOLEAN);
#endif

				m_flPlaybackRate = 1.0f + m_flChargeTime;
				PrimaryAttack();
			}

			//m_flNextPrimaryAttack = gpGlobals->curtime + 1.0f;

			//float flT = (m_flChargeTime - ffdev_ac_chargeuptime.GetFloat()) / (2.0f * ffdev_ac_maxchargetime.GetFloat());

			//m_flNextPrimaryAttack = gpGlobals->curtime + (GetFFWpnData().m_flCycleTime * (flT > 0.0f ? 1.0f : 1 - flT));

			if (!m_bFiring && m_flChargeTime > ffdev_ac_chargeuptime.GetFloat())
			{
				//WeaponSound(SINGLE);

#ifdef GAME_DLL
				CFFPlayer *pPlayer = GetPlayerOwner();
				pPlayer->AddSpeedEffect(SE_ASSAULTCANNON, 0.5f, ffdev_ac_speedeffect_max.GetFloat(), SEM_BOOLEAN);
#endif

				m_bFiring = true;
			}
		}
	}
	// No buttons down
	else
	{
		// Reduce speed at 3 times the rate
		if (m_flChargeTime > 0)
		{
			m_flChargeTime -= flTimeDelta;

			if (m_flChargeTime < 0)
				m_flChargeTime = 0;
		}
		
		if (m_bFiring)
		{
			//WeaponSound(STOP);
			StopLoopShotSound();

#ifdef GAME_DLL
			CFFPlayer *pPlayer = GetPlayerOwner();
			pPlayer->RemoveSpeedEffect(SE_ASSAULTCANNON);
#endif

			m_bFiring = false;
		}

		WeaponIdle();
	}

#ifdef CLIENT_DLL
	// the volume fades up while deploying, so wait a second before doing this stuff
	if (m_pRevSound && gpGlobals->curtime - m_flDeployTick > 1.0f)
	{
		float flPitch = FLerp(ffdev_ac_revsound_pitch_low.GetFloat(), ffdev_ac_revsound_pitch_high.GetFloat(), m_flChargeTime / ffdev_ac_maxchargetime.GetFloat());
		float flVolume = FLerp(ffdev_ac_revsound_volume_low.GetFloat(), ffdev_ac_revsound_volume_high.GetFloat(), m_flChargeTime / ffdev_ac_maxchargetime.GetFloat());
		CSoundEnvelopeController::GetController().SoundChangePitch(m_pRevSound, flPitch, 0.0f);
		CSoundEnvelopeController::GetController().SoundChangeVolume(m_pRevSound, flVolume, 0.0f);
	}
#endif
}

//----------------------------------------------------------------------------
// Purpose: Precache some extra sounds
//----------------------------------------------------------------------------
void CFFWeaponAssaultCannon::Precache() 
{
	PrecacheScriptSound("Assaultcannon.single_shot");
	PrecacheScriptSound("Assaultcannon.loop_shot");
	PrecacheScriptSound("Assaultcannon.Windup");
	PrecacheScriptSound("Assaultcannon.Winddown");
	PrecacheScriptSound("assaultcannon.rotate");
	PrecacheScriptSound("assaultcannon.overheat");
	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Precache some extra sounds
//----------------------------------------------------------------------------
void CFFWeaponAssaultCannon::PrimaryAttack()
{
	// Only the player fires this way so we can cast
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());

	if (!pPlayer)
		return;

	// Undisguise
#ifdef GAME_DLL
	pPlayer->ResetDisguise();
#endif

	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	float fireRate = GetFireRate();

	// MUST call sound before removing a round from the clip of a CMachineGun
	if (fireRate > ffdev_ac_loopshotsound_rate_max.GetFloat())
	{
		// only play the single shot sound
		WeaponSound(SINGLE);

		// stop playing the loop shot sound
		StopLoopShotSound();
	}
	else if (fireRate > ffdev_ac_loopshotsound_rate_min.GetFloat())
	{
		// Play both shot sounds while we're in between min and max
		WeaponSound(SINGLE);
		m_bPlayLoopShotSound = true;
	}
	else
	{
		// only play the loop shot sound while below min
		m_bPlayLoopShotSound = true;
	}

	if (m_bMuzzleFlash)
		pPlayer->DoMuzzleFlash();

	SendWeaponAnim(GetPrimaryAttackActivity());

	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	int iBulletsToFire = 0;

	// moving up above to test whether or not to play the loop shot sound
	//float fireRate = GetFireRate();

	while ( m_flNextPrimaryAttack <= gpGlobals->curtime )
	{
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		iBulletsToFire++;
	}

	if (iBulletsToFire == 0)
		return;

	// Make sure we can't fire more bullets that we have
	iBulletsToFire = min(iBulletsToFire, pPlayer->GetAmmoCount(m_iPrimaryAmmoType));

#ifdef GAME_DLL
	pPlayer->RemoveAmmo(iBulletsToFire, m_iPrimaryAmmoType);
#endif

	Vector vecForward;
	AngleVectors(pPlayer->EyeAngles(), &vecForward);

	FireBulletsInfo_t info(iBulletsToFire * pWeaponInfo.m_iBullets, 
		pPlayer->Weapon_ShootPosition(), 
		vecForward, 
		Vector(pWeaponInfo.m_flBulletSpread, pWeaponInfo.m_flBulletSpread, pWeaponInfo.m_flBulletSpread), 
		MAX_TRACE_LENGTH, 
		m_iPrimaryAmmoType);
	info.m_pAttacker = pPlayer;
	info.m_iDamage = (iBulletsToFire * pWeaponInfo.m_iBullets) * pWeaponInfo.m_iDamage;

	Vector vecTest = info.m_vecSrc;

#ifdef CLIENT_DLL
	QAngle tmpAngle;
	pPlayer->GetViewModel()->GetAttachment(1, info.m_vecSrc, tmpAngle);
#endif

	pPlayer->FireBullets(info);

	//m_flNextPrimaryAttack = gpGlobals->curtime + GetFFWpnData().m_flCycleTime;

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	}

	//Add our view kick in
	pPlayer->ViewPunchReset();
	pPlayer->ViewPunch(QAngle(-GetFFWpnData().m_flRecoilAmount, 0, 0));
}

float CFFWeaponAssaultCannon::GetFireRate()
{
	float t = m_flChargeTime / ffdev_ac_maxchargetime.GetFloat();
	t = clamp(t, 0.0f, 1.0f);
	t = SimpleSpline(t);

	return ffdev_ac_maxcycletime.GetFloat() * (1.0f - t) + ffdev_ac_mincycletime.GetFloat() * t;
}

Vector CFFWeaponAssaultCannon::GetFireSpread()
{
	float t = m_flChargeTime / ffdev_ac_maxchargetime.GetFloat();
	t = clamp(t, 0.0f, 1.0f);
	t = SimpleSpline(t);

	float flSpread = ffdev_ac_minspread.GetFloat() * (1.0f - t) + ffdev_ac_maxspread.GetFloat() * t;

	return Vector(flSpread, flSpread, flSpread);
}

void CFFWeaponAssaultCannon::PlayLoopShotSound()
{
	if (!m_bPlayLoopShotSound)
	{
		StopLoopShotSound();
		return;
	}

	const char *shootsound = GetShootSound( BURST );
	if (!shootsound || !shootsound[0])
		return;

	float flFireRate = GetFireRate();

	float flSoundRateMax = ffdev_ac_loopshotsound_rate_max.GetFloat();
	float flSoundRateMin = ffdev_ac_loopshotsound_rate_min.GetFloat();
	float flPercentForVolume = 1.0f;
	if (flSoundRateMax - flSoundRateMin != 0)
		flPercentForVolume = clamp((flSoundRateMax - clamp(flFireRate, flSoundRateMin, flSoundRateMax)) / (flSoundRateMax - flSoundRateMin), 0.0f, 1.0f);

	float flMaxCycleTime = ffdev_ac_maxcycletime.GetFloat();
	float flMinCycleTime = ffdev_ac_mincycletime.GetFloat();
	float flPercentForPitch = 1.0f;
	if (flMaxCycleTime - flMinCycleTime != 0)
		flPercentForPitch = clamp((flMaxCycleTime - clamp(flFireRate, flMinCycleTime, flMaxCycleTime)) / (flMaxCycleTime - flMinCycleTime), 0.0f, 1.0f);

	EmitSound_t params;
	params.m_pSoundName = shootsound;
	params.m_flSoundTime = 0.0f;
	params.m_pOrigin = NULL;
	params.m_pflSoundDuration = NULL;
	params.m_bWarnOnDirectWaveReference = true;
	params.m_SoundLevel = SNDLVL_GUNFIRE;
	params.m_flVolume = ffdev_ac_loopshotsound_volume_low.GetInt() + ((ffdev_ac_loopshotsound_volume_high.GetInt() - ffdev_ac_loopshotsound_volume_low.GetInt()) * flPercentForVolume);
	params.m_nPitch = ffdev_ac_loopshotsound_pitch_low.GetInt() + ((ffdev_ac_loopshotsound_pitch_high.GetInt() - ffdev_ac_loopshotsound_pitch_low.GetInt()) * flPercentForPitch);
	params.m_nFlags = SND_CHANGE_PITCH | SND_CHANGE_VOL;

	CPASAttenuationFilter filter( GetOwner(), params.m_SoundLevel );
	if ( IsPredicted() )
	{
		filter.UsePredictionRules();
	}
	EmitSound( filter, entindex(), params, params.m_hSoundScriptHandle );

	m_bPlayLoopShotSound = true;
}

void CFFWeaponAssaultCannon::StopLoopShotSound()
{
	//if ( IsPredicted() )
	//	return;

	// If we have some sounds from the weapon classname.txt file, play a random one of them
	const char *shootsound = GetShootSound( BURST );
	if ( !shootsound || !shootsound[0] )
		return;

	StopSound( entindex(), shootsound );

	m_bPlayLoopShotSound = false;
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: Keep the barrels spinning
//-----------------------------------------------------------------------------
void CFFWeaponAssaultCannon::UpdateBarrelSpin()
{
/*
	CFFPlayer *pOwner = ToFFPlayer(GetOwner());

	if (!pOwner)
		return;

	// A buffered version of m_flChargeTime, if you will.
	// This is to stop the jerkiness that is being annoying.
	if (pOwner->m_nButtons & IN_ATTACK && m_flNextSecondaryAttack <= gpGlobals->curtime)
	{
		m_flChargeTimeClient = max(m_flChargeTimeClient, m_flChargeTime);
	}
	else
	{
		m_flChargeTimeClient = min(m_flChargeTimeClient, m_flChargeTime);
	}

	// Max spinning speed
	if (m_flChargeTimeClient > 1.5f)
		m_flChargeTimeClient = 1.5f;

	CBaseViewModel *pVM = pOwner->GetViewModel();

	if (m_iBarrelRotation < 0)
	{
		m_iBarrelRotation = pVM->LookupPoseParameter("ac_rotate");
	}

	// Might need to separate m_flRotationValue from m_flChargeTime
	// Perhaps a separate client-side variable to track it
	m_flRotationValue += m_flChargeTimeClient * 3.0f;
	m_flRotationValue = pVM->SetPoseParameter(m_iBarrelRotation, m_flRotationValue);
*/

	// time for a new method

	CFFPlayer *pOwner = ToFFPlayer(GetOwner());
	if (!pOwner)
		return;

	CBaseViewModel *pVM = pOwner->GetViewModel();
	if (!pVM)
		return;

	if (m_iBarrelRotation < 0)
		m_iBarrelRotation = pVM->LookupPoseParameter("ac_rotate");

	// FLerp rules
	// the ac plays that idle rev sound, so I'm just gonna make this shit spin all the time
	m_flRotationValue += FLerp( ffdev_ac_barrelrotation_speed_min.GetFloat(), ffdev_ac_barrelrotation_speed_max.GetFloat(), m_flChargeTime / ffdev_ac_maxchargetime.GetFloat() );

	m_flRotationValue = pVM->SetPoseParameter(m_iBarrelRotation, m_flRotationValue);
}
#endif

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: This is a awful function to quickly get the AC charge. It will be
//			replaced tomorrow.
//-----------------------------------------------------------------------------
float GetAssaultCannonCharge()
{
	C_FFPlayer *pPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());

	if (!pPlayer)
		return 0.0f;

	C_FFWeaponBase *pWeapon = pPlayer->GetActiveFFWeapon();

	if (!pWeapon || pWeapon->GetWeaponID() != FF_WEAPON_ASSAULTCANNON)
		return 0.0f;

	CFFWeaponAssaultCannon *pAC = (CFFWeaponAssaultCannon *) pWeapon;

	return (100.0f * pAC->m_flChargeTime / ffdev_ac_maxchargetime.GetFloat());
}
#endif
