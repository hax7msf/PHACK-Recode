#include "../includes.h"
#include "../FEATURES/trash.h"
#include "../UTILS/interfaces.h"
#include "../SDK/IEngine.h"
#include "../SDK/CUserCmd.h"
#include "../SDK/ConVar.h"
#include "../SDK/CGlobalVars.h"
#include "../SDK/IViewRenderBeams.h"
#include "../FEATURES/Backtracking.h"
#include "../SDK/CBaseEntity.h"
#include "../SDK/CClientEntityList.h"
#include "../SDK/CBaseWeapon.h"
#include "../SDK/CTrace.h"	
#include "../FEATURES/Resolver.h"
#include "../SDK/CGlobalVars.h"
#include "../FEATURES/NewVisuals.h"
#include "../UTILS/render.h"
#include "../SDK/IVDebugOverlay.h"
#include "../FEATURES/AutoWall.h"
#include "../FEATURES/GrenadePrediction.h"
#include "../SDK/Collideable.h"
#include <string.h>
#include "Aimbot.h"
#include <map>
int alpha[65];
CColor breaking;
CColor backtrack;
static bool bPerformed = false, bLastSetting;
float fade_alpha[65];
float dormant_time[65];
CColor main_color;
CColor ammo;
int floatingTextsIdx = 0;
CVisuals::ESPBox Box;
#define clamp(val, min, max) (((val) > (max)) ? (max) : (((val) < (min)) ? (min) : (val)))
void CVisuals::set_hitmarker_time(float time)
{
	GLOBAL::flHurtTime = time;
}
bool CVisuals::GetBox(SDK::CBaseEntity* pEntity, CVisuals::ESPBox &result)
{
	Vector  vOrigin, min, max, sMin, sMax, sOrigin,
		flb, brt, blb, frt, frb, brb, blt, flt;
	float left, top, right, bottom;

	vOrigin = pEntity->GetVecOrigin();
	min = pEntity->GetCollideable()->Mins() + vOrigin;
	max = pEntity->GetCollideable()->Maxs() + vOrigin;

	Vector points[] = { Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z) };

	if (!RENDER::WorldToScreen(points[3], flb) || !RENDER::WorldToScreen(points[5], brt)
		|| !RENDER::WorldToScreen(points[0], blb) || !RENDER::WorldToScreen(points[4], frt)
		|| !RENDER::WorldToScreen(points[2], frb) || !RENDER::WorldToScreen(points[1], brb)
		|| !RENDER::WorldToScreen(points[6], blt) || !RENDER::WorldToScreen(points[7], flt))
		return false;

	Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

	left = flb.x;
	top = flb.y;
	right = flb.x;
	bottom = flb.y;

	for (int i = 1; i < 8; i++)
	{
		if (left > arr[i].x)
			left = arr[i].x;
		if (bottom < arr[i].y)
			bottom = arr[i].y;
		if (right < arr[i].x)
			right = arr[i].x;
		if (top > arr[i].y)
			top = arr[i].y;
	}

	result.x = left;
	result.y = top;
	result.w = right - left;
	result.h = bottom - top;

	return true;
}
void CVisuals::Draw()
{
	for (int i = 1; i <= INTERFACES::Engine->GetMaxClients(); i++)
	{
		auto entity = INTERFACES::ClientEntityList->GetClientEntity(i);
		auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());

		if (!entity)
			continue;

		if (!local_player)
			continue;



		if (!entity->IsAlive())
			continue;
		bool is_local_player = entity == local_player;
		bool is_teammate = local_player->GetTeam() == entity->GetTeam() && !is_local_player;

		if (is_local_player)
			continue;

		if (is_teammate)
			continue;

		if (entity->GetHealth() <= 0)
			continue;

		//--- Colors ---//
		int enemy_hp = entity->GetHealth();
		int hp_red = 255 - (enemy_hp * 2.55);
		int hp_green = enemy_hp * 2.55;
		CColor health_color = CColor(hp_red, hp_green, 1, alpha[entity->GetIndex()]);
		CColor dormant_color = CColor(100, 100, 100, alpha[entity->GetIndex()]);
		CColor box_color;
		CColor still_health;
		CColor alt_color;
		CColor zoom_color;
		CColor bomb_color;
		CColor lby;

		if (entity->GetIsDormant())
		{
			if (alpha[entity->GetIndex()] > 0)
				alpha[entity->GetIndex()] -= 0.4;
			main_color = dormant_color;
			still_health = dormant_color;
			alt_color = dormant_color;
			zoom_color = dormant_color;
			breaking = dormant_color;
			backtrack = dormant_color;
			box_color = dormant_color;
			//bomb_color = dormant_color;
			ammo = dormant_color;
			lby = dormant_color;
		}
		else if (!entity->GetIsDormant())
		{
			alpha[entity->GetIndex()] = 255;
			main_color = CColor(255, 255, 255, alpha[entity->GetIndex()]); //heath_color
			still_health = health_color;
			alt_color = BLACK;
			//zoom_color = SETTINGS::settings.info_col;
			breaking = CColor(220, 150, 150);
			backtrack = CColor(155, 220, 150);
			box_color = CColor(SETTINGS::settings.boxer_col[0] * 255, SETTINGS::settings.boxer_col[1] * 255, SETTINGS::settings.boxer_col[2] * 255, SETTINGS::settings.boxer_col[3] * 255);
			//	bomb_color = CColor(244, 66, 66, 150);
			//	ammo = CColor(SETTINGS::settings.ammo_col[0] * 255, SETTINGS::settings.ammo_col[1] * 255, SETTINGS::settings.ammo_col[2] * 255, SETTINGS::settings.ammo_col[3] * 255);
			//	lby = CColor(SETTINGS::settings.lbycolor[0] * 255, SETTINGS::settings.lbycolor[1] * 255, SETTINGS::settings.lbycolor[2] * 255, SETTINGS::settings.lbycolor[3] * 255);

		}
		Vector min, max, pos, pos3D, top, top3D; entity->GetRenderBounds(min, max);
		pos3D = entity->GetAbsOrigin() - Vector(0, 0, 10); top3D = pos3D + Vector(0, 0, max.z + 10);

		if (RENDER::WorldToScreen(pos3D, pos) && RENDER::WorldToScreen(top3D, top))
		{
			if (SETTINGS::settings.zeusrange) DrawZeusRange(entity);

			if (!is_teammate)
			{
				if (GetBox(entity, Box))
				{
					if (SETTINGS::settings.box_bool) DrawBox(Box, box_color, entity);
					if (SETTINGS::settings.health_bool) DrawHealth(entity, Box);
					if (SETTINGS::settings.weap_bool)DrawDropped(entity);
				}

				if (SETTINGS::settings.name_bool) DrawName(entity, main_color, i, pos, top);
				if (SETTINGS::settings.name_bool) DrawDamageIndicator();
				//	if (SETTINGS::settings.weap_bool) DrawWeapon(entity, main_color, i, pos, top);
			//	if (SETTINGS::settings.ammo_bool) DrawAmmo(entity, ammo, main_color, pos, top);
			}
			else if (is_teammate)
			{
				//	if (SETTINGS::settings.box_bool) DrawBox(Box, box_color, entity);
				if (SETTINGS::settings.nameteam) DrawName(entity, SETTINGS::settings.nameteam_col, i, pos, top);
				if (SETTINGS::settings.weaponteam) DrawWeapon(entity, SETTINGS::settings.weaponteam_col, i, pos, top);
				//	if (SETTINGS::settings.health_bool) DrawHealth(entity, Box);
				//if (SETTINGS::settings.ammoteam) DrawAmmo(entity, ammo, alt_color, pos, top);
			}
			penetration_reticle();
			misc_visuals();
			DrawInfo(entity, main_color, zoom_color, pos, top);
		}

		if (!is_teammate)
		{
			if (SETTINGS::settings.fov_bool) DrawFovArrows(entity, CColor(SETTINGS::settings.fov_col.RGBA[0], SETTINGS::settings.fov_col.RGBA[1], SETTINGS::settings.fov_col.RGBA[2]));
		}
		else if (is_teammate)
		{
			if (SETTINGS::settings.arrowteam) DrawFovArrows(entity, CColor(SETTINGS::settings.arrowteam_col.RGBA[0], SETTINGS::settings.arrowteam_col.RGBA[1], SETTINGS::settings.arrowteam_col.RGBA[2]));
		}
		DrawCrosshair();
	}
}
void CVisuals::SpecList()
{
	auto m_pLocalPlayer = reinterpret_cast<SDK::CBaseEntity*>(INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer()));

	RECT scrn = INTERFACES::Engine->GetViewport();
	int ayy = 0;
	int screen_width, screen_height;
	INTERFACES::Engine->GetScreenSize(screen_width, screen_height);
	for (int i = 0; i < INTERFACES::ClientEntityList->GetHighestEntityIndex(); i++)
		if (SETTINGS::settings.aa_bool)
		{
			SDK::CBaseEntity* entity = INTERFACES::ClientEntityList->GetClientEntity(i);
			SDK::player_info_t pinfo;

			if (entity &&  entity != m_pLocalPlayer)
			{
				if (INTERFACES::Engine->GetPlayerInfo(i, &pinfo) && !entity->IsAlive() && !entity->GetIsDormant())
				{
					HANDLE obs = entity->GetObserverModeHandle();

					if (obs)
					{
						SDK::CBaseEntity* pTarget = INTERFACES::ClientEntityList->GetClientEntity(i);
						SDK::player_info_t pinfo2;

						if (INTERFACES::Engine->GetPlayerInfo(pTarget->GetIndex(), &pinfo2))
						{
							char buf1[255];
							sprintf_s(buf1, "%s", pinfo.name);

							RENDER::DrawF(1890, screen_height - 1065 + (ayy * 10), FONTS::visuals_name_font, true, true, CColor(255, 255, 255, 255), buf1);

							ayy++;

						}
					}
				}
			}
		}
}
void CVisuals::DrawZeusRange(SDK::CBaseEntity* entity) {
	if (!INTERFACES::Engine->IsConnected() || !INTERFACES::Engine->IsInGame())
		return;
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!local_player) return;

	auto c_baseweapon = reinterpret_cast<SDK::CBaseWeapon*>(INTERFACES::ClientEntityList->GetClientEntity(entity->GetActiveWeaponIndex()));
	if (!c_baseweapon) return;

	auto weapon = reinterpret_cast<SDK::CBaseWeapon*>(INTERFACES::ClientEntityList->GetClientEntity(local_player->GetActiveWeaponIndex()));
	if (!weapon)
		return;

	//	if (c_baseweapon->GetItemDefenitionIndex() == SDK::ItemDefinitionIndex::WEAPON_TASER)
		//	return;
	if (weapon->GetItemDefenitionIndex() == SDK::ItemDefinitionIndex::WEAPON_TASER)
	{
		if (!in_tp) return;
		float step = M_PI * 2.0 / 1023;
		float rad = SETTINGS::settings.zeus_range;
		//float rad = 178.f; //���
		//Vector origin = Vector(Globals::eyePos.x, Globals::eyePos.y - 25, Globals::eyePos.z);
		Vector origin = local_player->GetEyePosition();

		static double rainbow;

		Vector screenPos;
		static Vector prevScreenPos;

		for (float rotation = 0; rotation < (M_PI * 2.0); rotation += step)
		{
			Vector pos(rad * cos(rotation) + origin.x, rad * sin(rotation) + origin.y, origin.z);

			SDK::Ray_t ray;
			SDK::trace_t trace;
			SDK::CTraceFilter filter;

			filter.pSkip1 = local_player;
			ray.Init(origin, pos);

			INTERFACES::Trace->TraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace);

			if (INTERFACES::DebugOverlay->ScreenPosition(trace.end, screenPos))
				continue;

			if (!prevScreenPos.IsZero() && !screenPos.IsZero() && screenPos.DistTo(Vector(-107374176, -107374176, -107374176)) > 3.f&& prevScreenPos.DistTo(Vector(-107374176, -107374176, -107374176)) > 3.f) {
				rainbow += 0.00001;
				if (rainbow > 1.f)
					rainbow = 0;

				CColor color = CColor::FromHSB(rainbow, 1.f, 1.f);


				RENDER::DrawLine(prevScreenPos.x, prevScreenPos.y, screenPos.x, screenPos.y, color);
				RENDER::DrawLine(prevScreenPos.x, prevScreenPos.y + 1, screenPos.x, screenPos.y + 1, color);
				//Draw::Line(Vector2D(prevScreenPos.x, prevScreenPos.y - 1), Vector2D(screenPos.x, screenPos.y - 1), color);
			}
			prevScreenPos = screenPos;
		}
	}
}
void CVisuals::ClientDraw()
{
	if (SETTINGS::settings.trash)
	{
		Gettergay->ModulateWorld2();
		Gettergay->Skycolorchanger2();
		Gettergay->Worldcolorchanger2();
	//	Worldcolorchanger();
	}
	DrawIndicator();
	DrawHitmarker();
	if (SETTINGS::settings.spread_bool == 1) DrawInaccuracy();
   //if (SETTINGS::settings.spread_bool == 2) DrawInaccuracy1();
	if (SETTINGS::settings.scope_bool) DrawBorderLines();
	if (SETTINGS::settings.aa_lines) {
		AntiAimLines();
	}
	if (SETTINGS::settings.sky_changer)
	{
		static SDK::ConVar* sv_skyname = INTERFACES::cvar->FindVar("sv_skyname");
		sv_skyname->nFlags &= ~FCVAR_CHEAT;
		switch (SETTINGS::settings.sky_type)
		{
		case 0: sv_skyname->SetValue("sky_l4d_rural02_ldr"); break;
		case 1: sv_skyname->SetValue("cs_baggage_skybox_"); break;
		case 2: sv_skyname->SetValue("cs_tibet"); break;
		case 3: sv_skyname->SetValue("embassy"); break;
		case 4: sv_skyname->SetValue("italy"); break;
		case 5: sv_skyname->SetValue("jungle"); break;
		case 6: sv_skyname->SetValue("office"); break;
		case 7: sv_skyname->SetValue("sky_cs15_daylight01_hdr"); break;
		case 8: sv_skyname->SetValue("sky_cs15_daylight02_hdr"); break;
		case 9: sv_skyname->SetValue("sky_day02_05"); break;
		case 10: sv_skyname->SetValue("nukeblank"); break;
		case 11: sv_skyname->SetValue("sky_venice"); break;
		case 12: sv_skyname->SetValue("sky_cs15_daylight03_hdr"); break;
		case 13: sv_skyname->SetValue("sky_cs15_daylight04_hdr"); break;
		case 14: sv_skyname->SetValue("sky_csgo_cloudy01"); break;
		case 15: sv_skyname->SetValue("sky_csgo_night02"); break;
		case 16: sv_skyname->SetValue("sky_csgo_night02b"); break;
		case 17: sv_skyname->SetValue("vertigo"); break;
		case 18: sv_skyname->SetValue("vertigoblue_hdr"); break;
		case 19: sv_skyname->SetValue("sky_dust"); break;
		case 20: sv_skyname->SetValue("vietnam"); break;
		case 21: sv_skyname->SetValue("sky2mh_"); break;
		case 22: sv_skyname->SetValue("clear_night_sky"); break;
		}
	}
	//fix for cool
	if (SETTINGS::settings.spread_bool != 0)
		INTERFACES::cvar->FindVar("crosshair")->SetValue(0);
	else
		INTERFACES::cvar->FindVar("crosshair")->SetValue(1);
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!local_player) // ����� �����
		return;
	if (INTERFACES::Globals->absoluteframetime < 10)
	{
		SETTINGS::settings.trash = false;
	}
	if (INTERFACES::Globals->absoluteframetime > 10)
	{
		SETTINGS::settings.trash = true;
	}
}
void CVisuals::misc_visuals() {

	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());

	SDK::ConVar* sniper_crosshair = INTERFACES::cvar->FindVar("weapon_debug_spread_show");

	if (!local_player->GetIsScoped())
		sniper_crosshair->SetValue(3);
	else
		sniper_crosshair->SetValue(0);
}
void CVisuals::penetration_reticle() {

	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer()); if (!local_player) return;

	auto weapon = reinterpret_cast<SDK::CBaseWeapon*>(INTERFACES::ClientEntityList->GetClientEntity(local_player->GetActiveWeaponIndex())); if (!weapon) return;

	Vector aimPoint = local_player->GetEyePosition();

	int screen_x, screen_y;
	INTERFACES::Engine->GetScreenSize(screen_x, screen_y);

	if (AutoWall->GetDamagez(aimPoint) >= 1.f)//settings::options.damage_val)
		RENDER::DrawFilledRect(screen_x / 2, screen_y / 2, 2, 2, CColor(0, 255, 0));
	else
		RENDER::DrawFilledRect(screen_x / 2, screen_y / 2, 2, 2, CColor(255, 0, 0));
}
void CVisuals::DrawBox(CVisuals::ESPBox size, CColor color, SDK::CBaseEntity* pEntity)
{
	int VertLine = (((float)size.w) * (0.20f));
	int HorzLine = (((float)size.h) * (0.30f));

	RENDER::Clear(size.x, size.y - 1, VertLine, 1, CColor(0, 0, 0, 255));
	RENDER::Clear(size.x + size.w - VertLine, size.y - 1, VertLine, 1, CColor(0, 0, 0, 255));
	RENDER::Clear(size.x, size.y + size.h - 1, VertLine, 1, CColor(0, 0, 0, 255));
	RENDER::Clear(size.x + size.w - VertLine, size.y + size.h - 1, VertLine, 1, CColor(0, 0, 0, 255));

	RENDER::Clear(size.x - 1, size.y, 1, HorzLine, CColor(0, 0, 0, 255));
	RENDER::Clear(size.x - 1, size.y + size.h - HorzLine, 1, HorzLine, CColor(0, 0, 0, 255));
	RENDER::Clear(size.x + size.w - 1, size.y, 1, HorzLine, CColor(0, 0, 0, 255));
	RENDER::Clear(size.x + size.w - 1, size.y + size.h - HorzLine, 1, HorzLine, CColor(0, 0, 0, 255));

	RENDER::Clear(size.x, size.y, VertLine, 1, color);
	RENDER::Clear(size.x + size.w - VertLine, size.y, VertLine, 1, color);
	RENDER::Clear(size.x, size.y + size.h, VertLine, 1, color);
	RENDER::Clear(size.x + size.w - VertLine, size.y + size.h, VertLine, 1, color);

	RENDER::Clear(size.x, size.y, 1, HorzLine, color);
	RENDER::Clear(size.x, size.y + size.h - HorzLine, 1, HorzLine, color);
	RENDER::Clear(size.x + size.w, size.y, 1, HorzLine, color);
	RENDER::Clear(size.x + size.w, size.y + size.h - HorzLine, 1, HorzLine, color);

}
void CVisuals::DrawName(SDK::CBaseEntity* entity, CColor color, int index, Vector pos, Vector top)
{
	SDK::player_info_t ent_info;
	Vector min, max;
	entity->GetRenderBounds(min, max);
	Vector pos3D, top3D;
	pos3D = entity->GetAbsOrigin() - Vector(0, 0, 10);
	top3D = pos3D + Vector(0, 0, max.z + 10);
	INTERFACES::Engine->GetPlayerInfo(index, &ent_info);

	if (RENDER::WorldToScreen(pos3D, pos) && RENDER::WorldToScreen(top3D, top))
	{
		wchar_t buffer[128];
		wsprintfW(buffer, L"%S", ent_info.name);
		if (MultiByteToWideChar(CP_UTF8, 0, ent_info.name, -1, buffer, 128) > 0)
		{
			int height = (pos.y - top.y);
			int width = height / 2;
			RENDER::DrawF(pos.x, top.y - 7, FONTS::visuals_name_font, true, true, color, ent_info.name); //numpad_menu_font
		}
	}
}
std::string fix_item_name(std::string name)
{
	if (name[0] == 'C')
		name.erase(name.begin());

	auto startOfWeap = name.find("Weapon");
	if (startOfWeap != std::string::npos)
		name.erase(name.begin() + startOfWeap, name.begin() + startOfWeap + 6);

	return name;
}

