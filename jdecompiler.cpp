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

#include "jdecompiler.h"
#include "jtformat.h"
#include "version.h"
#include <shlobj.h>
#include <fstream>
#include <regex>

#define DECOMPILER_WAITTIME	10000


bool jdecompiler::decompile(const wchar_t* file_name, const decompiler jd)
{
	assert(file_name && file_name[0]);

	if (jd != jd_jad && _java_bin_path.empty() && !find_java_bin(_java_bin_path)) {
		const wchar_t* msg[] = { TEXT(PLUGIN_NAME), L"Unable to decompile class file: Java interpreter not found" };
		_PSI.Message(&_FPG, &_FPG, FMSG_WARNING | FMSG_MB_OK, nullptr, msg, sizeof(msg) / sizeof(msg[0]), 0);
		return false;
	}

	bool rc = false;

	_PSI.AdvControl(&_FPG, ACTL_SETPROGRESSSTATE, TBPF_INDETERMINATE, nullptr);
	const wchar_t* msg[] = { TEXT(PLUGIN_NAME), L"Decompilation in progress..." };
	_PSI.Message(&_FPG, &_FPG, FMSG_NONE, nullptr, msg, sizeof(msg) / sizeof(msg[0]), 0);

	switch (jd) {
		case jd_jad: rc = decompile_jad(file_name); break;
		case jd_fernflower: rc = decompile_fernflower(file_name); break;
		case jd_javap: rc = decompile_javap(file_name); break;
	}

	_PSI.AdvControl(&_FPG, ACTL_PROGRESSNOTIFY, 0, nullptr);
	_PSI.AdvControl(&_FPG, ACTL_SETPROGRESSSTATE, TBPF_NOPROGRESS, nullptr);
	_PSI.PanelControl(PANEL_ACTIVE, FCTL_REDRAWPANEL, 0, nullptr);
	_PSI.PanelControl(PANEL_PASSIVE, FCTL_REDRAWPANEL, 0, nullptr);

	if (!rc) {
		wstring err_msg = TEXT(PLUGIN_NAME);
		err_msg += L'\n';
		err_msg += L"Unable to decompile class file with ";
		switch (jd) {
			case jd_jad: err_msg += L"JAD"; break;
			case jd_fernflower: err_msg += L"Fernflower"; break;
			case jd_javap: err_msg += L"javap"; break;
		}
		_PSI.Message(&_FPG, &_FPG, FMSG_ALLINONE | FMSG_WARNING | FMSG_MB_OK, nullptr, reinterpret_cast<const wchar_t* const*>(err_msg.c_str()), 0, 0);
	}

	return rc;
}


intptr_t jdecompiler::find_line(const jclass::jmember& member) const
{
	assert(!_java_file_name.empty());

	intptr_t line_num = 1;

	const char* jam[] = { "public", "protected", "private", "static", "abstract", "final", "volatile", "super", "transient", "abstract" };

	try {
		jtformat jf;
		string type_name = w2a(jf.get_type_name(member));
		size_t pos = 0;
		while ((pos = type_name.find_first_of("[]")) != string::npos)
			type_name.erase(pos, 1);
		string regex_tmpl = "\\s*((\\s)";
		for (size_t i = 0; i < sizeof(jam) / sizeof(jam[0]); ++i) {
			regex_tmpl += L'|';
			regex_tmpl += '(';
			regex_tmpl += jam[i];
			regex_tmpl += ')';
		}
		regex_tmpl += ")*\\s*";
		regex_tmpl += type_name;
		regex_tmpl += "\\s*[\\[\\]]*\\s*";
		regex_tmpl += w2a(member.name);
		regex_tmpl += "\\s*[\\[\\]]*\\s*";
		if (member.type == jclass::method)
			regex_tmpl += "(.*)";
		else
			regex_tmpl += "\\s*(=.*;|;)";
		regex rx(regex_tmpl.c_str());
		ifstream file_stream;
		file_stream.open(_java_file_name, fstream::in);
		if (file_stream.is_open()) {
			int ln = 1;
			while (line_num == 1 && !file_stream.eof()) {
				string chk_str;
				getline(file_stream, chk_str);
				if (regex_match(chk_str, rx))
					line_num = ln;
				++ln;
			}
			file_stream.close();
		}
	}
	catch (regex_error&) {
		line_num = 1;
	}

	return line_num;
}


