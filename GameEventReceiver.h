/*
 * GameEventReceiver.h
 *
 *  Created on: Sep 2, 2015
 *      Author: tracy
 */

#ifndef GAMEEVENTRECEIVER_H_
#define GAMEEVENTRECEIVER_H_

#include <irrlicht.h>

class GameEventReceiver : public irr::IEventReceiver {
public:
	virtual bool OnEvent(const irr::SEvent& event);
	virtual bool IsKeyDown(irr::EKEY_CODE keyCode) const;
	irr::s32 GetX() const { return X; };
	irr::s32 GetY() const { return Y; };
	GameEventReceiver();

private:
	bool KeyIsDown[irr::KEY_KEY_CODES_COUNT];
	irr::s32 X;
	irr::s32 Y;
};

#endif /* GAMEEVENTRECEIVER_H_ */