void CVisuals::DrawWeapon(SDK::CBaseEntity* entity, CColor color, int index, Vector pos, Vector top)
{
	SDK::player_info_t ent_info; INTERFACES::Engine->GetPlayerInfo(index, &ent_info);

	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!local_player) return;

	auto weapon = INTERFACES::ClientEntityList->GetClientEntity(entity->GetActiveWeaponIndex());
	if (!weapon) return;

	auto c_baseweapon = reinterpret_cast<SDK::CBaseWeapon*>(INTERFACES::ClientEntityList->GetClientEntity(entity->GetActiveWeaponIndex()));
	if (!c_baseweapon) return;

	bool is_teammate = local_player->GetTeam() == entity->GetTeam(), distanceThing, distanceThing2;
	if (SETTINGS::settings.ammo_bool) distanceThing = true; else distanceThing = false; if (SETTINGS::settings.ammoteam) distanceThing2 = true; else distanceThing2 = false;
	int height = (pos.y - top.y), width = height / 2, distanceOn = distanceThing ? pos.y + 12 : pos.y + 8, distanceOn2 = distanceThing2 ? pos.y + 12 : pos.y + 8;

	if (c_baseweapon->is_revolver())
		RENDER::DrawF(pos.x, is_teammate ? distanceOn2 : distanceOn, FONTS::visuals_esp_font, true, true, color, "R8 REVOLVER");

	else if (c_baseweapon->GetItemDefenitionIndex() == SDK::WEAPON_USP_SILENCER)
		RENDER::DrawF(pos.x, is_teammate ? distanceOn2 : distanceOn, FONTS::visuals_esp_font, true, true, color, "USP-S");

	else if (c_baseweapon->GetItemDefenitionIndex() == SDK::WEAPON_M4A1_SILENCER)
		RENDER::DrawF(pos.x, is_teammate ? distanceOn2 : distanceOn, FONTS::visuals_esp_font, true, true, color, "M4A1-S");
	else
		RENDER::DrawF(pos.x, is_teammate ? distanceOn2 : distanceOn, FONTS::visuals_esp_font, true, true, color, fix_item_name(weapon->GetClientClass()->m_pNetworkName));

}
void CVisuals::DrawDamageIndicator()
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());

	if (!local_player->GetHealth() > 0)
		return;

	float CurrentTime = local_player->GetTickBase() * INTERFACES::Globals->interval_per_tick;

	for (int i = 0; i < dmg_indicator.size(); i++) {
		if (dmg_indicator[i].earse_time < CurrentTime) {
			dmg_indicator.erase(dmg_indicator.begin() + i);
			continue;
		}

		if (!dmg_indicator[i].initializes) {
			dmg_indicator[i].Position = dmg_indicator[i].player->GetBonePosition(6);
			dmg_indicator[i].initializes = true;
		}

		if (CurrentTime - dmg_indicator[i].last_update > 0.0001f) {
			dmg_indicator[i].Position.z -= (0.1f * (CurrentTime - dmg_indicator[i].earse_time));
			dmg_indicator[i].last_update = CurrentTime;

			Vector ScreenPosition;

			if (RENDER::WorldToScreen(dmg_indicator[i].Position, ScreenPosition)) {
				RENDER::DrawF(ScreenPosition.x, ScreenPosition.y, FONTS::menu_window_font, true, true, CColor(SETTINGS::settings.awhit[0] * 255, SETTINGS::settings.awhit[1] * 255, SETTINGS::settings.awhit[2] * 255, SETTINGS::settings.awhit[3] * 255), std::to_string(dmg_indicator[i].dmg).c_str());
			}
		}

	}

}
void CVisuals::DrawHealth(SDK::CBaseEntity* entity, CVisuals::ESPBox size)
{
	size.y += (size.h + 6);
	size.h = 4;
	float HealthValue = entity->GetHealth();
	float HealthPerc = HealthValue / 100.f;
	float Width = (size.w * HealthPerc);
	size.w = Width;
	// --  Main Bar -- //
	if (HealthValue > 100)
		HealthValue = 100;
	Vector min, max;
	entity->GetRenderBounds(min, max);
	Vector pos, pos3D, top, top3D;
	pos3D = entity->GetAbsOrigin() - Vector(0, 0, 10);
	top3D = pos3D + Vector(0, 0, max.z + 10);

	int height = (pos.y - top.y);
	int width = height / 2;
	UINT hp = height - (UINT)((height * HealthValue) / 100);
	SDK::Vertex_t Verts[4];
	Verts[0].Init(Vector2D(size.x, size.y));
	Verts[1].Init(Vector2D(size.x + size.w + 5, size.y));
	Verts[2].Init(Vector2D(size.x + size.w, size.y + 5));
	Verts[3].Init(Vector2D(size.x - 5, size.y + 5));

	RENDER::PolygonOutline(4, Verts, CColor(0, 247, 132, 255), CColor(255, 255, 255, 170));

	SDK::Vertex_t Verts2[4];
/*	Verts2[0].Init(Vector2D(size.x + 1, size.y + 1));
	Verts2[1].Init(Vector2D(size.x + size.w + 4, size.y + 1));
	Verts2[2].Init(Vector2D(size.x + size.w, size.y + 5));
	Verts2[3].Init(Vector2D(size.x - 4, size.y + 5));

	CColor c = CColor(0, 255, 0, 200);

	//RENDER::Polygon(4, Verts2 + hp, c);*/

	Verts2[0].Init(Vector2D(size.x + 1, size.y + 1));
	Verts2[1].Init(Vector2D(size.x + size.w + 2, size.y + 1));
	Verts2[2].Init(Vector2D(size.x + size.w, size.y + 2));
	Verts2[3].Init(Vector2D(size.x - 2, size.y + 2));

	RENDER::Polygon(4, Verts2, CColor(255, 255, 255, 40));

}
void CVisuals::LagCompHitbox(SDK::CBaseEntity* entity, int index)
{
	float duration = SETTINGS::settings.lagcomptime;

	if (index < 0)
		return;

	if (!entity)
		return;

	SDK::studiohdr_t* pStudioModel = INTERFACES::ModelInfo->GetStudioModel(entity->GetModel());

	if (!pStudioModel)
		return;

	SDK::mstudiohitboxset_t* pHitboxSet = pStudioModel->GetHitboxSet(0);

	if (!pHitboxSet)
		return;

	for (int i = 0; i < pHitboxSet->numhitboxes; i++)
	{
		SDK::mstudiobbox_t* pHitbox = pHitboxSet->GetHitbox(i);

		if (!pHitbox)
			continue;

		auto bone_matrix = entity->GetBoneMatrix(pHitbox->bone);

		Vector vMin, vMax;

		MATH::VectorTransform(pHitbox->bbmin, bone_matrix, vMin);
		MATH::VectorTransform(pHitbox->bbmax, bone_matrix, vMax);

		if (pHitbox->radius > -1)
		{
			float lagcomp_colour[3] = { SETTINGS::settings.lagcompcolour.RGBA[0], SETTINGS::settings.lagcompcolour.RGBA[1], SETTINGS::settings.lagcompcolour.RGBA[2] };
			INTERFACES::DebugOverlay->AddCapsuleOverlay(vMin, vMax, pHitbox->radius, lagcomp_colour[0], lagcomp_colour[1], lagcomp_colour[2], SETTINGS::settings.lagcompalpha, duration); // color green 
		}
	}
}
void CVisuals::DrawDropped(SDK::CBaseEntity* entity)
{
	Vector min, max;
	entity->GetRenderBounds(min, max);
	Vector pos, pos3D, top, top3D;
	pos3D = entity->GetAbsOrigin() - Vector(0, 0, 10);
	top3D = pos3D + Vector(0, 0, max.z + 10);

	SDK::CBaseWeapon* weapon_cast = (SDK::CBaseWeapon*)entity;

	if (!weapon_cast)
		return;

	auto weapon = INTERFACES::ClientEntityList->GetClientEntity(entity->GetActiveWeaponIndex());
	auto c_baseweapon = reinterpret_cast<SDK::CBaseWeapon*>(INTERFACES::ClientEntityList->GetClientEntity(entity->GetActiveWeaponIndex()));

	if (!c_baseweapon)
		return;

	if (!weapon)
		return;

	SDK::CBaseEntity* plr = INTERFACES::ClientEntityList->GetClientEntityFromHandle((HANDLE)weapon_cast->GetOwnerHandle());
	if (!plr && RENDER::WorldToScreen(pos3D, pos) && RENDER::WorldToScreen(top3D, top))
	{
		std::string ItemName = fix_item_name(weapon->GetClientClass()->m_pNetworkName);
		int height = (pos.y - top.y);
		int width = height / 2;
		RENDER::DrawF(pos.x, pos.y, FONTS::visuals_esp_font, true, true, WHITE, ItemName.c_str()); //numpad_menu_font
	}
}
void CVisuals::DrawInfo(SDK::CBaseEntity* entity, CColor color, CColor alt, Vector pos, Vector top)
{
	std::vector<std::pair<std::string, CColor>> stored_info;

	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!local_player) return;

	bool is_local_player = entity == local_player;
	bool is_teammate = local_player->GetTeam() == entity->GetTeam() && !is_local_player;
	int screen_width, screen_height;
	float breaking_lby_fraction = fabs(MATH::NormalizeYaw(GLOBAL::real_angles.y - local_player->GetLowerBodyYaw())) / 180.f;
	INTERFACES::Engine->GetScreenSize(screen_width, screen_height);
	/*if (SETTINGS::settings.antiaw)
	{
		if (missed > miss)
		{
			RENDER::DrawF(10, screen_height / 2, FONTS::numpad_menu_font, false, false, CColor((1.f - breaking_lby_fraction) * 255.f, breaking_lby_fraction * 255.f, 0), "BAIM");
		}

	}*/
	if (entity->GetHealth() > 0)
		stored_info.push_back(std::pair<std::string, CColor>("hp: " + std::to_string(entity->GetHealth()), backtrack));
	auto weapon = INTERFACES::ClientEntityList->GetClientEntity(entity->GetActiveWeaponIndex());
	auto c_baseweapon = reinterpret_cast<SDK::CBaseWeapon*>(INTERFACES::ClientEntityList->GetClientEntity(entity->GetActiveWeaponIndex()));

	if (!weapon)
		return;

	if (!c_baseweapon)
		return;

	stored_info.push_back(std::pair<std::string, CColor>(fix_item_name(weapon->GetClientClass()->m_pNetworkName), backtrack));
	stored_info.push_back(std::pair<std::string, CColor>("ammo: " + std::to_string(c_baseweapon->GetLoadedAmmo()), backtrack));
	if (SETTINGS::settings.velocity)
	{
		if (!is_teammate)
		{
			if (local_player->IsAlive())
			{
				int alpha[65];
				if (entity->GetVelocity().Length2D() > 36)
					stored_info.push_back(std::pair<std::string, CColor>("velocity: " + std::to_string(entity->GetVelocity().Length2D()), CColor(0, 255, 0, alpha[entity->GetIndex()])));
				else if (entity->GetVelocity().Length2D() < 36 && entity->GetVelocity().Length2D() > 0.1)
					stored_info.push_back(std::pair<std::string, CColor>("velocity: " + std::to_string(entity->GetVelocity().Length2D()), CColor(255, 255, 0, alpha[entity->GetIndex()])));
				else
					stored_info.push_back(std::pair<std::string, CColor>("velocity: " + std::to_string(entity->GetVelocity().Length2D()), CColor(255, 0, 0, alpha[entity->GetIndex()])));
			}
		}
	}
	if (SETTINGS::settings.money_bool && !is_teammate)
		stored_info.push_back(std::pair<std::string, CColor>("$" + std::to_string(entity->GetMoney()), backtrack));
	else if (SETTINGS::settings.moneyteam && is_teammate)
		stored_info.push_back(std::pair<std::string, CColor>("$" + std::to_string(entity->GetMoney()), backtrack));
	if (SETTINGS::settings.smart_lag && local_player->IsAlive())
	{
		if (!(local_player->GetFlags() & FL_ONGROUND))
		{
			RENDER::DrawF(10, screen_height / 1.5, FONTS::visuals_lby_font, 0, 0, GREEN, std::to_string(SETTINGS::settings.jump_lag));
		}
		else if (local_player->GetVelocity().Length2D() > 0.1)
		{
			RENDER::DrawF(10, screen_height / 1.5, FONTS::visuals_lby_font, 0, 0, GREEN, std::to_string(SETTINGS::settings.move_lag));
		}
	}
	//	std::vector<std::pair<std::string, CColor>> stored_info;
	if (SETTINGS::settings.info_bool2)
	{

	}
	if (SETTINGS::settings.info_bool)
	{
		if (!is_teammate)
		{
			if (local_player->IsAlive())
			{
				if (local_player->GetHealth() == 0)
					stored_info.push_back(std::pair<std::string, CColor>("not resolving", alt));
				else if (resolve_type[entity->GetIndex()] == 1)
					stored_info.push_back(std::pair<std::string, CColor>("desync", color));
				else if (resolve_type[entity->GetIndex()] == 2)
					stored_info.push_back(std::pair<std::string, CColor>("desync + baim", alt));
				else if (resolve_type[entity->GetIndex()] == 3)
					stored_info.push_back(std::pair<std::string, CColor>("nospread", color));
				else if (resolve_type[entity->GetIndex()] == 4)
					stored_info.push_back(std::pair<std::string, CColor>("bruteforce", color));
				else if (resolve_type[entity->GetIndex()] == 5)
					stored_info.push_back(std::pair<std::string, CColor>("move desync", alt));
				else if (resolve_type[entity->GetIndex()] == 6)
					stored_info.push_back(std::pair<std::string, CColor>("breaking desync", color));
				else if (resolve_type[entity->GetIndex()] == 7)
					stored_info.push_back(std::pair<std::string, CColor>("backtrack + desync", color));
			}
		}
	}
	if (SETTINGS::settings.info_bool && !is_teammate)
	{
		if (entity->GetArmor() > 0)
			stored_info.push_back(std::pair<std::string, CColor>(entity->GetArmorName(), color));

		if (entity->GetIsScoped())
			stored_info.push_back(std::pair<std::string, CColor>("zoom", alt));
		if (MATH::NormalizeYaw(entity->GetEyeAnglesPointer()->y - entity->GetLowerBodyYaw()))
			stored_info.push_back(std::pair<std::string, CColor>("FAKE", color));
	}
	else if (SETTINGS::settings.flagsteam && is_teammate)
	{
		if (entity->GetArmor() > 0)
			stored_info.push_back(std::pair<std::string, CColor>(entity->GetArmorName(), color));

		if (entity->GetIsScoped())
			stored_info.push_back(std::pair<std::string, CColor>("zoom", alt));
		if (MATH::NormalizeYaw(entity->GetEyeAnglesPointer()->y - entity->GetLowerBodyYaw()))
			stored_info.push_back(std::pair<std::string, CColor>("FAKE", color));
	}

	int height = (pos.y - top.y), width = height / 2, i = 0;
	for (auto Text : stored_info)
	{
		RENDER::DrawF((pos.x + width / 2) + 5, top.y + i, FONTS::visuals_esp_font, false, false, Text.second, Text.first);
		i += 8;
	}
}
void CVisuals::DrawInaccuracy()
{
	/*auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!local_player) return;

	auto weapon = reinterpret_cast<SDK::CBaseWeapon*>(INTERFACES::ClientEntityList->GetClientEntity(local_player->GetActiveWeaponIndex()));
	if (!weapon) return;

	int W, H, cW, cH;
	INTERFACES::Engine->GetScreenSize(W, H);
	cW = W / 2; cH = H / 2;
	if (local_player->IsAlive())
	{
		auto accuracy = (weapon->GetInaccuracy() + weapon->GetSpreadCone()) * 500.f;
		if (!weapon->is_grenade() && !weapon->is_knife())
			RENDER::DrawFilledCircle(cW, cH, accuracy + 3, 30, CColor(0, 0, 0, 85));
	}*/
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!local_player) return;

	auto weapon = reinterpret_cast<SDK::CBaseWeapon*>(INTERFACES::ClientEntityList->GetClientEntity(local_player->GetActiveWeaponIndex()));
	if (!weapon) return;

	int W, H, cW, cH;
	INTERFACES::Engine->GetScreenSize(W, H);
	cW = W / 2; cH = H / 2;
	if (local_player->IsAlive())
	{
		auto accuracy = (weapon->GetInaccuracy() + weapon->GetSpreadCone()) * 500.f;
		if (!weapon->is_grenade() && !weapon->is_knife())
		{
			//float spreadcolur[3] = { SETTINGS::settings.spread_colour.RGBA[0], SETTINGS::settings.spread_colour.RGBA[1], SETTINGS::settings.spread_colour.RGBA[2] };
			//CColor pColor = CColor(spreadcolur[0], spreadcolur[1], spreadcolur[2], 85);
			RENDER::DrawFilledCircle(cW, cH, accuracy + 3, 30, CColor(0, 0, 0, 85));
		}
	}
}

