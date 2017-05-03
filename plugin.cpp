/**************************************************************************
 *  JClassInfo plug-in for FAR 3.0                                        *
 *  Copyright (C) 2012-2014 by Artem Senichev <artemsen@gmail.com>        *
 *  https://sourceforge.net/projects/farplugs/                            *
 *                                                                        *
 *  This program is free software: you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                   *
 *                                                                        *
 *  This program is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 **************************************************************************/

#include "common.h"
#include "panel.h"
#include "jclass.h"
#include "settings.h"
#include "version.h"

//! Plugin GUID {9963EEF7-260B-4B46-89AA-FB7BC9ABD5CD}
const GUID _FPG = { 0x9963eef7, 0x260b, 0x4b46, { 0x89, 0xaa, 0xfb, 0x7b, 0xc9, 0xab, 0xd5, 0xcd } };

PluginStartupInfo    _PSI;
FarStandardFunctions _FSF;


void WINAPI SetStartupInfoW(const PluginStartupInfo* psi)
{
	_PSI = *psi;
	_FSF = *psi->FSF;
	_PSI.FSF = &_FSF;

	settings::load();
}


void WINAPI GetGlobalInfoW(GlobalInfo* info)
{
	info->StructSize = sizeof(GlobalInfo);
	info->MinFarVersion = FARMANAGERVERSION;
	info->Version = MAKEFARVERSION(PLUGIN_VERSION_NUM, VS_RELEASE);
	info->Guid = _FPG;
	info->Title = TEXT(PLUGIN_NAME);
	info->Description = TEXT(PLUGIN_DESCR);
	info->Author = TEXT(PLUGIN_AUTHOR);
}


void WINAPI GetPluginInfoW(PluginInfo* info)
{
	assert(info);

	info->StructSize = sizeof(PluginInfo);

	if (!settings::cmd_prefix.empty())
		info->CommandPrefix = settings::cmd_prefix.c_str();

	static const wchar_t* menu_strings[1];
	menu_strings[0] = TEXT(PLUGIN_NAME);

	info->PluginConfig.Guids = &_FPG;
	info->PluginConfig.Strings = menu_strings;
	info->PluginConfig.Count = sizeof(menu_strings) / sizeof(menu_strings[0]);

	if (!settings::add_to_panel_menu)
		info->Flags |= PF_DISABLEPANELS;
	else {
		info->PluginMenu.Guids = &_FPG;
		info->PluginMenu.Strings = menu_strings;
		info->PluginMenu.Count = sizeof(menu_strings) / sizeof(menu_strings[0]);
	}

#ifdef _DEBUG
	info->Flags |= PF_PRELOAD;
#endif // _DEBUG
}


HANDLE WINAPI AnalyseW(const AnalyseInfo* info)
{
	if (!info || info->StructSize < sizeof(AnalyseInfo) || !info->FileName)
		return nullptr;
	if (!jclass::format_supported(static_cast<const unsigned char*>(info->Buffer), info->BufferSize))
		return nullptr;
	return panel::open(info->FileName, true);
}


