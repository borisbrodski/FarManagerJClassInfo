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

#include "jclass.h"


//! Java class file file header
#define JCLASS_HEADER 0xcafebabe

//! Default name for unknown values
static const wchar_t* UNKNOWN_NAME = L"{Unknown}";


bool jclass::format_supported(const unsigned char* file_hdr, const size_t file_hdr_len)
{
	return (file_hdr && file_hdr_len >= 4 && be2le(*reinterpret_cast<const uint32_t*>(file_hdr)) == JCLASS_HEADER);
}


bool jclass::read(const wchar_t* file_name, jclassinfo& class_info, vector<jmember>& members)
{
	assert(file_name && *file_name);

	try {
		if (!read_java_class(file_name))
			return false;

		map<uint16_t, wstring>::const_iterator it_string;

		//Fill output info for class description
		assert(_class_name && _const_pool.size() > _class_name && _const_pool[_class_name - 1].type == CONSTANT_Class);
		class_info.access = _class_access_flag;
		class_info.name = get_string(be2le(_const_pool[_class_name - 1].cp_class->name_index));
		if (class_info.name.empty())
			class_info.name = UNKNOWN_NAME;

		assert(_super_class && _const_pool.size() > _super_class && _const_pool[_super_class - 1].type == CONSTANT_Class);
		class_info.super = get_string(be2le(_const_pool[_super_class - 1].cp_class->name_index));

		//Fill output info for methods and fields description
		get_member_descr(method, members);
		get_member_descr(field, members);
	}
	catch (...) {
		return false;
	}

	return true;
}


bool jclass::read_java_class(const wchar_t* file_name)
{
	assert(file_name && *file_name);

	//Load file
	HANDLE file = CreateFile(file_name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE)
		return false;
	LARGE_INTEGER file_size;
	if (!GetFileSizeEx(file, &file_size) || file_size.QuadPart == 0 || file_size.QuadPart > 1024 * 1024 * 1024 /* 1G */) {
		CloseHandle(file);
		return false;
	}
	_data_buff.resize(static_cast<size_t>(file_size.QuadPart));
	DWORD bytes_read = 0;
	if (!ReadFile(file, &_data_buff.front(), static_cast<DWORD>(file_size.QuadPart), &bytes_read, nullptr)) {
		CloseHandle(file);
		return false;
	}
	CloseHandle(file);

	_data_pos = 0;

	//Header
	const uint32_t jclass_hdr = read_num<uint32_t>();
	if (jclass_hdr != JCLASS_HEADER)
		return false;

	//Minor and major version numbers of this class file
	/*const uint16_t minor_version =*/ read(sizeof(uint16_t));
	/*const uint16_t major_version =*/ read(sizeof(uint16_t));

	//Table of structures representing various string constants, class e t.c.
	read_constant_pool();

	//Mask of flags used to denote access permissions to and properties of this class or interface
	_class_access_flag = read_num<uint16_t>();

	//The value of the this_class item must be a valid index into the constant_pool table
	_class_name = read_num<uint16_t>();

	//Super class value must be zero or must be a valid index into the constant_pool table
	_super_class = read_num<uint16_t>();

	//Super interfaces of this class or interface
	read_interfaces();

	//Represent all fields, both class variables and instance variables, declared by this class or interface type
	read_fields();

	//The method info structures represent all methods declared by this class or interface type
	read_methods();

	//Not needed
	//read_attributes();

	return true;
}