void CVisuals::DrawBulletBeams()
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!local_player) return;

	if (!INTERFACES::Engine->IsInGame() || !local_player) { Impacts.clear(); return; }
	if (Impacts.size() > 30) Impacts.pop_back();

	for (int i = 0; i < Impacts.size(); i++)
	{
		auto current = Impacts.at(i);
		if (!current.pPlayer) continue;
		if (current.pPlayer->GetIsDormant()) continue;

		bool is_local_player = current.pPlayer == local_player;
		bool is_teammate = local_player->GetTeam() == current.pPlayer->GetTeam() && !is_local_player;

		if (current.pPlayer == local_player)
			current.color = CColor(SETTINGS::settings.bulletlocal_col[0] * 255, SETTINGS::settings.bulletlocal_col[1] * 255, SETTINGS::settings.bulletlocal_col[2] * 255, SETTINGS::settings.bulletlocal_col[3] * 255);
		else if (current.pPlayer != local_player && !is_teammate)
			current.color = CColor(SETTINGS::settings.bulletenemy_col[0] * 255, SETTINGS::settings.bulletenemy_col[1] * 255, SETTINGS::settings.bulletenemy_col[2] * 255, SETTINGS::settings.bulletenemy_col[3] * 255);
		else if (current.pPlayer != local_player && is_teammate)
			current.color = CColor(SETTINGS::settings.bulletteam_col[0] * 255, SETTINGS::settings.bulletteam_col[1] * 255, SETTINGS::settings.bulletteam_col[2] * 255, SETTINGS::settings.bulletteam_col[3] * 255);

		SDK::BeamInfo_t beamInfo;
		beamInfo.m_nType = SDK::TE_BEAMPOINTS;
		beamInfo.m_pszModelName = "sprites/purplelaser1.vmt";
		beamInfo.m_nModelIndex = -1;
		beamInfo.m_flHaloScale = 0.0f;
		beamInfo.m_flLife = SETTINGS::settings.bulletlife;
		beamInfo.m_flWidth = SETTINGS::settings.bulletsize;
		beamInfo.m_flEndWidth = 2.0f;
		beamInfo.m_flFadeLength = 0.0f;
		beamInfo.m_flAmplitude = 2.0f;
		beamInfo.m_flBrightness = 255.f;
		beamInfo.m_flSpeed = 0.2f;
		beamInfo.m_nStartFrame = 0;
		beamInfo.m_flFrameRate = 0.f;
		beamInfo.m_flRed = current.color.RGBA[0];
		beamInfo.m_flGreen = current.color.RGBA[1];
		beamInfo.m_flBlue = current.color.RGBA[2];
		beamInfo.m_nSegments = 2;
		beamInfo.m_bRenderable = true;
		beamInfo.m_nFlags = SDK::FBEAM_ONLYNOISEONCE | SDK::FBEAM_NOTILE | SDK::FBEAM_HALOBEAM;

		beamInfo.m_vecStart = current.pPlayer->GetVecOrigin() + current.pPlayer->GetViewOffset();
		beamInfo.m_vecEnd = current.vecImpactPos;

		auto beam = INTERFACES::ViewRenderBeams->CreateBeamPoints(beamInfo);
		if (beam) INTERFACES::ViewRenderBeams->DrawBeam(beam);

		Impacts.erase(Impacts.begin() + i);
	}
}

