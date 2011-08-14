#include "GamePlayer.h"

GamePlayer::GamePlayer( Camera * cam, Ogre::SceneNode * pNode ) :
	pather(0),
	result(1),
	mAutoPather(false),
	mGoalNode( pNode )
{
	setupBody(cam->getSceneManager());
	setupCamera(cam);
	setupAnimations();

	pather = new MicroPather(this);
	//mTestNode = cam->getSceneManager()->getRootSceneNode()->createChildSceneNode();

	//doAutoSearch(); // 进行自动搜索
}

GamePlayer::~GamePlayer()
{
	delete pather;
}

void GamePlayer::addTime( Real deltaTime )
{
	updateBody( deltaTime );
	updateAnimations( deltaTime );
	updateCamera( deltaTime );
}

void GamePlayer::injectKeyDown( const OIS::KeyEvent & evt )
{
	// 切换自动|手动寻径模式
	if( evt.key == OIS::KC_TAB )
	{
		mAutoPather = ! mAutoPather;
		if( mAutoPather )
		{
			doAutoSearch(); // 进行自动搜索
			if( ! result ) setBaseAnimation( ANIM_RUN, true );
			else setBaseAnimation( ANIM_NONE, true );
		}
		else
		{
			result = 1;
			pather->Reset(); // 重置
			setBaseAnimation( ANIM_NONE, true );
		}
	}

	// keep track of the player's intended direction
	if		(evt.key == OIS::KC_W) mKeyDirection.z = -1;
	else if (evt.key == OIS::KC_A) mKeyDirection.x = -1;
	else if (evt.key == OIS::KC_S) mKeyDirection.z = 1;
	else if (evt.key == OIS::KC_D) mKeyDirection.x = 1;

	// 在移动的同时，按住左侧Shift键，变为Walk状态，默认为Run状态
	if( evt.key == OIS::KC_LSHIFT ) mRun = false;

	if( !mKeyDirection.isZeroLength() && mBaseAnimID == ANIM_NONE )
	{
		if( mRun )
			setBaseAnimation(ANIM_RUN, true);	// Run
		else
			setBaseAnimation(ANIM_WALK, true);	// Walk
	}

}

void GamePlayer::injectKeyUp( const OIS::KeyEvent & evt )
{
	// keep track of the player's intended direction
	if		(evt.key == OIS::KC_W && mKeyDirection.z == -1) mKeyDirection.z = 0;
	else if (evt.key == OIS::KC_A && mKeyDirection.x == -1) mKeyDirection.x = 0;
	else if (evt.key == OIS::KC_S && mKeyDirection.z == 1) mKeyDirection.z = 0;
	else if (evt.key == OIS::KC_D && mKeyDirection.x == 1) mKeyDirection.x = 0;

	if( evt.key == OIS::KC_LSHIFT ) mRun = true;

	if( !mAutoPather && mKeyDirection.isZeroLength() && (mBaseAnimID == ANIM_WALK || mBaseAnimID == ANIM_RUN) )
	{
		setBaseAnimation( ANIM_NONE );
	}
}

void GamePlayer::injectMouseMove( const OIS::MouseEvent & evt )
{
	// update camera goal based on mouse movement
	updateCameraGoal(-0.05f * evt.state.X.rel, -0.05f * evt.state.Y.rel, -0.0005f * evt.state.Z.rel);
}

void GamePlayer::injectMouseDown( const OIS::MouseEvent & evt, OIS::MouseButtonID id )
{
}

void GamePlayer::setupBody( SceneManager * sceneMgr )
{
	// create main model
	mBodyNode = sceneMgr->getRootSceneNode()->createChildSceneNode(Vector3::UNIT_Y * CHAR_HEIGHT);
	mBodyEnt = sceneMgr->createEntity("MaxPayenBody", "maxpayen.mesh");
	mBodyNode->attachObject( mBodyEnt );
	mBodyNode->setPosition( Vector3::ZERO );

	mKeyDirection = Vector3::ZERO;
	mVerticalVelocity = 0;
	mRun = true;
}

void GamePlayer::setupAnimations()
{
	// this is very important due to the nature of the exported animations
	mBodyEnt->getSkeleton()->setBlendMode(ANIMBLEND_CUMULATIVE);

	String animNames[] = { "Walk", "Run" };

	// populate our animation list
	for (int i = 0; i < NUM_ANIMS; i++)
	{
		mAnims[i] = mBodyEnt->getAnimationState( animNames[i] );
		mAnims[i]->setLoop(true);
		mFadingIn[i] = false;
		mFadingOut[i] = false;
	}

	// start off in the idle state (top and bottom together)
	setBaseAnimation( ANIM_NONE );
	//mAnims[ANIM_WALK]->setEnabled(true);

	// relax the hands since we're not holding anything
	//mAnims[ANIM_HANDS_RELAXED]->setEnabled(true);
}

