/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */


/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "menus.h"

#include <base/str.h>
#include <base/time.h>

#include <engine/client.h>
#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <engine/textrender.h>

#include <game/client/animstate.h>
#include <game/client/gameclient.h>
#include <game/client/ui.h>
#include <game/client/ui_scrollregion.h>
#include <game/localization.h>

#include <algorithm>
#include <cstring>
#include <iterator>

enum
{
	VEBURY_TAB_ADMIN_PANEL = 0,
	VEBURY_TAB_LOG,
	NUMBER_OF_VEBURY_TABS,
};

// Parses a line of the form produced by the server's "status" command. The line we
// actually receive over rcon is a full log line, i.e. it also contains a timestamp
// and the "server" system tag before the actual status content, e.g.:
//   2026-07-28 17:15:52 I server: id=0 addr=1.2.3.4:12345 name='Foo' client=19090 secure=yes flags=132
// Note: the server internally formats the address as "<{...}>" but these are just
// markers it uses to decide whether to hide IPs (see CServer::StrHideIps); by the time
// the line reaches us via rcon, the markers are already stripped and only the plain
// "addr=<ip>:<port>" (or "addr=XXX" if IPs are hidden) remains.
// Returns false if the line does not look like a status line.
static bool ParseVeburyStatusLine(const char *pLine, int *pClientId, char *pAddrBuf, size_t AddrBufSize, char *pVersionBuf, size_t VersionBufSize)
{
	// Search for "id=" anywhere in the line (it's not necessarily at the very start,
	// since the line may be prefixed with a timestamp and system tag).
	const char *pIdTag = str_find(pLine, "id=");
	if(!pIdTag)
		return false;

	*pClientId = str_toint(pIdTag + 3);
	if(*pClientId < 0 || *pClientId >= MAX_CLIENTS)
		return false;

	const char *pAddrTag = str_find(pIdTag, "addr=");
	if(!pAddrTag)
		return false;
	pAddrTag += 5; // skip "addr="

	const char *pAddrEnd = str_find(pAddrTag, " ");
	if(!pAddrEnd || pAddrEnd <= pAddrTag)
		return false;

	// Only search for "client=" after the name field, to avoid a player naming
	// themselves in a way that could spoof the parsed client version.
	const char *pNameTag = str_find(pAddrEnd, "name='");
	const char *pSearchStart = pNameTag ? pNameTag + 6 : pAddrEnd;
	const char *pNameEnd = pNameTag ? str_find(pSearchStart, "' client=") : nullptr;
	const char *pClientTag = str_find(pNameEnd ? pNameEnd + 1 : pSearchStart, "client=");
	if(!pClientTag)
		return false;

	const size_t AddrLen = std::min((size_t)(pAddrEnd - pAddrTag), AddrBufSize - 1);
	std::memcpy(pAddrBuf, pAddrTag, AddrLen);
	pAddrBuf[AddrLen] = '\0';

	const char *pVersionStart = pClientTag + 7; // skip "client="
	if(str_startswith(pVersionStart, "0.7:"))
		pVersionStart += 4;
	str_format(pVersionBuf, VersionBufSize, "%d", str_toint(pVersionStart));

	return true;
}

void CMenus::OnVeburyRconLine(const char *pLine)
{
	int ClientId;
	char aAddr[64];
	char aVersion[16];
	if(!ParseVeburyStatusLine(pLine, &ClientId, aAddr, sizeof(aAddr), aVersion, sizeof(aVersion)))
		return;

	SVeburyStatusInfo &Info = m_aVeburyStatusCache[ClientId];
	str_copy(Info.m_aIp, aAddr);
	str_copy(Info.m_aVersion, aVersion);
	Info.m_LastUpdated = Client()->GlobalTime();
}

void CMenus::VeburyRequestStatusIfDue()
{
	if(!Client()->RconAuthed())
		return;

	const float Now = Client()->GlobalTime();
	if(Now - m_VeburyLastStatusRequestTime < 3.0f)
		return;

	m_VeburyLastStatusRequestTime = Now;
	// Unmask IPs for our own rcon session, then request the player list.
	Client()->Rcon("show_ips 1");
	Client()->Rcon("status");
}