void CVisuals::DrawCrosshair()
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!local_player) return;

	auto crosshair = INTERFACES::cvar->FindVar("crosshair");
	if (SETTINGS::settings.xhair_type == 0)
	{
		crosshair->SetValue("1");
		return;
	}
	else
		crosshair->SetValue("0");

	int W, H, cW, cH;
	INTERFACES::Engine->GetScreenSize(W, H);

	cW = W / 2; cH = H / 2;

	int dX = W / 120.f, dY = H / 120.f;
	int drX, drY;

	if (SETTINGS::settings.xhair_type == 2)
	{
		drX = cW - (int)(dX * (((local_player->GetPunchAngles().y * 2.f) * 0.45f) + local_player->GetPunchAngles().y));
		drY = cH + (int)(dY * (((local_player->GetPunchAngles().x * 2.f) * 0.45f) + local_player->GetPunchAngles().x));
	}
	else
	{
		drX = cW;
		drY = cH;
	}

	INTERFACES::Surface->DrawSetColor(BLACK);
	INTERFACES::Surface->DrawFilledRect(drX - 4, drY - 2, drX - 4 + 8, drY - 2 + 4);
	INTERFACES::Surface->DrawFilledRect(drX - 2, drY - 4, drX - 2 + 4, drY - 4 + 8);

	INTERFACES::Surface->DrawSetColor(WHITE);
	INTERFACES::Surface->DrawFilledRect(drX - 3, drY - 1, drX - 3 + 6, drY - 1 + 2);
	INTERFACES::Surface->DrawFilledRect(drX - 1, drY - 3, drX - 1 + 2, drY - 3 + 6);
}