void GamePlayer::setupCamera( Camera * cam )
{
	// create a pivot at roughly the character's shoulder
	mCameraPivot = cam->getSceneManager()->getRootSceneNode()->createChildSceneNode();
	// this is where the camera should be soon, and it spins around the pivot
	mCameraGoal = mCameraPivot->createChildSceneNode(Vector3(0, 0, 5));
	// this is where the camera actually is
	mCameraNode = cam->getSceneManager()->getRootSceneNode()->createChildSceneNode();
	mCameraNode->setPosition(mCameraPivot->getPosition() + mCameraGoal->getPosition());

	mCameraPivot->setFixedYawAxis(true);
	mCameraGoal->setFixedYawAxis(true);
	mCameraNode->setFixedYawAxis(true);

	// our model is quite small, so reduce the clipping planes
	cam->setNearClipDistance(0.1);
	cam->setFarClipDistance(100);
	mCameraNode->attachObject(cam);

	mPivotPitch = 0;
}

void GamePlayer::updateBody( Real deltaTime )
{
	//mGoalDirection = Vector3::ZERO;   // we will calculate this

	//if( mKeyDirection != Vector3::ZERO || mAutoPather )
	if( mKeyDirection != Vector3::ZERO || ! result )
	{
		// 自动寻径模式下目标方向从获得的路径向量队列中获取
		//if( mAutoPather )
		if( ! result )
		{
			// 自动寻找下一个路径点
			if( mAutoRunning )
			{
				if(
					abs( mBodyNode->getPosition().x - mGoalPoint.x ) < 0.05f
				 && abs( mBodyNode->getPosition().z - mGoalPoint.z ) < 0.05f
				 )
				{
					// 如果非常接近目标路径点
					// 则需要通过下一个路径点改变运动方向
					mAutoRunning = false;
				}

			}
			// 如果到达指定路径点，而路径队列非空，继续下一个路径点
			else if( ! mPathPoints.empty() )
			{
				mGoalDirection = mPathPoints.front() - mBodyNode->getPosition();
				mGoalPoint = mPathPoints.front();	// 存储临时目标路径点
				mPathPoints.pop();					// 从路径点队列中删除该点
				mAutoRunning = true;				// 设为运动状态
			}
			else
			{
				result = 1;
				mAutoPather = false;
				mGoalDirection = Ogre::Vector3::ZERO;
				//setBaseAnimation( ANIM_NONE, true );
			}
		}
		// 非自动寻径模式下目标方向由键盘方向输入合成
		else
		{
			mGoalDirection = Vector3::ZERO;   // we will calculate this
			// calculate actually goal direction in world based on player's key directions
			mGoalDirection += mKeyDirection.z * mCameraNode->getOrientation().zAxis();
			mGoalDirection += mKeyDirection.x * mCameraNode->getOrientation().xAxis();
		}
		mGoalDirection.y = 0;
		mGoalDirection.normalise();

		Quaternion toGoal = mBodyNode->getOrientation().zAxis().getRotationTo(mGoalDirection);

		// calculate how much the character has to turn to face goal direction
		Real yawToGoal = toGoal.getYaw().valueDegrees();
		// this is how much the character CAN turn this frame
		Real yawAtSpeed = yawToGoal / Math::Abs(yawToGoal) * deltaTime * TURN_SPEED;
		// reduce "turnability" if we're in midair
		//if (mBaseAnimID == ANIM_JUMP_LOOP) yawAtSpeed *= 0.2f;

		// turn as much as we can, but not more than we need to
		if (yawToGoal < 0) yawToGoal = std::min<Real>(0, std::max<Real>(yawToGoal, yawAtSpeed)); //yawToGoal = Math::Clamp<Real>(yawToGoal, yawAtSpeed, 0);
		else if (yawToGoal > 0) yawToGoal = std::max<Real>(0, std::min<Real>(yawToGoal, yawAtSpeed)); //yawToGoal = Math::Clamp<Real>(yawToGoal, 0, yawAtSpeed);
			
		mBodyNode->yaw( Degree(yawToGoal) );

		// move in current body direction (not the goal direction)
		if( mRun )
			mBodyNode->translate(0, 0, deltaTime * RUN_SPEED * mAnims[mBaseAnimID]->getWeight(), Node::TS_LOCAL);
		else
			mBodyNode->translate(0, 0, deltaTime * WALK_SPEED * mAnims[mBaseAnimID]->getWeight(), Node::TS_LOCAL);

	}

}