bool jdecompiler::decompile_jad(const wchar_t* file_name)
{
	assert(file_name && file_name[0]);

	const wchar_t* decompiler_fext = L"jad.java";
	const wstring tmp_path = get_tmp_path();
	_java_file_name = tmp_path;
	_java_file_name += L'\\';
	_java_file_name += _FSF.PointToName(file_name);
	const size_t ext_pos = _java_file_name.rfind(L'.');
	if (ext_pos != string::npos)
		_java_file_name.erase(ext_pos + 1);
	_java_file_name += decompiler_fext;

	const wstring decompiler_module = module_path() + L"jad.exe";
	wstring decompiler_params = L" -nonlb -o -d \"";
	decompiler_params += tmp_path;
	decompiler_params += L"\" -s \"";
	decompiler_params += decompiler_fext;
	decompiler_params += L"\" ";
	decompiler_params += file_name;
	decompiler_params += L"\"";
	return execute(decompiler_module.c_str(), decompiler_params.c_str());
}


bool jdecompiler::decompile_fernflower(const wchar_t* file_name)
{
	assert(file_name && file_name[0]);

	const wstring tmp_path = get_tmp_path();

	wstring decompiler_params = L" -jar \"";
	decompiler_params += module_path() + L"fernflower.jar\" ";
	decompiler_params += L"\"";
	decompiler_params += file_name;
	decompiler_params += L"\" \"";
	decompiler_params += tmp_path;
	decompiler_params += L"\"";

	_java_file_name = tmp_path + L'\\';
	_java_file_name += _FSF.PointToName(file_name);
	const size_t ext_pos = _java_file_name.rfind(L'.');
	if (ext_pos != string::npos)
		_java_file_name.erase(ext_pos + 1);
	_java_file_name += L"java";

	const wstring java_exe = _java_bin_path + L"java.exe";
	return execute(java_exe.c_str(), decompiler_params.c_str(), INVALID_HANDLE_VALUE, 0);
}