HANDLE WINAPI OpenW(const OpenInfo* info)
{
	if (!info || info->StructSize < sizeof(OpenInfo))
		return nullptr;
	if (info->OpenFrom == OPEN_ANALYSE && info->Data)
		return reinterpret_cast<OpenAnalyseInfo*>(info->Data)->Handle;

	//Determine file name for open
	wstring file_name;
	if (info->OpenFrom == OPEN_COMMANDLINE && info->Data) {
		const OpenCommandLineInfo* ocli = reinterpret_cast<const OpenCommandLineInfo*>(info->Data);
		if (!ocli || ocli->StructSize < sizeof(OpenCommandLineInfo) || !ocli->CommandLine || !ocli->CommandLine[0])
			return nullptr;
		//Get command line
		wstring cmd_line = ocli->CommandLine;
		size_t pos = 0;
		while ((pos = cmd_line.find(L'\"', pos)) != string::npos)
			cmd_line.erase(pos, 1);
		while (!cmd_line.empty() && iswspace(cmd_line[0]))
			cmd_line.erase(0, 1);
		while (!cmd_line.empty() && iswspace(cmd_line[cmd_line.length() - 1]))
			cmd_line.erase(cmd_line.length() - 1, 1);
		if (cmd_line.empty())
			return nullptr;
		//Expand environment variables in path string
		wstring exp_path(2048, 0);
		if (ExpandEnvironmentStrings(cmd_line.c_str(), &exp_path.front(), static_cast<DWORD>(exp_path.size() - 1)))
			exp_path.resize(lstrlen(exp_path.c_str()));
		else
			exp_path = cmd_line;
		const size_t path_len = _FSF.ConvertPath(CPM_FULL, exp_path.c_str(), nullptr, 0);
		if (path_len) {
			file_name.resize(path_len);
			_FSF.ConvertPath(CPM_FULL, exp_path.c_str(), &file_name[0], path_len);
		}
	}
	else if (info->OpenFrom == OPEN_PLUGINSMENU) {
		PanelInfo pi;
		ZeroMemory(&pi, sizeof(pi));
		pi.StructSize = sizeof(pi);
		if (!_PSI.PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, &pi))
			return nullptr;
		const intptr_t ppi_len = _PSI.PanelControl(PANEL_ACTIVE, FCTL_GETPANELITEM, static_cast<intptr_t>(pi.CurrentItem), nullptr);
		if (ppi_len == 0)
			return nullptr;
		vector<unsigned char> buffer(ppi_len);
		PluginPanelItem* ppi = reinterpret_cast<PluginPanelItem*>(&buffer.front());
		FarGetPluginPanelItem fgppi;
		ZeroMemory(&fgppi, sizeof(fgppi));
		fgppi.StructSize = sizeof(fgppi);
		fgppi.Size = buffer.size();
		fgppi.Item = ppi;
		if (!_PSI.PanelControl(PANEL_ACTIVE, FCTL_GETPANELITEM, static_cast<intptr_t>(pi.CurrentItem), &fgppi))
			return nullptr;
		const size_t file_name_len = _FSF.ConvertPath(CPM_FULL, ppi->FileName, nullptr, 0);
		if (file_name_len) {
			file_name.resize(file_name_len);
			_FSF.ConvertPath(CPM_FULL, ppi->FileName, &file_name[0], file_name_len);
		}
	}

	return file_name.empty() ? nullptr : panel::open(file_name.c_str(), false);
}


void WINAPI GetOpenPanelInfoW(OpenPanelInfo* info)
{
	if (info && info->StructSize >= sizeof(OpenPanelInfo) && info->hPanel)
		reinterpret_cast<panel*>(info->hPanel)->get_panel_info(*info);
}


void WINAPI ClosePanelW(const ClosePanelInfo* info)
{
	if (info && info->StructSize >= sizeof(ClosePanelInfo) && info->hPanel)
		delete reinterpret_cast<panel*>(info->hPanel);
}


intptr_t WINAPI GetFindDataW(GetFindDataInfo* info)
{
	if (!info || info->StructSize < sizeof(GetFindDataInfo) || !info->hPanel)
		return 0;
	reinterpret_cast<panel*>(info->hPanel)->get_panel_list(&info->PanelItem, info->ItemsNumber);
	return 1;
}


void WINAPI FreeFindDataW(const FreeFindDataInfo* info)
{
	if (info && info->StructSize >= sizeof(FreeFindDataInfo) && info->hPanel)
		reinterpret_cast<panel*>(info->hPanel)->free_panel_list(info->PanelItem, info->ItemsNumber);
}


intptr_t WINAPI ProcessPanelInputW(const ProcessPanelInputInfo* info)
{
	if (!info || info->StructSize < sizeof(ProcessPanelInputInfo) || info->Rec.EventType != KEY_EVENT || !info->hPanel)
		return 0;
	return reinterpret_cast<panel*>(info->hPanel)->handle_keyboard(info->Rec.Event.KeyEvent) ? 1 : 0;
}


intptr_t WINAPI CompareW(const CompareInfo* info)
{
	if (!info || info->StructSize < sizeof(CompareInfo))
		return -1;
	if (info->Item1->FileSize != info->Item2->FileSize)
		return (static_cast<intptr_t>(info->Item2->FileSize) - static_cast<intptr_t>(info->Item1->FileSize));
	return wcscmp(info->Item1->AlternateFileName, info->Item2->AlternateFileName);
}


intptr_t WINAPI ConfigureW(const ConfigureInfo* info)
{
	if (!info || info->StructSize < sizeof(ConfigureInfo))
		return 0;
	settings::configure();
	return 0;
}