void jclass::read_constant_pool()
{
	const uint16_t constant_pool_count = read_num<uint16_t>();
	for (uint16_t i = 1; i < constant_pool_count; ++i) {
		j_const_pool pool;
		pool.type = static_cast<const_pool_type>(read_num<uint8_t>());
		pool.data = &_data_buff[_data_pos];
		_const_pool.push_back(pool);
		switch (pool.type) {
			case CONSTANT_Class:				read(sizeof(const_pool_class)); break;
			case CONSTANT_Fieldref:				read(sizeof(const_pool_fieldref)); break;
			case CONSTANT_Methodref:			read(sizeof(const_pool_methodref)); break;
			case CONSTANT_InterfaceMethodref:	read(sizeof(const_pool_interfmethodref)); break;
			case CONSTANT_String:				read(sizeof(const_pool_string)); break;
			case CONSTANT_Integer:				read(sizeof(const_pool_integer)); break;
			case CONSTANT_Float:				read(sizeof(const_pool_float)); break;
			case CONSTANT_Long:
				read(sizeof(const_pool_long));
				//Phantom pool item
				_const_pool.push_back(j_const_pool());
				i++;
				break;
			case CONSTANT_Double:
				read(sizeof(const_pool_double));
				//Phantom pool item
				_const_pool.push_back(j_const_pool());
				i++;
				break;
			case CONSTANT_NameAndType:			read(sizeof(const_pool_nameandtype)); break;
			case CONSTANT_Utf8:					read(read_num<uint16_t>()); break;
			default:
				throw exception();
		}
	}
}


void jclass::read_interfaces()
{
	const uint16_t interfaces_count = read_num<uint16_t>();
	for (uint16_t i = 0; i < interfaces_count; ++i) {
		//Ignore all
		/*const uint16_t name_index =*/ read(sizeof(uint16_t));
	}
}


void jclass::read_fields()
{
	const uint16_t fields_count = read_num<uint16_t>();
	for (uint16_t i = 0; i < fields_count; ++i) {
		j_field f;
		f.access_flag = read_num<uint16_t>();
		f.name_index = read_num<uint16_t>();
		f.descriptor_index = read_num<uint16_t>();
		read_attributes();
		_fields.insert(make_pair(i, f));
	}
}


void jclass::read_methods()
{
	const uint16_t methods_count = read_num<uint16_t>();
	for (uint16_t i = 0; i < methods_count; ++i) {
		j_method m;
		m.access_flag = read_num<uint16_t>();
		m.name_index = read_num<uint16_t>();
		m.descriptor_index = read_num<uint16_t>();
		read_attributes();
		_methods.insert(make_pair(i, m));
	}
}


void jclass::read_attributes()
{
	const uint16_t attributes_count = read_num<uint16_t>();
	for (uint16_t i = 0; i < attributes_count; ++i) {
		//Ignore all
		/*const uint16_t attribute_name_index =*/ read(sizeof(uint16_t));
		const uint32_t attribute_length = read_num<uint32_t>();
		/*const unsigned char* info =*/ read(attribute_length);
	}
}


wstring jclass::get_string(const uint16_t index) const
{
	assert(index);
	const j_const_pool& pool_item = _const_pool[index - 1];
	assert(pool_item.type == CONSTANT_Utf8);
	const uint16_t length = be2le(pool_item.cp_utf8->length);
	assert(length);

	wstring wide;
	const int req = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&pool_item.cp_utf8->bytes), static_cast<int>(length), nullptr, 0);
	if (req) {
		wide.resize(static_cast<size_t>(req));
		MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&pool_item.cp_utf8->bytes), static_cast<int>(length), &wide.front(), req);
	}
	return wide;
}


void jclass::get_member_descr(const jmember_type type, vector<jmember>& members) const
{
	map<uint16_t, j_method>::const_iterator it_b = (type == method ? _methods.begin() : _fields.begin());
	map<uint16_t, j_method>::const_iterator it_e = (type == method ? _methods.end() : _fields.end());
	for (map<uint16_t, j_method>::const_iterator it = it_b; it != it_e; ++it) {
		jmember met;
		met.name = get_string(it->second.name_index);
		if (met.name.empty())
			met.name = UNKNOWN_NAME;
		met.description = get_string(it->second.descriptor_index);
		met.access = it->second.access_flag;
		met.type = type;
		members.push_back(met);
	}
}


const unsigned char* jclass::read(const size_t len)
{
	if (_data_pos + len >= _data_buff.size())
		throw exception();
	const unsigned char* data = &_data_buff[_data_pos];
	_data_pos += len;
	return data;
}