void CVisuals::DrawFovArrows(SDK::CBaseEntity* entity, CColor color)
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!local_player) return;
	if (entity->GetIsDormant()) return;

	Vector screenPos, client_viewangles;
	int screen_width = 0, screen_height = 0;
	float radius = 300.f;

	if (UTILS::IsOnScreen(aimbot->get_hitbox_pos(entity, SDK::HitboxList::HITBOX_HEAD), screenPos)) return;

	INTERFACES::Engine->GetViewAngles(client_viewangles);
	INTERFACES::Engine->GetScreenSize(screen_width, screen_height);

	const auto screen_center = Vector(screen_width / 2.f, screen_height / 2.f, 0);
	const auto rot = DEG2RAD(client_viewangles.y - UTILS::CalcAngle(local_player->GetEyePosition(), aimbot->get_hitbox_pos(entity, SDK::HitboxList::HITBOX_HEAD)).y - 90);

	std::vector<SDK::Vertex_t> vertices;
	vertices.push_back(SDK::Vertex_t(Vector2D(screen_center.x + cosf(rot) * radius, screen_center.y + sinf(rot) * radius)));
	vertices.push_back(SDK::Vertex_t(Vector2D(screen_center.x + cosf(rot + DEG2RAD(2)) * (radius - 16), screen_center.y + sinf(rot + DEG2RAD(2)) * (radius - 16))));
	vertices.push_back(SDK::Vertex_t(Vector2D(screen_center.x + cosf(rot - DEG2RAD(2)) * (radius - 16), screen_center.y + sinf(rot - DEG2RAD(2)) * (radius - 16))));

	RENDER::TexturedPolygon(3, vertices, color);
}
void CVisuals::DrawIndicator()
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!local_player) return;
	if (local_player->GetHealth() <= 0) return;
	auto weapon = reinterpret_cast<SDK::CBaseWeapon*>(INTERFACES::ClientEntityList->GetClientEntity(local_player->GetActiveWeaponIndex()));
	if (!weapon) return;
	float breaking_lby_fraction = fabs(MATH::NormalizeYaw(GLOBAL::real_angles.y - local_player->GetLowerBodyYaw())) / 180.f;
	float lby_delta = abs(MATH::NormalizeYaw(GLOBAL::real_angles.y - local_player->GetLowerBodyYaw()));

	int screen_width, screen_height;
	INTERFACES::Engine->GetScreenSize(screen_width, screen_height);

	int iY = 88;

	static double rainbow;
	rainbow += 0.00001;
	if (rainbow > 1.f)
		rainbow = 0;

	if (SETTINGS::settings.overrideenable)
	{
		iY += 22; bool overridekeyenabled;
		if (SETTINGS::settings.overridemethod == 0)
			RENDER::DrawF(10, screen_height - iY, FONTS::visuals_lby_font, false, false, SETTINGS::settings.overridething ? CColor(0, 255, 0) : CColor(255, 0, 0), "OVERRIDE");
		else if (SETTINGS::settings.overridemethod == 1)
		{
			GetAsyncKeyState(UTILS::INPUT::input_handler.keyBindings(SETTINGS::settings.overridekey)) ?
				RENDER::DrawF(10, screen_height - iY, FONTS::visuals_lby_font, false, false, CColor(0, 255, 0), "OVERRIDE") :
				RENDER::DrawF(10, screen_height - iY, FONTS::visuals_lby_font, false, false, CColor(255, 0, 0), "OVERRIDE");
		}
	}
	if (SETTINGS::settings.multi_bool)
	{
		RENDER::DrawF(15, screen_height / 2, FONTS::numpad_menu_font, false, false, CColor::FromHSB(rainbow, 1.f, 1.f), "Multipoint (crash)");
	}
	else
	{
		RENDER::DrawF(15, screen_height / 2, FONTS::numpad_menu_font, false, false, CColor(GREEN), "BASIC Point");
	}
	if (SETTINGS::settings.baim)
	{
		if (GetAsyncKeyState(UTILS::INPUT::input_handler.keyBindings(SETTINGS::settings.baimkey)))
		{
			RENDER::DrawF(10, screen_height / 2, FONTS::numpad_menu_font, false, false, CColor((1.f - breaking_lby_fraction) * 255.f, breaking_lby_fraction * 255.f, 0), "BAIM");
			//RENDER::DrawF(10, screen_height / 2, FONTS::visuals_lby_font, false, false, CColor(255, 0, 0), "BAIM");
		}
		if (SETTINGS::settings.baimfakewalk)
		{
			if (GetAsyncKeyState(VK_SHIFT))
			{
				RENDER::DrawF(10, screen_height / 2, FONTS::numpad_menu_font, false, false, CColor((1.f - breaking_lby_fraction) * 255.f, breaking_lby_fraction * 255.f, 0), "BAIM");
			}
		}
		if (weapon->GetItemDefenitionIndex() == SDK::ItemDefinitionIndex::WEAPON_TASER)
		{
			RENDER::DrawF(10, screen_height / 2, FONTS::numpad_menu_font, false, false, CColor::FromHSB(rainbow, 1.f, 1.f), "TASER");
			SETTINGS::settings.fakefix_bool = false;
			SETTINGS::settings.delay_shot = 0;

			//RENDER::DrawF(10, screen_height / 2, FONTS::visuals_lby_font, false, false, CColor(255, 0, 0), "BAIM");
		}
		else
		{
			SETTINGS::settings.fakefix_bool = true;
			SETTINGS::settings.delay_shot = 1;
		}
	}
	if (SETTINGS::settings.fakewalk)
	{
		if (GetAsyncKeyState(VK_SHIFT))
		{

			RENDER::DrawF(10, screen_height - iY, FONTS::visuals_lby_font, false, false, CColor((1.f - breaking_lby_fraction) * 255.f, breaking_lby_fraction * 255.f, 0), "DESYNC");


		}
	}
	if (SETTINGS::settings.aa_bool)
	{
		/*iY += 22;
		RENDER::DrawF(10, screen_height - iY, FONTS::visuals_lby_font, false, false, CColor((1.f - breaking_lby_fraction) * 255.f, breaking_lby_fraction * 255.f, 0), "LBY");
		int w1, h1;
		INTERFACES::Engine->GetScreenSize(w1, h1);
		auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
		auto nci = INTERFACES::Engine->GetNetChannelInfo();

		float ping = 1.f / nci->GetLatency(FLOW_INCOMING);
		char *PING;
		float fps = 1.f / INTERFACES::Globals->absoluteframetime;
		char *FPS;
		int radix = 10;
		char buffer[20];
		PING = _itoa(ping, buffer, radix);
		FPS = _itoa(fps, buffer, radix);
		RENDER::DrawF(50, h1 / 2 - 80, FONTS::menu_window_font, false, false, WHITE, PING);
		RENDER::DrawF(10, h1 / 2 - 60, FONTS::menu_window_font, false, false, WHITE, "FPS:");
		RENDER::DrawF(50, h1 / 2 - 60, FONTS::menu_window_font, false, false, WHITE, FPS);
		RENDER::DrawF(10, h1 / 2 - 80, FONTS::menu_window_font, false, false, WHITE, "Ping:");*/
		int w1, h1;
		INTERFACES::Engine->GetScreenSize(w1, h1);
		auto nci = INTERFACES::Engine->GetNetChannelInfo();
		std::string ping = std::to_string((int)(nci->GetAvgLatency(FLOW_INCOMING) + nci->GetAvgLatency(FLOW_OUTGOING) * 1000)) + " ms";
		//g_Render.String(8, 510, CD3DFONT_DROPSHADOW, Color(250, 150, 200, 180), g_Fonts.pFontTahoma10.get(), ("PING: " + ping).c_str());
		RENDER::DrawF(10, h1 / 2 - 80, FONTS::menu_window_font, false, false, CColor(250, 150, 200, 180), "Ping:" + ping);
		std::string fps = std::to_string(static_cast<int>(1.f / INTERFACES::Globals->absoluteframetime));
		RENDER::DrawF(10, h1 / 2 - 60, FONTS::menu_window_font, false, false, CColor(250, 150, 200, 180), "FPS:" + fps);
		//g_Render.String(8, 520, CD3DFONT_DROPSHADOW, Color(250, 150, 200, 180), g_Fonts.pFontTahoma10.get(), ("FPS: " + fps).c_str());
	}
	if (SETTINGS::settings.rifk_arrow)
	{
		auto client_viewangles = Vector();
		INTERFACES::Engine->GetViewAngles(client_viewangles);
		const auto screen_center = Vector2D(screen_width / 2.f, screen_height / 2.f);

		constexpr auto radius = 80.f;
		auto draw_arrow = [&](float rot, CColor color) -> void
		{
			std::vector<SDK::Vertex_t> vertices;
			vertices.push_back(SDK::Vertex_t(Vector2D(screen_center.x + cosf(rot) * radius, screen_center.y + sinf(rot) * radius)));
			vertices.push_back(SDK::Vertex_t(Vector2D(screen_center.x + cosf(rot + DEG2RAD(8)) * (radius - 12), screen_center.y + sinf(rot + DEG2RAD(8)) * (radius - 12)))); //25
			vertices.push_back(SDK::Vertex_t(Vector2D(screen_center.x + cosf(rot - DEG2RAD(8)) * (radius - 12), screen_center.y + sinf(rot - DEG2RAD(8)) * (radius - 12)))); //25
			RENDER::TexturedPolygon(3, vertices, color);
		};

		static auto alpha = 0.f; static auto plus_or_minus = false;
		if (alpha <= 0.f || alpha >= 255.f) plus_or_minus = !plus_or_minus;
		alpha += plus_or_minus ? (255.f / 7 * 0.015) : -(255.f / 7 * 0.015); alpha = clamp(alpha, 0.f, 255.f);

		auto fake_color = CColor(RED);
		const auto fake_rot = DEG2RAD(client_viewangles.y - GLOBAL::fake_angles.y - 90);
		draw_arrow(fake_rot, fake_color);

		static double rainbow;
		rainbow += 0.00001;
		if (rainbow > 1.f)
			rainbow = 0;

		auto real_color = CColor(GREEN);
		const auto real_rot = DEG2RAD(client_viewangles.y - GLOBAL::real_angles.y - 90);
		draw_arrow(real_rot, real_color);
	}
}
void CVisuals::LogEvents()
{
	static bool convar_performed = false, convar_lastsetting;

	if (convar_lastsetting != SETTINGS::settings.info_bool)
	{
		convar_lastsetting = SETTINGS::settings.info_bool;
		convar_performed = false;
	}

	if (!convar_performed)
	{
		static auto developer = INTERFACES::cvar->FindVar("developer");
		developer->SetValue(1);
		static auto con_filter_text_out = INTERFACES::cvar->FindVar("con_filter_text_out");
		static auto con_filter_enable = INTERFACES::cvar->FindVar("con_filter_enable");
		static auto con_filter_text = INTERFACES::cvar->FindVar("con_filter_text");

		con_filter_text->SetValue(".     ");
		con_filter_text_out->SetValue("");
		con_filter_enable->SetValue(2);
		convar_performed = true;
	}
}
void CVisuals::AntiAimLines()
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!local_player)
		return;
	if (local_player->IsAlive())
	{
		Vector src3D, dst3D, forward, src, dst;
		SDK::trace_t tr;
		SDK::Ray_t ray;
		SDK::CTraceFilter filter;


		filter.pSkip1 = local_player;

		MATH::AngleVectors(Vector(0, local_player->GetLowerBodyYaw(), 0), &forward);
		src3D = local_player->GetVecOrigin();
		dst3D = src3D + (forward * 42.f); //replace 50 with the length you want the line to have 

		ray.Init(src3D, dst3D);

		INTERFACES::Trace->TraceRay(ray, 0, &filter, &tr);

		if (!RENDER::WorldToScreen(src3D, src) || !RENDER::WorldToScreen(tr.end, dst))
			return;

		RENDER::DrawLine(src.x, src.y, dst.x, dst.y, CColor(0, 0, 255, 255));
		//RENDER::DrawF(src.x, src.y, dst.x, dst.y, FONTS::visuals_lby_font, CColor(0, 0, 255, 255), "Real"); ���� �� ����

		MATH::AngleVectors(Vector(0, GLOBAL::fake_angles.y, 0), &forward);
		dst3D = src3D + (forward * 42.f); //replace 50 with the length you want the line to have 

		ray.Init(src3D, dst3D);

		INTERFACES::Trace->TraceRay(ray, 0, &filter, &tr);

		if (!RENDER::WorldToScreen(src3D, src) || !RENDER::WorldToScreen(tr.end, dst))
			return;

		RENDER::DrawLine(src.x, src.y, dst.x, dst.y, CColor(255, 0, 0, 255));
		//RENDER::DrawF(src.x, src.y, dst.x, dst.y, FONTS::visuals_lby_font, CColor(0, 0, 255, 255), "Fake"); ����������(
	}
}