void CMenus::VeburyExecuteAction(EVeburyAction Action)
{
	if(!Client()->RconAuthed())
		return;

	// If a previous pending action somehow hasn't been resolved yet, flush it now
	// with whatever data is available before queueing a new one.
	if(m_VeburyPendingAction.m_Active)
		VeburyFlushPendingAction();

	m_VeburyPendingAction.m_vEntries.clear();
	for(int ClientId = 0; ClientId < MAX_CLIENTS; ClientId++)
	{
		if(!m_aVeburySelectedPlayers[ClientId])
			continue;

		m_aVeburySelectedPlayers[ClientId] = false;

		if(!GameClient()->m_Snap.m_apPlayerInfos[ClientId])
			continue;

		SVeburyPendingEntry Entry;
		Entry.m_ClientId = ClientId;
		str_copy(Entry.m_aName, GameClient()->m_aClients[ClientId].m_aName);
		m_VeburyPendingAction.m_vEntries.push_back(Entry);
	}

	if(m_VeburyPendingAction.m_vEntries.empty())
		return;

	m_VeburyPendingAction.m_Active = true;
	m_VeburyPendingAction.m_Action = Action;
	str_copy(m_VeburyPendingAction.m_aReason, m_VeburyReasonInput.GetString());
	int TimeValue = m_VeburyTimeInput.IsEmpty() ? 0 : str_toint(m_VeburyTimeInput.GetString());
	m_VeburyPendingAction.m_TimeValue = std::max(TimeValue, 0);
	m_VeburyPendingAction.m_QueuedTime = Client()->GlobalTime();

	// Refresh player info (IP/version) *before* actually applying the action, since
	// ban/kick immediately remove the player from the server's "status" output.
	Client()->Rcon("show_ips 1");
	Client()->Rcon("status");
}

void CMenus::VeburyUpdatePendingAction()
{
	if(!m_VeburyPendingAction.m_Active)
		return;

	// Resolve as soon as we have fresh data for every pending player, but don't
	// wait forever in case a player already disconnected before we could ask.
	bool AllResolved = true;
	for(const SVeburyPendingEntry &PendingEntry : m_VeburyPendingAction.m_vEntries)
	{
		if(m_aVeburyStatusCache[PendingEntry.m_ClientId].m_LastUpdated < m_VeburyPendingAction.m_QueuedTime)
		{
			AllResolved = false;
			break;
		}
	}

	const float Elapsed = Client()->GlobalTime() - m_VeburyPendingAction.m_QueuedTime;
	if(AllResolved || Elapsed > 0.6f)
		VeburyFlushPendingAction();
}

