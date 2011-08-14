#ifndef __GamePlayer_H__
#define __GamePlayer_H__

#include <OgreRoot.h>

//#include <OgreEntity.h>
//#include <OgreCamera.h>
//#include <OgreViewport.h>
//#include <OgreSceneManager.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

//#include <SdkTrays.h>
#include <SdkCameraMan.h>

#include "micropather.h" // A*寻径

using namespace Ogre;
using namespace micropather;

#define NUM_ANIMS 2           // number of animations the character has
#define CHAR_HEIGHT 0          // height of character's center of mass above ground
#define CAM_HEIGHT 1           // height of camera above character's center of mass
#define RUN_SPEED 5           // character running speed in units per second
#define WALK_SPEED 2           // character walking speed in units per second
#define TURN_SPEED 500.0f      // character turning in degrees per second
#define ANIM_FADE_SPEED 7.5f   // animation crossfade speed in % of full weight per second
//#define JUMP_ACCEL 30.0f       // character jump acceleration in upward units per squared second
//#define GRAVITY 90.0f          // gravity in downward units per squared second

class GamePlayer : public Graph
{
private:
	// all the animations our character has, and a null ID
	// some of these affect separate body parts and will be blended together
	enum AnimID
	{
		ANIM_WALK,
		ANIM_RUN,
		ANIM_NONE
	};

public:
	GamePlayer( Camera * cam, Ogre::SceneNode * pNode );
	~GamePlayer();

	void addTime( Real deltaTime );
	void injectKeyDown( const OIS::KeyEvent & evt );
	void injectKeyUp( const OIS::KeyEvent & evt );

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
	void injectMouseMove( const OIS::MultiTouchEvent & evt );
	void injectMouseDown( const OIS::MultiTouchEvent & evt );
#else
	void injectMouseMove( const OIS::MouseEvent & evt );
	void injectMouseDown( const OIS::MouseEvent & evt, OIS::MouseButtonID id );
#endif

private:
	void setupBody( SceneManager * sceneMgr );
	void setupAnimations();
	void setupCamera( Camera * cam );
	void updateBody( Real deltaTime );
	void updateAnimations( Real deltaTime );
	void fadeAnimations( Real deltaTime );
	void updateCamera( Real deltaTime );
	void updateCameraGoal( Real deltaYaw, Real deltaPitch, Real deltaZoom );
	void setBaseAnimation( AnimID id, bool reset = false );
	void setTopAnimation( AnimID id, bool reset = false );

	//-------------------------------
	// 三个micropather中的纯虚函数
	virtual float LeastCostEstimate( void * stateCurr, void * stateEnd );			// 获取当前节点到目标节点的距离
	virtual void AdjacentCost( void * state, std::vector< StateCost > * adjacent ); // 获取当前节点的相邻节点
	virtual void PrintStateInfo( void * state ); // *** Debug 输出

	void Node2XZ( void * pNode, float &x, float &z );
	void * XZ2Node( float x, float z );
	bool passable( const float x, const float z );	// 是否可通过
	void doAutoSearch(void);						// 进行自动搜索 | 主要

	MicroPather * pather;
	std::vector< void * > path;					// pather.Solve()后获得的路径向量
	int result;	// 搜索结果，0:SOLEVD 1:NO_SOLUTION 2:START_END_SAME

	bool mAutoPather;							// 是否开启自动寻径模式
	Ogre::SceneNode * mGoalNode;				// 目标节点
	//Ogre::SceneNode * mTestNode;				// \检测节点
	std::queue< Ogre::Vector3 > mPathPoints;	// 路径队列
	bool mAutoRunning;							// 是否处于自动运动状态
	Ogre::Vector3 mGoalPoint;					// 临时目标路径点
	//-------------------------------

	Camera * mCamera;
	SceneNode * mBodyNode;
	SceneNode * mCameraPivot;
	SceneNode * mCameraGoal;
	SceneNode * mCameraNode;
	Real mPivotPitch;
	Entity * mBodyEnt;
	AnimationState * mAnims[ NUM_ANIMS ];    // master animation list
	AnimID mBaseAnimID;                   // current base (full- or lower-body) animation
	//AnimID mTopAnimID;                    // current top (upper-body) animation
	bool mFadingIn[ NUM_ANIMS ];            // which animations are fading in
	bool mFadingOut[ NUM_ANIMS ];           // which animations are fading out
	Vector3 mKeyDirection;      // player's local intended direction based on WASD keys
	Vector3 mGoalDirection;     // actual intended direction in world-space
	bool mRun;					// 标记是否为跑步状态
	Real mVerticalVelocity;     // for jumping
	Real mTimer;                // general timer to see how long animations have been playing
};

#endif