void CVisuals::DrawHitmarker()
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());

	if (!local_player)
		return;

	if (local_player->GetHealth() <= 0)
		return;

	static int lineSize = 6;

	static float alpha = 0;
	float step = 255.f / 0.3f * INTERFACES::Globals->frametime;


	if (GLOBAL::flHurtTime + 0.4f >= INTERFACES::Globals->curtime)
		alpha = 255.f;
	else
		alpha -= step;

	if (alpha > 0) {
		int screenSizeX, screenCenterX;
		int screenSizeY, screenCenterY;
		INTERFACES::Engine->GetScreenSize(screenSizeX, screenSizeY);

		screenCenterX = screenSizeX / 2;
		screenCenterY = screenSizeY / 2;
		CColor col = CColor(255, 255, 255, alpha);
		RENDER::DrawLine(screenCenterX - lineSize * 2, screenCenterY - lineSize * 2, screenCenterX - (lineSize), screenCenterY - (lineSize), col);
		RENDER::DrawLine(screenCenterX - lineSize * 2, screenCenterY + lineSize * 2, screenCenterX - (lineSize), screenCenterY + (lineSize), col);
		RENDER::DrawLine(screenCenterX + lineSize * 2, screenCenterY + lineSize * 2, screenCenterX + (lineSize), screenCenterY + (lineSize), col);
		RENDER::DrawLine(screenCenterX + lineSize * 2, screenCenterY - lineSize * 2, screenCenterX + (lineSize), screenCenterY - (lineSize), col);
	}
}

