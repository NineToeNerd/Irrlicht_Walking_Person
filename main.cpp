/*
 * main.cpp
 *
 *  Created on: Sep 2, 2015
 *      Author: tracy
 */

#include <cstdlib>
#include <iostream>
#include <irrlicht.h>
#include "GameEventReceiver.h"

//FRAMES:
//WALK 1-24
//RUN 25-48
//IDLE 49-72
//DIE 98-145
//HIT 147-152
//ATTACK 153-164 (162??)

using namespace irr;

const f32 SIZE_SCALE = 2;

//speeds in units per second
static f32 MOVEMENT_SPEED;
const f32 WALK_SPEED = 50 * SIZE_SCALE;
const f32 RUN_SPEED = 100 * SIZE_SCALE;

const f32 ROTATION_SPEED = 30.f * SIZE_SCALE;
const f32 CAMERA_ZOOM_SPEED = 15.f * SIZE_SCALE;

const f32 GRAVITY = -1 * SIZE_SCALE/2;

enum
{
	WALK_START = 0,
	WALK_END = 23,
	RUN_START = 24,
	RUN_END = 47,
	IDLE_START = 48,
	IDLE_END = 71,
	HUH_START = 72,
	HUH_END = 96,
	DIE_START = 97,
	DIE_END = 144,
	HIT_START = 146,
	HIT_END = 151,
	ATTACK_START = 152,
	ATTACK_END = 162,
};

enum
{
    // I use this ISceneNode ID to indicate a scene node that is
    // not pickable by getSceneNodeAndCollisionPointFromRay()
    ID_IsNotPickable = 0,

    // I use this flag in ISceneNode IDs to indicate that the
    // scene node can be picked by ray selection.
    IDFlag_IsPickable = 1 << 0,

    // I use this flag in ISceneNode IDs to indicate that the
    // scene node can be highlighted.  In this example, the
    // homonids can be highlighted, but the level mesh can't.
    IDFlag_IsHighlightable = 1 << 1
};

