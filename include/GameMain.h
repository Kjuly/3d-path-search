#ifndef __GameMain_h_
#define __GameMain_h_

#include "GameBase.h"     // 基础设置
#include "GamePlayer.h"   // 角色类

//#include <Terrain/OgreTerrain.h>
//#include <Terrain/OgreTerrainGroup.h>

class GameMain : public GameBase
{
public:
    GameMain(void);
    virtual ~GameMain(void);

protected:
    virtual void createScene(void);
    virtual void createEnvir(void);     // Environment: Light, Sky, etc.
    virtual void createCamera(void);    // Camera
    virtual void createViewports(void); // Viewport
    virtual void createFrameListener(void);
    virtual void destroyScene(void);

    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

    // OIS::KeyListener
    virtual bool keyPressed( const OIS::KeyEvent & evt );
    virtual bool keyReleased( const OIS::KeyEvent & evt );
    // OIS::MouseListener
    virtual bool mouseMoved( const OIS::MouseEvent & evt );
    virtual bool mousePressed( const OIS::MouseEvent & evt, OIS::MouseButtonID id );
    virtual bool mouseReleased( const OIS::MouseEvent & evt, OIS::MouseButtonID id );

	Ogre::Viewport * mViewport;

	// 角色对象
	GamePlayer * mPlayer;

	// 障碍物
	std::deque< Ogre::SceneNode * > boxNodes;
	std::deque< Ogre::Entity * > boxEntitys;
};

#endif // #ifndef __GameMain_h_