void CMenus::VeburyFlushPendingAction()
{
	if(!m_VeburyPendingAction.m_Active)
		return;

	const EVeburyAction Action = m_VeburyPendingAction.m_Action;
	const char *pReason = m_VeburyPendingAction.m_aReason;
	const int TimeValue = m_VeburyPendingAction.m_TimeValue;

	for(const SVeburyPendingEntry &PendingEntry : m_VeburyPendingAction.m_vEntries)
	{
		const int ClientId = PendingEntry.m_ClientId;

		SVeburyLogEntry Entry;
		str_timestamp_format(Entry.m_aTimestamp, sizeof(Entry.m_aTimestamp), TimestampFormat::TIME);
		str_copy(Entry.m_aName, PendingEntry.m_aName);
		str_copy(Entry.m_aReason, pReason);

		const SVeburyStatusInfo &StatusInfo = m_aVeburyStatusCache[ClientId];
		str_copy(Entry.m_aIp, StatusInfo.m_aIp[0] ? StatusInfo.m_aIp : "?");
		str_copy(Entry.m_aVersion, StatusInfo.m_aVersion[0] ? StatusInfo.m_aVersion : "?");

		char aCmd[512];
		switch(Action)
		{
		case EVeburyAction::BAN:
			str_copy(Entry.m_aAction, "Ban");
			if(TimeValue > 0)
				str_format(Entry.m_aDuration, sizeof(Entry.m_aDuration), "%d min", TimeValue);
			else
				str_copy(Entry.m_aDuration, "permanent");
			// ban <id> <minutes> <reason>  (0 minutes = permanent ban)
			str_format(aCmd, sizeof(aCmd), "ban %d %d %s", ClientId, TimeValue, pReason);
			break;
		case EVeburyAction::KICK:
			str_copy(Entry.m_aAction, "Kick");
			str_copy(Entry.m_aDuration, "-");
			// kick <id> <reason>  (kick has no time component)
			str_format(aCmd, sizeof(aCmd), "kick %d %s", ClientId, pReason);
			break;
		case EVeburyAction::MUTE:
		{
			// muteid <id> <seconds> <reason>  (needs a strictly positive duration)
			const int Seconds = std::max(TimeValue, 1);
			str_copy(Entry.m_aAction, "Mute");
			str_format(Entry.m_aDuration, sizeof(Entry.m_aDuration), "%d sec", Seconds);
			str_format(aCmd, sizeof(aCmd), "muteid %d %d %s", ClientId, Seconds, pReason);
			break;
		}
		}

		Client()->Rcon(aCmd);

		m_vVeburyLog.push_back(Entry);
		if(m_vVeburyLog.size() > 200)
			m_vVeburyLog.erase(m_vVeburyLog.begin());
	}

	m_VeburyPendingAction.m_Active = false;
	m_VeburyPendingAction.m_vEntries.clear();
}

void CMenus::RenderSettingsVebury(CUIRect MainView)
{
	VeburyRequestStatusIfDue();

	static int s_CurTab = VEBURY_TAB_ADMIN_PANEL;

	CUIRect TabBar, Button;
	MainView.HSplitTop(20.0f, &TabBar, &MainView);
	const float TabWidth = TabBar.w / (float)NUMBER_OF_VEBURY_TABS;
	static CButtonContainer s_aPageTabs[NUMBER_OF_VEBURY_TABS] = {};
	const char *apTabNames[NUMBER_OF_VEBURY_TABS] = {
		"Admin panel",
		"Log"};

	for(int Tab = 0; Tab < NUMBER_OF_VEBURY_TABS; ++Tab)
	{
		TabBar.VSplitLeft(TabWidth, &Button, &TabBar);
		const int Corners = Tab == 0 ? IGraphics::CORNER_L : (Tab == NUMBER_OF_VEBURY_TABS - 1 ? IGraphics::CORNER_R : IGraphics::CORNER_NONE);
		if(DoButton_MenuTab(&s_aPageTabs[Tab], apTabNames[Tab], s_CurTab == Tab, &Button, Corners, nullptr, nullptr, nullptr, nullptr, 4.0f))
			s_CurTab = Tab;
	}

	MainView.HSplitTop(10.0f, nullptr, &MainView);

	if(s_CurTab == VEBURY_TAB_ADMIN_PANEL)
		RenderVeburyAdminPanel(MainView);
	else
		RenderVeburyLog(MainView);
}

