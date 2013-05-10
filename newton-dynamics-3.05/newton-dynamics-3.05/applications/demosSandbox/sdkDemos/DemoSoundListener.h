/* Copyright (c) <2009> <Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/

#ifndef __DEMO_SOUND_LISTENER_H__
#define __DEMO_SOUND_LISTENER_H__

#include "toolbox_stdafx.h"
#include "dSoundManager.h"
#include "DemoListenerBase.h"

class DemoSoundListener: public DemoListenerBase, public dSoundManager
{
	public:
	DemoSoundListener(DemoEntityManager* const scene);
	~DemoSoundListener();

	virtual void PreUpdate (const NewtonWorld* const world, dFloat timestep);
	virtual void PostUpdate (const NewtonWorld* const world, dFloat timestep);
};


#endif