void GamePlayer::updateAnimations( Real deltaTime )
{
	Real baseAnimSpeed = 1;
	//Real topAnimSpeed = 1;

	mTimer += deltaTime;

	// increment the current base and top animation times
	if( mBaseAnimID != ANIM_NONE ) mAnims[mBaseAnimID]->addTime( deltaTime * baseAnimSpeed );
	if( mKeyDirection.isZeroLength() && ! mAutoPather ) setBaseAnimation( ANIM_NONE, true );
	//if (mTopAnimID != ANIM_NONE) mAnims[mTopAnimID]->addTime(deltaTime * topAnimSpeed);

	// apply smooth transitioning between our animations
	fadeAnimations(deltaTime);
}

void GamePlayer::fadeAnimations( Real deltaTime )
{
	for (int i = 0; i < NUM_ANIMS; i++)
	{
		if (mFadingIn[i])
		{
			// slowly fade this animation in until it has full weight
			Real newWeight = mAnims[i]->getWeight() + deltaTime * ANIM_FADE_SPEED;
			mAnims[i]->setWeight(Math::Clamp<Real>(newWeight, 0, 1));
			if (newWeight >= 1) mFadingIn[i] = false;
		}
		else if (mFadingOut[i])
		{
			// slowly fade this animation out until it has no weight, and then disable it
			Real newWeight = mAnims[i]->getWeight() - deltaTime * ANIM_FADE_SPEED;
			mAnims[i]->setWeight(Math::Clamp<Real>(newWeight, 0, 1));
			if (newWeight <= 0)
			{
				mAnims[i]->setEnabled(false);
				mFadingOut[i] = false;
			}
		}
	}
}

void GamePlayer::updateCamera( Real deltaTime )
{
	// place the camera pivot roughly at the character's shoulder
	mCameraPivot->setPosition(mBodyNode->getPosition() + Vector3::UNIT_Y * CAM_HEIGHT);
	// move the camera smoothly to the goal
	Vector3 goalOffset = mCameraGoal->_getDerivedPosition() - mCameraNode->getPosition();
	mCameraNode->translate(goalOffset * deltaTime * 9.0f);
	// always look at the pivot
	mCameraNode->lookAt(mCameraPivot->_getDerivedPosition(), Node::TS_WORLD);
}

void GamePlayer::updateCameraGoal( Real deltaYaw, Real deltaPitch, Real deltaZoom )
{
	mCameraPivot->yaw(Degree(deltaYaw), Node::TS_WORLD);

	// bound the pitch
	if (!(mPivotPitch + deltaPitch > 25 && deltaPitch > 0) &&
		!(mPivotPitch + deltaPitch < -60 && deltaPitch < 0))
	{
		mCameraPivot->pitch(Degree(deltaPitch), Node::TS_LOCAL);
		mPivotPitch += deltaPitch;
	}
		
	Real dist = mCameraGoal->_getDerivedPosition().distance(mCameraPivot->_getDerivedPosition());
	Real distChange = deltaZoom * dist;

	// bound the zoom
	if (!(dist + distChange < 2 && distChange < 0) &&
		!(dist + distChange > 25 && distChange > 0))
	{
		mCameraGoal->translate(0, 0, distChange, Node::TS_LOCAL);
	}
}

void GamePlayer::setBaseAnimation( AnimID id, bool reset /*= false*/ )
{
	if (mBaseAnimID >= 0 && mBaseAnimID < NUM_ANIMS)
	{
		// if we have an old animation, fade it out
		mFadingIn[mBaseAnimID] = false;
		mFadingOut[mBaseAnimID] = true;
	}

	mBaseAnimID = id;

	if (id != ANIM_NONE)
	{
		// if we have a new animation, enable it and fade it in
		mAnims[id]->setEnabled(true);
		mAnims[id]->setWeight(0);
		mFadingOut[id] = false;
		mFadingIn[id] = true;
		if (reset) mAnims[id]->setTimePosition(0);
	}
}

void GamePlayer::setTopAnimation( AnimID id, bool reset /*= false*/ )
{
	/*if (mTopAnimID >= 0 && mTopAnimID < NUM_ANIMS)
	{
		// if we have an old animation, fade it out
		mFadingIn[mTopAnimID] = false;
		mFadingOut[mTopAnimID] = true;
	}

	mTopAnimID = id;

	if (id != ANIM_NONE)
	{
		// if we have a new animation, enable it and fade it in
		mAnims[id]->setEnabled(true);
		mAnims[id]->setWeight(0);
		mFadingOut[id] = false;
		mFadingIn[id] = true;
		if (reset) mAnims[id]->setTimePosition(0);
	}*/
}