void CMenus::RenderVeburyAdminPanel(CUIRect MainView)
{
	if(!Client()->RconAuthed())
	{
		Ui()->DoLabel(&MainView, Localize("You need to authenticate with rcon (see the console) to use this panel."), 14.0f, TEXTALIGN_MC);
		return;
	}

	// Left: player list. Right: reason + time + action buttons.
	CUIRect PlayerListOuter, RightPanel;
	MainView.VSplitLeft(MainView.w * 0.55f, &PlayerListOuter, &RightPanel);
	RightPanel.VSplitLeft(16.0f, nullptr, &RightPanel);

	PlayerListOuter.Draw(ColorRGBA(0.0f, 0.0f, 0.0f, 0.15f), IGraphics::CORNER_ALL, 5.0f);
	CUIRect PlayerList;
	PlayerListOuter.Margin(4.0f, &PlayerList);

	static CScrollRegion s_ScrollRegion;
	CScrollRegionParams ScrollParams;
	ScrollParams.m_ScrollUnit = 3 * 28.0f;
	s_ScrollRegion.Begin(&PlayerList, &ScrollParams);

	for(const auto &pInfoByName : GameClient()->m_Snap.m_apInfoByName)
	{
		if(!pInfoByName)
			continue;

		const int ClientId = pInfoByName->m_ClientId;
		if(ClientId == GameClient()->m_Snap.m_LocalClientId)
			continue;

		CUIRect Row;
		PlayerList.HSplitTop(26.0f, &Row, &PlayerList);
		PlayerList.HSplitTop(2.0f, nullptr, &PlayerList);
		if(!s_ScrollRegion.AddRect(Row))
			continue;

		const bool Selected = m_aVeburySelectedPlayers[ClientId];
		if(Ui()->DoButtonLogic(&m_aVeburySelectedPlayers[ClientId], Selected, &Row, BUTTONFLAG_LEFT))
			m_aVeburySelectedPlayers[ClientId] = !Selected;

		const bool Hovered = Ui()->HotItem() == &m_aVeburySelectedPlayers[ClientId];
		Row.Draw(ColorRGBA(1.0f, 1.0f, 1.0f, Selected ? 0.25f : (Hovered ? 0.10f : 0.05f)), IGraphics::CORNER_ALL, 4.0f);

		CUIRect TeeRect, Label;
		Row.VSplitLeft(Row.h, &TeeRect, &Label);
		Label.VSplitLeft(4.0f, nullptr, &Label);

		CTeeRenderInfo TeeInfo = GameClient()->m_aClients[ClientId].m_RenderInfo;
		TeeInfo.m_Size = TeeRect.h;
		const CAnimState *pIdleState = CAnimState::GetIdle();
		vec2 OffsetToMid;
		CRenderTools::GetRenderTeeOffsetToRenderedTee(pIdleState, &TeeInfo, OffsetToMid);
		const vec2 TeeRenderPos(TeeRect.x + TeeInfo.m_Size / 2, TeeRect.y + TeeInfo.m_Size / 2 + OffsetToMid.y);
		RenderTools()->RenderTee(pIdleState, &TeeInfo, EMOTE_NORMAL, vec2(1.0f, 0.0f), TeeRenderPos);

		// Additional info: client version, filled in from "status" (refreshed periodically).
		const SVeburyStatusInfo &StatusInfo = m_aVeburyStatusCache[ClientId];
		char aRowLabel[MAX_NAME_LENGTH * 3 + 32];
		if(StatusInfo.m_aVersion[0])
			str_format(aRowLabel, sizeof(aRowLabel), "%s (v%s)", GameClient()->m_aClients[ClientId].m_aName, StatusInfo.m_aVersion);
		else
			str_copy(aRowLabel, GameClient()->m_aClients[ClientId].m_aName);

		Ui()->DoLabel(&Label, aRowLabel, 14.0f, TEXTALIGN_ML);
	}

	s_ScrollRegion.End();

	struct SVeburyPreset
	{
		CButtonContainer *m_pButton;
		const char *m_pLabel;
		const char *m_pValue;
		const char *m_pTooltip;
	};
	const auto &&RenderPresetRow = [&](CUIRect Row, const SVeburyPreset *pPresets, size_t NumPresets, CLineInput *pTarget) {
		const float PresetSpacing = 4.0f;
		const float PresetWidth = (Row.w - PresetSpacing * (NumPresets - 1)) / NumPresets;
		for(size_t i = 0; i < NumPresets; i++)
		{
			const SVeburyPreset &Preset = pPresets[i];
			CUIRect PresetButton;
			Row.VSplitLeft(PresetWidth, &PresetButton, &Row);
			Row.VSplitLeft(PresetSpacing, nullptr, &Row);
			if(DoButton_Menu(Preset.m_pButton, Preset.m_pLabel, 0, &PresetButton, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 4.0f))
				pTarget->Set(Preset.m_pValue);
			GameClient()->m_Tooltips.DoToolTip(Preset.m_pButton, &PresetButton, Localize(Preset.m_pTooltip));
		}
	};

	// Right panel: reason + time.
	CUIRect ReasonLabel, ReasonBox, ReasonPresetRow, TimeLabel, TimeBox, TimePresetRow, SelectedInfo, DeselectButton;
	RightPanel.HSplitTop(20.0f, &ReasonLabel, &RightPanel);
	Ui()->DoLabel(&ReasonLabel, Localize("Reason:"), 14.0f, TEXTALIGN_ML);
	RightPanel.HSplitTop(24.0f, &ReasonBox, &RightPanel);
	m_VeburyReasonInput.SetEmptyText(Localize("Rule violation"));
	Ui()->DoEditBox(&m_VeburyReasonInput, &ReasonBox, 14.0f);

	RightPanel.HSplitTop(6.0f, nullptr, &RightPanel);

	// Quick presets: fill the reason field with common reasons.
	RightPanel.HSplitTop(20.0f, &ReasonPresetRow, &RightPanel);
	static CButtonContainer s_PresetReasonFlood, s_PresetReasonCheat, s_PresetReasonBot;
	const SVeburyPreset aReasonPresets[] = {
		{&s_PresetReasonFlood, "Flood", "Flood", "Reason: Flood"},
		{&s_PresetReasonCheat, "Cheat", "Cheat", "Reason: Cheat"},
		{&s_PresetReasonBot, "Bot", "Bot", "Reason: using a bot/script"},
	};
	RenderPresetRow(ReasonPresetRow, aReasonPresets, std::size(aReasonPresets), &m_VeburyReasonInput);

	RightPanel.HSplitTop(14.0f, nullptr, &RightPanel);

	RightPanel.HSplitTop(20.0f, &TimeLabel, &RightPanel);
	Ui()->DoLabel(&TimeLabel, Localize("Time"), 14.0f, TEXTALIGN_ML);
	RightPanel.HSplitTop(24.0f, &TimeBox, &RightPanel);
	m_VeburyTimeInput.SetEmptyText("0");
	Ui()->DoEditBox(&m_VeburyTimeInput, &TimeBox, 14.0f);

	RightPanel.HSplitTop(6.0f, nullptr, &RightPanel);

	// Quick presets: fill the time field with common durations.
	RightPanel.HSplitTop(20.0f, &TimePresetRow, &RightPanel);
	static CButtonContainer s_PresetBan3600, s_PresetMute300, s_PresetMute500, s_PresetMute700;
	const SVeburyPreset aTimePresets[] = {
		{&s_PresetBan3600, "3600m", "3600", "Ban: 3600 minutes"},
		{&s_PresetMute300, "300s", "300", "Mute: 300 seconds"},
		{&s_PresetMute500, "500s", "500", "Mute: 500 seconds"},
		{&s_PresetMute700, "700s", "700", "Mute: 700 seconds"},
	};
	RenderPresetRow(TimePresetRow, aTimePresets, std::size(aTimePresets), &m_VeburyTimeInput);
	int NumSelected = 0;
	for(bool Sel : m_aVeburySelectedPlayers)
		if(Sel)
			NumSelected++;

	char aSelectedBuf[64];
	str_format(aSelectedBuf, sizeof(aSelectedBuf), Localize("%d player(s) selected"), NumSelected);
	RightPanel.HSplitTop(20.0f, &SelectedInfo, &RightPanel);
	Ui()->DoLabel(&SelectedInfo, aSelectedBuf, 13.0f, TEXTALIGN_ML);

	RightPanel.HSplitTop(6.0f, nullptr, &RightPanel);

	RightPanel.HSplitTop(22.0f, &DeselectButton, &RightPanel);
	static CButtonContainer s_DeselectAllButton;
	if(DoButton_Menu(&s_DeselectAllButton, Localize("Deselect all"), 0, &DeselectButton) && NumSelected > 0)
	{
		for(bool &Selected : m_aVeburySelectedPlayers)
			Selected = false;
	}

	RightPanel.HSplitTop(14.0f, nullptr, &RightPanel);

	// Ban / Kick / Mute, stacked right below the selection controls.
	const bool HasSelection = NumSelected > 0;

	CUIRect BanButton, KickButton, MuteButton;
	RightPanel.HSplitTop(24.0f, &BanButton, &RightPanel);
	RightPanel.HSplitTop(6.0f, nullptr, &RightPanel);
	RightPanel.HSplitTop(24.0f, &KickButton, &RightPanel);
	RightPanel.HSplitTop(6.0f, nullptr, &RightPanel);
	RightPanel.HSplitTop(24.0f, &MuteButton, &RightPanel);

	static CButtonContainer s_BanButton, s_KickButton, s_MuteButton;
	if(DoButton_Menu(&s_BanButton, Localize("Ban"), 0, &BanButton) && HasSelection)
		VeburyExecuteAction(EVeburyAction::BAN);
	GameClient()->m_Tooltips.DoToolTip(&s_BanButton, &BanButton, Localize("Ban all selected players (rcon: ban)"));

	if(DoButton_Menu(&s_KickButton, Localize("Kick"), 0, &KickButton) && HasSelection)
		VeburyExecuteAction(EVeburyAction::KICK);
	GameClient()->m_Tooltips.DoToolTip(&s_KickButton, &KickButton, Localize("Kick all selected players (rcon: kick)"));

	if(DoButton_Menu(&s_MuteButton, Localize("Mute"), 0, &MuteButton) && HasSelection)
		VeburyExecuteAction(EVeburyAction::MUTE);
	GameClient()->m_Tooltips.DoToolTip(&s_MuteButton, &MuteButton, Localize("Mute all selected players (rcon: muteid)"));
}