bool jdecompiler::decompile_javap(const wchar_t* file_name)
{
	assert(file_name && file_name[0]);

	wstring class_path = file_name;
	const size_t cp_pos = class_path.rfind('\\');
	if (cp_pos == string::npos)
		return false;
	class_path.erase(cp_pos);

	wstring class_name = _FSF.PointToName(file_name);
	const size_t cn_pos = class_name.rfind('.');
	if (cn_pos == string::npos)
		return false;
	class_name.erase(cn_pos);

	_java_file_name = get_tmp_path();
	_java_file_name += L'\\';
	_java_file_name += class_name;
	_java_file_name += L".java";

	wstring decompiler_params = L" -verbose -private -c -classpath \"";
	decompiler_params += class_path + L"\" ";
	decompiler_params += class_name;

	const wstring javap_exe = _java_bin_path + L"javap.exe";

	SECURITY_ATTRIBUTES  sec;
	ZeroMemory(&sec, sizeof(sec));
	sec.nLength = sizeof(sec);
	sec.bInheritHandle = TRUE;
	HANDLE std_out_file = CreateFile(_java_file_name.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, &sec, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (std_out_file == INVALID_HANDLE_VALUE)
		return false;

	const bool rc = execute(javap_exe.c_str(), decompiler_params.c_str(), std_out_file, 0);
	
	CloseHandle(std_out_file);
	return rc;
}


bool jdecompiler::execute(const wchar_t* exe, const wchar_t* params, HANDLE stdout_file /*= INVALID_HANDLE_VALUE*/, const DWORD expected_code /*= 0xFFFFFFFF*/) const
{
	assert(exe && exe[0]);

	bool rc = false;

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(STARTUPINFO);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW;

	if (stdout_file != INVALID_HANDLE_VALUE) {
		si.dwFlags |= STARTF_USESTDHANDLES;
		si.hStdOutput = stdout_file;
		si.hStdError = stdout_file;
	}

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	rc = CreateProcess(exe, const_cast<wchar_t*>(params), nullptr, nullptr, TRUE, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi) != FALSE;
	if (rc && WaitForSingleObject(pi.hProcess, DECOMPILER_WAITTIME) == WAIT_TIMEOUT) {
		TerminateProcess(pi.hProcess, 0);
		rc = false;
	}
	if (rc && expected_code != 0xFFFFFFFF) {
		DWORD exit_code = 0;
		if (!GetExitCodeProcess(pi.hProcess, &exit_code) || exit_code != expected_code)
			rc = false;
	}

	if (pi.hThread)
		CloseHandle(pi.hThread);
	if (pi.hProcess)
		CloseHandle(pi.hProcess);

	return rc;
}


wstring jdecompiler::module_path() const
{
	wstring path =_PSI.ModuleName;
	path.resize(path.rfind(L'\\') + 1);
	return path;
}


wstring jdecompiler::get_tmp_path() const
{
	wchar_t tmp_path[MAX_PATH];
	if (!GetTempPath(MAX_PATH, tmp_path))
		tmp_path[0] = 0;
	if (tmp_path[wcslen(tmp_path) - 1] == L'\\')
		tmp_path[wcslen(tmp_path) - 1] = 0;
	return tmp_path;
}


bool jdecompiler::find_java_bin(wstring& java_bin_path) const
{
	const wchar_t* java_exe = L"java.exe";

	if (execute(java_exe, nullptr))
		return true;

	//Search in JAVA_HOME
	wstring java_home(1024, 0);
	java_home.resize(GetEnvironmentVariable(L"JAVA_HOME", &java_home[0], static_cast<DWORD>(java_home.size())));
	if (!java_home.empty()) {
		java_home += L"\\bin\\";
		const wstring chk_path = java_home + L"java.exe";
		if (execute(chk_path.c_str(), nullptr)) {
			java_bin_path = java_home;
			return true;
		}
	}

	//Search in registry
	const wchar_t* keys[] = {
		L"SOFTWARE\\JavaSoft\\Java Development Kit\\1.8",
		L"SOFTWARE\\JavaSoft\\Java Development Kit\\1.7",
		L"SOFTWARE\\JavaSoft\\Java Development Kit\\1.6",
 		L"SOFTWARE\\JavaSoft\\Java Runtime Environment\\1.8",
 		L"SOFTWARE\\JavaSoft\\Java Runtime Environment\\1.7",
		L"SOFTWARE\\JavaSoft\\Java Runtime Environment\\1.6"
	};
	for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
		wstring path = get_javahome_path(keys[i]);
		if (!path.empty()) {
			path += L"\\bin\\";
			const wstring chk_path = path + L"java.exe";
			if (execute(chk_path.c_str(), nullptr)) {
				java_bin_path = path;
				return true;
			}
		}
	}

	return false;
}


wstring jdecompiler::get_javahome_path(const wchar_t* key_path) const
{
	assert(key_path && key_path[0]);

	wstring path;

	HKEY reg_key;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, key_path, 0, KEY_READ, &reg_key) == ERROR_SUCCESS) {
		const wchar_t* path_key = L"JavaHome";
		DWORD data_len = 0;
		if (RegQueryValueEx(reg_key, path_key, NULL, NULL, NULL, &data_len) == ERROR_SUCCESS) {
			path.resize(data_len / sizeof(wchar_t), 0);
			if (!RegQueryValueEx(reg_key, path_key, NULL, NULL, reinterpret_cast<LPBYTE>(&path[0]), &data_len) == ERROR_SUCCESS)
				path.clear();
			else if (!path.empty() && path.back() == 0)
				path.erase(path.length() - 1);	//Remove last null
		}
		RegCloseKey(reg_key);
	}

	return path;

}


string jdecompiler::w2a(const wstring& val) const
{
	string enc;
	const int req = WideCharToMultiByte(CP_UTF8, 0, val.c_str(), static_cast<int>(val.length()), 0, 0, nullptr, nullptr);
	if (req) {
		enc.resize(static_cast<size_t>(req));
		WideCharToMultiByte(CP_UTF8, 0, val.c_str(), static_cast<int>(val.length()), &enc.front(), req, nullptr, nullptr);
	}
	return enc;
}