//-----------------------------------------------------------------------------
// 100100 前三位为z值，后三位为x值
// 此外由于地图(0,0)点在中心，所以还需要做50的偏移
void GamePlayer::Node2XZ( void * pNode, float &x, float &z )
{
	int index = (int)pNode;
	z = index / 1000 - 50;
	x = index % 1000 - 50;
}
//-----------------------------------------------------------------------------
void * GamePlayer::XZ2Node( float x, float z )
{
	int tmp =  (int)( z + 50 ) * 1000 + (int)( x + 50 );
	return (void *)tmp;
}
//-----------------------------------------------------------------------------
// 当前节点(非原始节点)到目标节点的最短距离
float GamePlayer::LeastCostEstimate( void * stateCurr, void * stateEnd )
{
	float xCurr, zCurr, xEnd, zEnd;
	Node2XZ( stateCurr, xCurr, zCurr );
	Node2XZ( stateEnd, xEnd, zEnd );

	float dx = xCurr - xEnd;
	float dz = zCurr - zEnd;

	//std::cout << "SQUT:  " << (float) sqrt( (double)(dx*dx) + (double)(dz*dz) ) << std::endl;
	return (float) sqrt( (double)(dx*dx) + (double)(dz*dz) );
}
//-----------------------------------------------------------------------------
// 计算相邻节点代价
void GamePlayer::AdjacentCost( void * state, std::vector< StateCost > * adjacent )
{
	const float dx[8] = { 1, 1, 0, -1, -1, -1,  0,  1 };
	const float dz[8] = { 0, 1, 1,  1,  0, -1, -1, -1 };
	// 上下左右代价为1，斜线方向代价为1.41(根号2)
	const float cost[8] = { 1.0f, 1.41f, 1.0f, 1.41f, 1.0f, 1.41f, 1.0f, 1.41f };
	//Ogre::Vector3 currPos; // 当前节点(非原始节点)
	float xCurr, zCurr;
	Node2XZ( state, xCurr, zCurr );
	//std::cout << currPos << std::endl;
	//std::cout << xCurr << " --- " << zCurr << std::endl;

	for( int i = 0; i < 8; ++i )
	{
		float currX = xCurr + dx[i];
		float currZ = zCurr + dz[i];

		//mTestNode->setPosition( currX, 0, currZ );
		//std::cout << mTestNode->getPosition() << " | "<< cost[i] << std::endl;
		//std::cout << Ogre::Vector3(currX,0,currZ) << std::endl;

		if( passable( currX, currZ ) )
		{
			StateCost nodeCost = { XZ2Node( currX, currZ ), cost[i] };
			adjacent->push_back( nodeCost );
		}
	}
}
//-----------------------------------------------------------------------------
void GamePlayer::PrintStateInfo( void * state )
{
	float x, z;
	Node2XZ( state, x, z );
	std::cout << Ogre::Vector3(x, 0, z);
}
//-----------------------------------------------------------------------------
bool GamePlayer::passable( const float x, const float z )
{
	if( x >= -100 && x <= 100 && z >= -100 && z <= 100 )
		return true;
	return false;
}
//-----------------------------------------------------------------------------
void GamePlayer::doAutoSearch(void)
{
	float totalCost;

	// result: 0,SOLVED; 1,NO_SOLUTION; 2,START_END_SAME
	//result = pather->Solve( (void*)mBodyNode, (void*)mGoalNode, &path, &totalCost );
	result = pather->Solve(
			XZ2Node( mBodyNode->getPosition().x, mBodyNode->getPosition().z ),
			XZ2Node( mGoalNode->getPosition().x, mGoalNode->getPosition().z ),
			&path, &totalCost
			);

	std::cout << "\n---------------------------------\n";
	//std::cout << mGoalNode->getPosition().x << std::endl;
	//std::cout << result << std::endl;

	// 清空当前路径点队列
	while( ! mPathPoints.empty() ) { mPathPoints.pop(); }
	unsigned size = path.size();
	float x, z;
	Ogre::Vector3 pathPoint;
	// 从path中获取路径点
	for( int i = 0; i < size; ++i )
	{
		Node2XZ( path[i], x, z );
		pathPoint = Ogre::Vector3( x, 0, z );
		std::cout << pathPoint << std::endl; // *** Debug 输出路径点
		mPathPoints.push( pathPoint );
	}
}
//-----------------------------------------------------------------------------