void CMenus::RenderVeburyLog(CUIRect MainView)
{
	if(m_vVeburyLog.empty())
	{
		Ui()->DoLabel(&MainView, Localize("No moderation actions recorded yet."), 14.0f, TEXTALIGN_MC);
		return;
	}

	static CScrollRegion s_ScrollRegion;
	CScrollRegionParams ScrollParams;
	ScrollParams.m_ScrollUnit = 3 * 46.0f;
	s_ScrollRegion.Begin(&MainView, &ScrollParams);

	// Newest entries first.
	for(auto It = m_vVeburyLog.rbegin(); It != m_vVeburyLog.rend(); ++It)
	{
		const SVeburyLogEntry &Entry = *It;

		CUIRect Row;
		MainView.HSplitTop(42.0f, &Row, &MainView);
		MainView.HSplitTop(4.0f, nullptr, &MainView);
		if(!s_ScrollRegion.AddRect(Row))
			continue;

		Row.Draw(ColorRGBA(1.0f, 1.0f, 1.0f, 0.05f), IGraphics::CORNER_ALL, 4.0f);
		Row.Margin(6.0f, &Row);

		CUIRect Line1, Line2;
		Row.HSplitTop(Row.h / 2.0f, &Line1, &Line2);

		char aLine1[256];
		str_format(aLine1, sizeof(aLine1), "[%s] %s — %s (v%s, %s)", Entry.m_aTimestamp, Entry.m_aAction, Entry.m_aName, Entry.m_aVersion, Entry.m_aIp);
		Ui()->DoLabel(&Line1, aLine1, 13.0f, TEXTALIGN_ML);

		char aLine2[256];
		str_format(aLine2, sizeof(aLine2), "%s: %s | %s: %s", Localize("Reason"), Entry.m_aReason[0] ? Entry.m_aReason : "-", Localize("Duration"), Entry.m_aDuration);
		Ui()->DoLabel(&Line2, aLine2, 12.0f, TEXTALIGN_ML);
	}

	s_ScrollRegion.End();
}
