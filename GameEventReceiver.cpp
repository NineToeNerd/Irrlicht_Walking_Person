/*
 * GameEventReceiver.cpp
 *
 *  Created on: Sep 2, 2015
 *      Author: tracy
 */

#include "GameEventReceiver.h"

GameEventReceiver::GameEventReceiver()
{
	for (irr::u32 i=0; i<irr::KEY_KEY_CODES_COUNT; ++i)
        KeyIsDown[i] = false;

	X = 640/2;
	Y = 480/2;
}

bool GameEventReceiver::OnEvent(const irr::SEvent& event)
{
	//Remember whether each key is down or up
	if (event.EventType == irr::EET_KEY_INPUT_EVENT)
		KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;

	//Keep track of mouse position for rotation
	X = event.MouseInput.X;
	Y = event.MouseInput.Y;

	return false;
}

bool GameEventReceiver::IsKeyDown(irr::EKEY_CODE keyCode) const
{
	return KeyIsDown[keyCode];
}
