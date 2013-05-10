/////////////////////////////////////////////////////////////////////////////
// Name:        dPluginRecord.h
// Purpose:     
// Author:      Julio Jerez
// Modified by: 
// Created:     22/05/2010 07:45:05
// RCS-ID:      
// Copyright:   Copyright (c) <2010> <Newton Game Dynamics>
// License:     
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
// 
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely
/////////////////////////////////////////////////////////////////////////////

#ifndef __D_PLUGIN_RECORD__
#define __D_PLUGIN_RECORD__

#ifdef D_PLUGIN_EXPORT
	#define D_PLUGIN_API __declspec(dllexport)
#else
	#define D_PLUGIN_API __declspec(dllimport)
#endif



class dPluginRecord
{
	public:
	enum dPluginType
	{
		m_import = 0,
		m_export,
		m_model,
		m_mesh,
		m_body,
		m_collision,
	};

	dPluginRecord(void);
	virtual ~dPluginRecord(void);

	virtual const char* GetMenuName () {return NULL;}
	virtual const char* GetFileExtension () {return NULL;}
	virtual const char* GetFileDescription () {return NULL;}


	virtual const char* GetSignature () = 0;
	virtual dPluginType GetType () = 0;

};

#endif