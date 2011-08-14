#include "GameMain.h"

//-------------------------------------------------------------------------------------
GameMain::GameMain(void)
{
}
//-------------------------------------------------------------------------------------
GameMain::~GameMain(void)
{
}
//-------------------------------------------------------------------------------------
void GameMain::destroyScene(void)
{
	if( mPlayer ) delete mPlayer; 

	std::deque< Ogre::SceneNode * >::iterator boxNodeIter = boxNodes.begin();
	while( boxNodeIter != boxNodes.end() )
	{
		delete * boxNodeIter;
		++boxNodeIter;
	}
	boxNodes.clear();

	std::deque< Ogre::Entity * >::iterator boxEntityIter = boxEntitys.begin();
	while( boxEntityIter != boxEntitys.end() )
	{
		delete * boxEntityIter;
		++boxEntityIter;
	}
	boxEntitys.clear();
}
//-------------------------------------------------------------------------------------
void GameMain::createScene(void)
{
	createCamera();    // Camera
	createViewports(); // Viewport
	createEnvir();     // Environment: Light, Sky, etc.

	//-------------------------
	// 地平面
	Ogre::MeshManager::getSingleton().createPlane("floor", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Ogre::Plane(Ogre::Vector3::UNIT_Y, 0), 100, 100, 10, 10, true, 1, 10, 10, Ogre::Vector3::UNIT_Z);

	// create a floor entity, give it a material, and place it at the origin
	Ogre::Entity* floor = mSceneMgr->createEntity("Floor", "floor");
    floor->setMaterialName("Examples/Rockwall");
	floor->setCastShadows(false);
    mSceneMgr->getRootSceneNode()->attachObject(floor);

	//-------------------------
	// 创建障碍物分布地图
	//-------------------------
	// 创建障碍物
	/*Ogre::SceneNode * boxNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	Entity * boxEntity = mSceneMgr->createEntity("Box","cube.mesh");
	boxEntity->setMaterialName("Examples/BumpyMetal");
	
	boxNode->attachObject( boxEntity );
	boxNode->scale( 0.01f, 0.01f, 0.01f );
	boxNode->setPosition( 0,0.5,0 );

	boxNodes.push_back( boxNode );
	boxEntitys.push_back( boxEntity );*/

	Ogre::SceneNode * ogreHeadNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	Entity * ogreHeadEntity = mSceneMgr->createEntity("OgreHead","ogrehead.mesh");
	
	ogreHeadNode->attachObject( ogreHeadEntity );
	ogreHeadNode->scale( 0.02f, 0.02f, 0.02f );
	//ogreHeadNode->setPosition( 1, 0, 1 );
	ogreHeadNode->setPosition( 15, 0.5, -20 );

	boxNodes.push_back( ogreHeadNode );
	boxEntitys.push_back( ogreHeadEntity );

	//-------------------------
	// 创建角色
	mPlayer = new GamePlayer( mCamera, ogreHeadNode );
}
//-------------------------------------------------------------------------------------
void GameMain::createEnvir(void)
{
	// Light
    Ogre::Vector3 lightdir(0.55, -0.3, 0.75);
    lightdir.normalise();

    Ogre::Light* light = mSceneMgr->createLight("tstLight");
    light->setType( Ogre::Light::LT_DIRECTIONAL );
    light->setDirection( lightdir );
    light->setDiffuseColour( Ogre::ColourValue::White );
    light->setSpecularColour( Ogre::ColourValue(0.4, 0.4, 0.4) );

	// 环境光
    mSceneMgr->setAmbientLight( Ogre::ColourValue(0.5, 0.5, 0.5) );

	// Sky
    Ogre::Plane plane;
    plane.d = 100;
    plane.normal = Ogre::Vector3::NEGATIVE_UNIT_Y;
    mSceneMgr->setSkyPlane(true, plane, "Examples/CloudySky", 500, 20, true, 0.5, 150, 150);
}
//-------------------------------------------------------------------------------------
void GameMain::createCamera(void)
{
    // Create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");

	// create a default camera controller
    mCameraMan = new OgreBites::SdkCameraMan( mCamera ); 
	mCameraMan->setStyle( OgreBites::CS_MANUAL ); // 手动控制模式
}
//-------------------------------------------------------------------------------------
void GameMain::createViewports(void)
{
	mViewport = mWindow->addViewport( mCamera );
	mViewport->setBackgroundColour( ColourValue(1.0f,1.0f,1.0f) );

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio( Ogre::Real(mViewport->getActualWidth()) /
			Ogre::Real(mViewport->getActualHeight()) );
}
//-------------------------------------------------------------------------------------
void GameMain::createFrameListener(void)
{
    GameBase::createFrameListener();
}
//-------------------------------------------------------------------------------------
bool GameMain::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    bool ret = GameBase::frameRenderingQueued( evt );

	// 更新角色动画状态
	mPlayer->addTime( evt.timeSinceLastFrame );

    return ret;
}
// ---------------------------------------------------------------------
bool GameMain::keyPressed( const OIS::KeyEvent & evt )
{
	GameBase::keyPressed( evt );

	if ( ! mTrayMgr->isDialogVisible() ) mPlayer->injectKeyDown( evt );

	switch( evt.key )
	{
	case OIS::KC_ESCAPE:
		mShutDown = true;
		break;
	default:
		break;
	}

	return true;
}
// ---------------------------------------------------------------------
bool GameMain::keyReleased( const OIS::KeyEvent & evt )
{
	//GameBase::keyPressed( evt );

	if ( ! mTrayMgr->isDialogVisible() ) mPlayer->injectKeyUp(evt);
	return true;
}
// ---------------------------------------------------------------------
bool GameMain::mouseMoved( const OIS::MouseEvent & evt )
{
	if ( ! mTrayMgr->isDialogVisible() ) mPlayer->injectMouseMove( evt );
    return true;
}
// ---------------------------------------------------------------------
bool GameMain::mousePressed( const OIS::MouseEvent & evt, OIS::MouseButtonID id )
{
	if ( ! mTrayMgr->isDialogVisible() ) mPlayer->injectMouseDown( evt, id );
    return true;
}
// ---------------------------------------------------------------------
bool GameMain::mouseReleased( const OIS::MouseEvent & evt, OIS::MouseButtonID id )
{
    if (mTrayMgr->injectMouseUp(evt, id)) return true;
    mCameraMan->injectMouseUp(evt, id);
    return true;
}
// ---------------------------------------------------------------------