void CVisuals::DrawBorderLines()
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!local_player) return;

	auto weapon = INTERFACES::ClientEntityList->GetClientEntity(local_player->GetActiveWeaponIndex());
	if (!weapon) return;

	int screen_x;
	int screen_y;
	int center_x;
	int center_y;
	INTERFACES::Engine->GetScreenSize(screen_x, screen_y);
	INTERFACES::Engine->GetScreenSize(center_x, center_y);
	center_x /= 2; center_y /= 2;

	if (local_player->GetIsScoped())
	{
		RENDER::DrawLine(0, center_y, screen_x, center_y, CColor(0, 0, 0, 255));
		RENDER::DrawLine(center_x, 0, center_x, screen_y, CColor(0, 0, 0, 255));
	}
}

const char* NoNamersAnim[27] =
{
	"                  ",
	"                 P",
	"                PHA",
	"               PHAC",
	"              PHACK",
	"             PHACK R",
	"            PHACK RE",
	"           PHACK REC",
	"          PHACK RECO",
	"         PHACK RECOD",
	"        PHACK RECODE ",
	"       PHACK Re$ode  ",
	"      $PHACK RECODE$ ",
	"     PHACK RECODE    ",
	"    PHACK RECODE    ",
	"   PHACK RECODE      ",
	"  PHACK RECODE       ",
	" PHACK RECODE        ",
	"PHACK RECODE         ",
	"PHACK RECOD          ",
	"PHACK RECO           ",
	"PHACK REC            ",
	"PHACK RE             ",
	"PHACK R              ",
	"PHACK               ",
	"PHA                ",
	"P                 "
};
const char* nnanim[27] =
{
	"                  ",
	"                ga",
	"               gam",
	"              game",
	"             games",
	"            gamese",
	"           gamesen",
	"          gamesens",
	"         gamesense",
	"        gamesense ",
	"       gamesense  ",
	"      gamesense   ",
	"     gamesense    ",
	"    gamesense     ",
	"   gamesense      ",
	"  gamesense       ",
	" gamesense        ",
	"gamesense         ",
	"amesense          ",
	"mesense           ",
	"esense            ",
	"sense             ",
	"ense              ",
	"nse               ",
	"se                ",
	"e                 "
	"                  "
};
const char* nnanim1[27] =
{
	"                  ",
	"                 f",
	"                fa",
	"               fat",
	"              fata",
	"             fatal",
	"            fatali",
	"           fatalit",
	"          fatality",
	"         fatality.",
	"       fatality.w ",
	"      fatality.wi ",
	"     fatality.win ",
	"    fatality.win  ",
	"   fatality.win   ",
	"  fatality.win    ",
	" fatality.win     ",
	"fatality.win      ",
	"atality.win       ",
	"tality.win        ",
	"ality.win         ",
	"lity.win          ",
	"ity.win           ",
	"ty.win            ",
	"y.win             ",
	".win              ",
	"win               "
	"in                "
	"n                 "
	"                  "
};
const char* NoNamersAnim2[27] =
{
	"                  ",
	"                 m",
	"                mo",
	"               mon",
	"              mone",
	"             money",
	"            moneyb",
	"           moneybo",
	"          moneybot",
	"         moneybot  ",
	"        moneybot   ",
	"       moneybot    ",
	"      moneybot     ",
	"     moneybot      ",
	"    moneybot       ",
	"   moneybot        ",
	"  moneybot         ",
	" moneybot          ",
	"moneybot           ",
	"oneybot            ",
	"neybot             ",
	"eybot              ",
	"ybot               ",
	"bot                ",
	"ot                 ",
	"t                  ",
	"                   "
};
void setClanTag(const char* tag, const char* name)
{
	static auto pSetClanTag = reinterpret_cast<void(__fastcall*)(const char*, const char*)>(((DWORD)UTILS::FindPattern("engine.dll", (PBYTE)"\x53\x56\x57\x8B\xDA\x8B\xF9\xFF\x15\x00\x00\x00\x00\x6A\x24\x8B\xC8\x8B\x30", "xxxxxxxxx????xxxxxx")));
	pSetClanTag(tag, name);
}
static int counter = 0;
int kek = 0;
int stc = 0;
int autism = 0;