int main()
{
	//Create event receiver for game
	GameEventReceiver receiver;

	//Create device
	IrrlichtDevice* device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(640, 480),
			16, false, false, false, &receiver);

	if (device == 0)
		return 1; // could not create device from driver type

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();

	device->setWindowCaption(L"Walking Person test");

	//Add scene with triangle selector for collision detection
	device->getFileSystem()->addFileArchive("./media/map-20kdm2.pk3");
    scene::IAnimatedMesh* q3mesh = smgr->getMesh("20kdm2.bsp");
    scene::IMeshSceneNode* q3node = 0;
    if (q3mesh)
    	q3node = smgr->addOctreeSceneNode(q3mesh->getMesh(0), 0, IDFlag_IsPickable);

    q3mesh->drop();
    scene::ITriangleSelector* selector = 0;

    if (q3node)
    {
    	q3node->setPosition(core::vector3df(-1350,-100,-1400));
        selector = smgr->createOctreeTriangleSelector(q3node->getMesh(), q3node, 128);
        q3node->setTriangleSelector(selector);
        //Not done with the selector yet, so don't drop it
    }

	//Create a player node to move around
    scene::IAnimatedMeshSceneNode* player = smgr->addAnimatedMeshSceneNode(
    		smgr->getMesh("./media/jgAnimeGirl1.b3d"));
	if (player)
	{
		player->setScale(core::vector3df(1,1,1)*SIZE_SCALE);
		player->setPosition(core::vector3df(0,0,0));
	    player->setMaterialTexture(0,driver->getTexture("./media/jgAnimeGirl1.bmp"));
		player->setMaterialFlag(video::EMF_LIGHTING, false);
		player->getMesh()->setMaterialFlag(video::EMF_LIGHTING, false);
		player->setFrameLoop(IDLE_START, IDLE_END);
	}

	//Create collision animator to keep player from going through walls & ground
	scene::ISceneNodeAnimatorCollisionResponse* anim = 0;

	const core::aabbox3d<f32>& box = player->getBoundingBox();

	//Get the height to the person's eye level, to make camera target
	//the right part of the player each frame
	f32 heightDiff = 0.8*box.MaxEdge.Y*SIZE_SCALE;

	if (selector)
	{
		core::vector3df radius = box.MaxEdge - box.getCenter();
		anim = smgr->createCollisionResponseAnimator(selector, player,
				radius, core::vector3df(0, GRAVITY, 0), core::vector3df(0,0,0));
		selector->drop(); //done with selector
		player->addAnimator(anim);
	}

	//Create a third-person camera
	scene::ICameraSceneNode* cameraTP = smgr->addCameraSceneNode(player,
			core::vector3df(0,30,-30), core::vector3df(0,0,0), ID_IsNotPickable);

	//Create a first-person camera
	scene::ICameraSceneNode* cameraFP = smgr->addCameraSceneNode(player,
			core::vector3df(0,heightDiff/SIZE_SCALE,2*SIZE_SCALE),
			core::vector3df(0,heightDiff/SIZE_SCALE,10*SIZE_SCALE), ID_IsNotPickable);

	smgr->setActiveCamera(cameraTP);

	scene::ISceneCollisionManager* collMgr = smgr->getSceneCollisionManager();

	//In order to do frame rate independent movement, we have to know
	//how long it was since the last frame
	u32 then = device->getTimer()->getTime();

	//Keep track of frame number for various restrictions
	u32 frames = 0;

	//Keep track of last frame the player jumped to restrict from flying
	s32 lastFrameJump = -1;

	//Keep camera type from toggling rapidly when pressing C
	s32 lastCameraChange = -1;

	//game loop
	while (device->run())
	if (device->isWindowActive())
	{
		//Get the time difference for frame-independent movement and rotation
		const u32 now = device->getTimer()->getTime();
		const f32 frameDeltaTime = (f32)(now - then) / 1000.f; //in seconds
		then = now;

		 //Set up variables for later movement and rotation calculations
		core::vector3df position = player->getPosition();
		core::vector3df oldPosition = position;
		core::vector3df camPosition = cameraTP->getAbsolutePosition();
		camPosition.Y = position.Y; //so the fwdVector doesn't point down
		core::vector3df fwdVector = (position - camPosition).normalize();
		core::vector3df rotation = player->getRotation();

		s32 startFrame = player->getStartFrame();

		//Rotate player if holding the left/right arrow keys
		if (receiver.IsKeyDown(KEY_LEFT))
		    rotation.Y -= ROTATION_SPEED*frameDeltaTime;
		else if (receiver.IsKeyDown(KEY_RIGHT))
        	rotation.Y += ROTATION_SPEED*frameDeltaTime;

		player->setRotation(rotation);

        if (receiver.IsKeyDown(KEY_SPACE)) //jump
        {
        	if ((frames - lastFrameJump) > 300) //can't jump too often
        	{
        		anim->jump(0.6);
            	lastFrameJump = frames;
        	}
        }

        if (receiver.IsKeyDown(KEY_KEY_R))
        {
        	MOVEMENT_SPEED = RUN_SPEED; //RUN
        }
        else
        {
        	MOVEMENT_SPEED = WALK_SPEED; //WALK
        }

        //set the frame-independent forward movement vector
        core::vector3df positionDiff = fwdVector * (MOVEMENT_SPEED * frameDeltaTime);

        //Move character if up/down arrow keys are being held down
        if (receiver.IsKeyDown(KEY_UP))
        	position += positionDiff;
        else if (receiver.IsKeyDown(KEY_DOWN))
          	position -= positionDiff;

        player->setPosition(position);

        if (oldPosition == position)
        {
        	if (startFrame != IDLE_START)
        	    player->setFrameLoop(IDLE_START, IDLE_END);
        }
        else
        {
        	if (MOVEMENT_SPEED == RUN_SPEED && startFrame != RUN_START)
        	{
        		player->setFrameLoop(RUN_START, RUN_END);
        	}
        	else if (MOVEMENT_SPEED == WALK_SPEED && startFrame != WALK_START)
        	{
        		player->setFrameLoop(WALK_START, WALK_END);
        	}

        }

        scene::ICameraSceneNode* activeCamera = smgr->getActiveCamera();

        if (receiver.IsKeyDown(KEY_KEY_C) && (frames - lastCameraChange) > 50)
        {
        	if (activeCamera == cameraTP)
        		smgr->setActiveCamera(cameraFP);
        	else
        		smgr->setActiveCamera(cameraTP);
        	lastCameraChange = frames;
        }



        if (activeCamera == cameraTP)
        {
            //Camera zooms if +/- keys held down
            camPosition = cameraTP->getPosition();

            if (receiver.IsKeyDown(KEY_PLUS) && camPosition.Z < -4*SIZE_SCALE) //ZOOM IN
            {
                camPosition.Z += CAMERA_ZOOM_SPEED*frameDeltaTime;
                camPosition.Y -= 0.5*CAMERA_ZOOM_SPEED*frameDeltaTime;
            }
            else if (receiver.IsKeyDown(KEY_MINUS) && camPosition.Z > -50*SIZE_SCALE) //ZOOM OUT
            {
        	    camPosition.Z -= CAMERA_ZOOM_SPEED*frameDeltaTime;
        	    camPosition.Y += 0.5*CAMERA_ZOOM_SPEED*frameDeltaTime;
            }

            //Don't want the camera pointing at the person's feet
            position.Y += heightDiff;

            cameraTP->setTarget(position);
            cameraTP->setPosition(camPosition);
            cameraTP->updateAbsolutePosition();
        }
        else
        {
        	fwdVector.Y += heightDiff;
        	fwdVector.X *= 20*SIZE_SCALE;
        	fwdVector.Z *= 20*SIZE_SCALE;
        	cameraFP->setTarget(position + fwdVector);
        }

        //Draw the scene
        driver->beginScene(true, true, 0);
        smgr->drawAll();
        driver->endScene();

        //Keep count of frame number
        frames++;
	}
	anim->drop();
	device->drop();

	return 0;
}