void CVisuals::phack3()
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!local_player) return;

	static size_t lastTime = 0;
	if (GetTickCount() > lastTime)
	{
		kek++;
		if (kek > 10) {
			autism++; if (autism > 18) autism = 0;
			setClanTag(NoNamersAnim[autism], "PHACK-V2");
			lastTime = GetTickCount() + 500;
		}

		if (kek > 11) kek = 0;
	}
}
void CVisuals::fatality()
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!local_player) return;

	static size_t lastTime = 0;
	if (GetTickCount() > lastTime)
	{
		kek++;
		if (kek > 10) {
			autism++; if (autism > 18) autism = 0;
			setClanTag(nnanim1[autism], "fatalitybeta");
			lastTime = GetTickCount() + 500;
		}

		if (kek > 11) kek = 0;
	}
}
void CVisuals::skeet()
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!local_player) return;

	static size_t lastTime = 0;
	if (GetTickCount() > lastTime)
	{
		kek++;
		if (kek > 10) {
			autism++; if (autism > 20) autism = 0;
			setClanTag(nnanim[autism], "gamesense");
			lastTime = GetTickCount() + 500;
		}

		if (kek > 11) kek = 0;
	}
}
void CVisuals::moneybot()
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!local_player) return;

	static size_t lastTime = 0;
	if (GetTickCount() > lastTime)
	{
		kek++;
		if (kek > 10) {
			autism++; if (autism > 18) autism = 0;
			setClanTag(NoNamersAnim2[autism], "moneybot");
			lastTime = GetTickCount() + 500;
		}

		if (kek > 11) kek = 0;
	}
}
void CVisuals::phack2()
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!local_player) return;
	static size_t lastTime = 0;
	int server_time = local_player->GetTickBase() * INTERFACES::Globals->interval_per_tick;
	if (GetTickCount() > lastTime)
	{
		stc++;
		if (stc > 10) {
			int value = server_time % 15;
			switch (value)
			{
			case 0:setClanTag("P", "phack"); break;
			case 1:setClanTag("PH", "phack"); break;
			case 2:setClanTag("PHA", "phack"); break;
			case 3:setClanTag("PHAC", "phack"); break;
			case 4:setClanTag("PHACK", "phack"); break;
			case 5:setClanTag("PHACK S", "phack"); break;
			case 6:setClanTag("PHACK ST", "phack"); break;
			case 7:setClanTag("PHACK STY", "phack"); break;
			case 8:setClanTag("PHACK STYL", "phack"); break;
			case 9:setClanTag("PHACK STYLE", "phack"); break;
			case 10:setClanTag("PHACK STYLE>>", "phack"); break;
			case 11:setClanTag("$$ $$ $$", "phack"); break;
			case 12:setClanTag("SERVERSIDE", "phack"); break;
			case 13:setClanTag("PHACK", "phack"); break;
			case 14:setClanTag("pHACK > all", "phack"); break;
			}
			lastTime = GetTickCount() + 500;
		}

		if (stc > 11) stc = 0;
	}
}
CVisuals* visuals = new CVisuals();