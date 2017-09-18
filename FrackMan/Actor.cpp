#include "Actor.h"
#include "StudentWorld.h"
#include <random>

using namespace std;

// Return a random int from min to max, inclusive

int randIntAc(int min, int max)
{
	if (max < min)
		swap(max, min);
	static random_device rd;
	static mt19937 generator(rd());
	uniform_int_distribution<> distro(min, max);
	return distro(generator);
}

//////////////////////////////////////////////////////////////////////////////////////
// ACTOR CLASS extending GraphObject
//////////////////////////////////////////////////////////////////////////////////////

Actor::Actor(StudentWorld * world, int startX, int startY, Direction startDir, /*bool visible,*/ int imageID, double size, int depth)
	:GraphObject(imageID, startX, startY, startDir, size, depth)
{
	m_world = world;
	m_alive = true;
}

bool Actor::isAlive() const
{
	return m_alive;
}

void Actor::setDead()
{
	m_alive = false;
}

bool Actor::annoy(unsigned int amt)
{
	return false;
}

StudentWorld * Actor::getWorld() const
{
	return m_world;
}

bool Actor::canActorsPassThroughMe() const
{
	return true;
}

bool Actor::canDigThroughDirt() const
{
	return false;
}

bool Actor::canPickThingsUp() const
{
	return false;
}

bool Actor::huntsFrackMan() const
{
	return false;
}

bool Actor::needsToBePickedUpToFinishLevel() const
{
	return false;
}

bool Actor::moveToIfPossible(int x, int y)
{
	//Check world to ensure moving possibility
	if (m_world->canActorMoveTo(this, x, y)) {
		moveTo(x, y);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////
// AGENT CLASS extending Actor
//////////////////////////////////////////////////////////////////////////////////////

Agent::Agent(StudentWorld * world, int startX, int startY, Direction startDir, int imageID, unsigned int hitPoints, int depth)
	:Actor(world, startX, startY, startDir, imageID, SIZE_AG, depth)
{
	m_hp = hitPoints;
}

unsigned int Agent::getHitPoints() const
{
	return m_hp;
}

// Returns true if damage kills actor, false otherwise
bool Agent::annoy(unsigned int amount)
{
	//Apply damage, and if agent reaches hp of 0 or less, set it do dead
	m_hp -= amount;
	if (m_hp <= 0) {
		if (canDigThroughDirt()) { //If FrackMan dies
			setDead();
			getWorld()->playSound(SOUND_PLAYER_GIVE_UP);
		}
		else if (huntsFrackMan()) { //If protester dies
			getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
		}
		return true;
	}
	return false;
}

bool Agent::canPickThingsUp() const
{
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////
// FRACKMAN CLASS extending Agent
//////////////////////////////////////////////////////////////////////////////////////

FrackMan::FrackMan(StudentWorld * world, int startX, int startY)
	:Agent(world, startX, startY, DEFAULT_DIR_FM, IID_PLAYER, DEFAULT_FM_HP, DEPTH_FM)
{
	m_gold = DEFAULT_GOLD;
	m_sonar = DEFAULT_SONAR;
	m_water = DEFAULT_SQUIRTS;
	setVisible(true);
}

void FrackMan::move()
{
	int key;
	if (getWorld()->getKey(key)) { //If keystroke detected
		switch (key)
		{
		case KEY_PRESS_ESCAPE:
			setDead();
			break;
		case KEY_PRESS_UP:
			if (getDirection() != up) {
				setDirection(up);
			}
			else {
				//Check if new position will be in bounds
				int newX = getX();
				int newY = getY() + 1;
				digMoveTo(newX, newY);
			}
			break;
		case KEY_PRESS_DOWN:
			if (getDirection() != down) {
				setDirection(down);
			}
			else {
				//Check if new position will be in bounds
				int newX = getX();
				int newY = getY() - 1;
				digMoveTo(newX, newY);
			}
			break;
		case KEY_PRESS_RIGHT:
			if (getDirection() != right) {
				setDirection(right);
			}
			else {
				//Check if new position will be in bounds
				int newX = getX() + 1;
				int newY = getY();
				digMoveTo(newX, newY);
			}
			break;
		case KEY_PRESS_LEFT:
			if (getDirection() != left) {
				setDirection(left);
			}
			else {
				//Check if new position will be in bounds
				int newX = getX() - 1;
				int newY = getY();
				digMoveTo(newX, newY);
			}
			break;
		case KEY_PRESS_SPACE: //Squirt water
			//Check to see if FM has any squirts
			if (m_water > 0) {
				getWorld()->playSound(SOUND_PLAYER_SQUIRT);
				m_water--;
				int newX = getX();
				int newY = getY();
				switch(getDirection()) {
				case up:
					newY += SPRITE_HEIGHT;
					break;
				case down:
					newY -= SPRITE_HEIGHT;
					break;
				case right:
					newX += SPRITE_WIDTH;
					break;
				case left:
					newX -= SPRITE_WIDTH;
				}
				getWorld()->addActor(new Squirt(getWorld(), newX, newY, getDirection()));
			}
			break;
		case'z':
		case'Z': //Use sonar
			if (m_sonar > 0) {
				m_sonar--;
				getWorld()->revealAllNearbyObjects(getX(), getY(), 12);
			}
			break;
		case KEY_PRESS_TAB: //Drop gold
			if (m_gold > 0) {
				m_gold--;
				getWorld()->addActor(new GoldNugget(getWorld(), getX(), getY(), true)); //Create TEMPORARY GOLD
			}
			break;
		default:
			moveTo(getX(), getY());
		}
	}
}

void FrackMan::addGold()
{
	m_gold++;
}

bool FrackMan::canDigThroughDirt() const
{
	return true;
}

void FrackMan::addSonar()
{
	m_sonar++;
}

void FrackMan::addWater()
{
	m_water += 5;
}

unsigned int FrackMan::getGold() const
{
	return m_gold;
}

unsigned int FrackMan::getSonar() const
{
	return m_sonar;
}

unsigned int FrackMan::getWater() const
{
	return m_water;
}

void FrackMan::digMoveTo(int x, int y)
{
	if (getWorld()->canActorMoveTo(this, x, y)) {
		getWorld()->clearDirt(x, y);
		moveTo(x, y);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
// DIRT CLASS extending Actor
//////////////////////////////////////////////////////////////////////////////////////

Dirt::Dirt(StudentWorld * world, int startX, int startY)
	:Actor(world, startX, startY, DIR_DIRT, IID_DIRT, SIZE_DIRT, DEPTH_DIRT)
{
	setVisible(true);
}

void Dirt::move()
{
	//Just don't get dug god dammit!
}

//////////////////////////////////////////////////////////////////////////////////////
// BOULDER CLASS extending Actor
//////////////////////////////////////////////////////////////////////////////////////

Boulder::Boulder(StudentWorld * world, int startX, int startY)
	:Actor(world, startX, startY, DIR_BOULDER, IID_BOULDER, SIZE_BOULDER, DEPTH_BOULDER)
{
	m_state = stable;
	m_ticksWaiting = 0;
	getWorld()->clearDirt(startX, startY); //Clear dirt around
	setVisible(true);
}

void Boulder::move()
{
	if (!isAlive()) {
		return;
	}
	switch (m_state) {
	case stable:
		//Check if there's any dirt below. If so, change the state of boulder
		if (getWorld()->canActorMoveTo(this, getX(), getY() - 1)) {
			m_state = waiting;
		}
		break;
	case waiting:
		//Count appropriate number of ticks until the boulder can move
		if (m_ticksWaiting < TICKS_WAIT_BOULDER) {
			m_ticksWaiting++;
		}
		else {
			m_state = falling;
			getWorld()->playSound(SOUND_FALLING_ROCK);
		}
		break;
	case falling:
		//Keep falling and killing until hitting an obstruction
		if (getWorld()->canActorMoveTo(this, getX(), getY() - 1)) {
			moveTo(getX(), getY() - 1);
			getWorld()->annoyAllNearbyActors(this, DAMAGE_BOULDER, RADIUS_BOULDER);
		}
		else {
			setDead();
		}
		break;
	}
}

bool Boulder::canActorsPassThroughMe() const
{
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////
// SQUIRT CLASS extending Actor
//////////////////////////////////////////////////////////////////////////////////////

Squirt::Squirt(StudentWorld * world, int startX, int startY, Direction startDir)
	:Actor(world, startX, startY, startDir, IID_WATER_SPURT, SIZE_SQUIRT, DEPTH_SQUIRT)
{
	//Check if placed in a valid position
	if (!getWorld()->canActorMoveTo(this, startX, startY)) {
		setDead();
		return;
	}
	m_age = 0;
	setVisible(true);
}

void Squirt::move()
{
	//Act only if alive
	if (isAlive()) {
		int newX = getX();
		int newY = getY();
		//Find potential new coordinates based on direction
		switch (getDirection()) {
		case up:
			newY++;
			break;
		case down:
			newY--;
			break;
		case right:
			newX++;
			break;
		case left:
			newX--;
			break;
		}
		//If there's no obstruction and lifespan has not been spent, keep going. Otherwise kill squirt
		if (getWorld()->canActorMoveTo(this, newX, newY) && m_age < AGE_SQUIRT) {
			moveTo(newX, newY);
			m_age++;
			if (0 < getWorld()->annoyAllNearbyActors(this, DAMAGE_SQUIRT, RADIUS_SQUIRT)) {
				setDead();
			}
		
		}
		else {
			setDead();
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////
// ACTIVATINGOBJECT CLASS extending Actor
//////////////////////////////////////////////////////////////////////////////////////

ActivatingObject::ActivatingObject(StudentWorld * world, int startX, int startY, int imageID, int soundToPlay, bool activateOnPlayer, bool activateOnProtester, bool initallyActive)
	:Actor(world, startX, startY, DIR_AO, imageID, SIZE_AO, DEPTH_AO)
{
	m_sound = soundToPlay;
	m_playerObj = activateOnPlayer;
	m_protesterObj = activateOnProtester;
	m_expired = false;
	m_isActive = false;
	m_tickWaiting = -1; //Will change when setTicksToLive() is called
	if (initallyActive) {
		setTicksToLive();
	}
}

void ActivatingObject::move()
{
	//Do nothing if dead
	if (!isAlive()) {
		return;
	}
	//If there's a FrackMan nearby become visible
	Actor* fm = getWorld()->findNearbyFrackMan(this, VISIBILITY_AO);
	if (fm != nullptr && !isVisible()) {
		setVisible(true);
		return;
	}
	
	//Check if activating object can be picked up by player
	if (m_playerObj) {
		fm = getWorld()->findNearbyFrackMan(this, RADIUS_AO);
		if (fm != nullptr) {
			setDead();
			//Implies oil barrel
			if (needsToBePickedUpToFinishLevel()) {
				getWorld()->reduceOilLeft();
			}
			getWorld()->playSound(m_sound);
		}
	}
	//Check if activating object can be picked up by a protester
	else if(m_protesterObj){
		Protester* protester = (Protester*)getWorld()->findNearbyProtester(this, RADIUS_AO);
		if (protester != nullptr) {
			setDead();
			getWorld()->playSound(m_sound);
			protester->addGold();
		}
	}

	//Reduce ticks if object is active
	if (m_isActive) {
		m_tickWaiting--; //Decrement until hitting zero, thus killing the object
		if (m_tickWaiting <= 0) {
			setDead();
			m_expired = true;
		}
	}
}

void ActivatingObject::setTicksToLive()
{
	m_isActive = true;
	//Assume it's a dropped gold that always has 100 waiting time
	if (m_protesterObj) {
		m_tickWaiting = TICKS_WAIT_GOLD;
	}
	//All objects picked up by player have the same wait time, determined by the level
	if (m_playerObj) {
		int ticksLvl = 300 - (10 * getWorld()->getLevel());
		m_tickWaiting = ticksLvl > TICKS_WAIT_MIN_AO ? ticksLvl : TICKS_WAIT_MIN_AO;
	}
}

bool ActivatingObject::isExpired() const
{
	return m_expired;
}

//////////////////////////////////////////////////////////////////////////////////////
// OILBARREL CLASS extending ActivatingObject
//////////////////////////////////////////////////////////////////////////////////////

OilBarrel::OilBarrel(StudentWorld * world, int startX, int startY)
	:ActivatingObject(world, startX, startY, IID_BARREL, SOUND_FOUND_OIL, true, false, false)
	//Will activate only on player 
{
}

void OilBarrel::move()
{
	ActivatingObject::move(); //Call parent move and do a few things of its own
	//Can only be removed by being picked up and we can ensure that player will get points when barrel goes away
	if (!isAlive()) {
		getWorld()->increaseScore(SCORE_BARREL);
	}
}

bool OilBarrel::needsToBePickedUpToFinishLevel() const
{
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////
// GOLDNUGGET CLASS extending ActivatingObject
//////////////////////////////////////////////////////////////////////////////////////

GoldNugget::GoldNugget(StudentWorld * world, int startX, int startY, bool temporary)
	:ActivatingObject(world, startX, startY, IID_GOLD, temporary ? SOUND_PROTESTER_FOUND_GOLD : SOUND_GOT_GOODIE, !temporary, temporary, temporary)
	//Only temporary gold can be picked up by the protestor, and cannot be picked up by player
{
	m_isTemporary = temporary;
	if (m_isTemporary) { //Implies a nugget that was dropped by player
		setVisible(true);
	}
}

void GoldNugget::move()
{
	ActivatingObject::move();
	//If pickupable by player and is dead, then we can ensure that player will get points
	if (!m_isTemporary && !isAlive()) {
		getWorld()->increaseScore(SCORE_GOLD_PLAYER);
		getWorld()->giveFrackManGold();
		return;
	}
	//If pickupable by protestor and is dead, we must first make sure that it did not expire before granting points
	Protester* potentialProt = (Protester*)getWorld()->findNearbyProtester(this, 4);
	if (potentialProt != nullptr && m_isTemporary && !isAlive() && !isExpired()) {
		potentialProt->addGold();
		return;
	}
}

//////////////////////////////////////////////////////////////////////////////////////
// SONARKIT CLASS extending ActivatingObject
//////////////////////////////////////////////////////////////////////////////////////

SonarKit::SonarKit(StudentWorld * world, int startX, int startY)
	:ActivatingObject(world, startX, startY, IID_SONAR, SOUND_GOT_GOODIE, true, false, true)
{
	setVisible(true);
}

void SonarKit::move()
{
	ActivatingObject::move();
	if (!isAlive()) {
		if (isExpired()) {
			return; //Do nothing because it was not picked up by the player
		}
		//Assumes picked up by player
		getWorld()->increaseScore(SCORE_SONAR);
		getWorld()->giveFrackManSonar();
	}
}

//////////////////////////////////////////////////////////////////////////////////////
// WATERPOOL CLASS extending ActivatingObject
//////////////////////////////////////////////////////////////////////////////////////

WaterPool::WaterPool(StudentWorld * world, int startX, int startY)
	:ActivatingObject(world, startX, startY, IID_WATER_POOL, SOUND_GOT_GOODIE, true, false, true)
{
	setVisible(true);
}

void WaterPool::move()
{
	ActivatingObject::move();
	if (!isAlive()) {
		if (isExpired()) {
			return; //Do nothing because it was not picked up by the player
		}
		//Assumes picked up by player
		getWorld()->increaseScore(SCORE_WATER);
		getWorld()->giveFrackManWater();
	}
}

//////////////////////////////////////////////////////////////////////////////////////
// PROTESTER CLASS extending Agent
//////////////////////////////////////////////////////////////////////////////////////

Protester::Protester(StudentWorld * world, int startX, int startY, int imageID, unsigned int hitPoints)
	:Agent(world, startX, startY, DEFAULT_DIR_PR, imageID, hitPoints, DEPTH_PR)
{
	squaresToMoveCurrentDir = randIntAc(8, 60);
	leaving = false;
	ticksToWait = 3 - (getWorld()->getLevel() / 4) > 0 ? 3 - (getWorld()->getLevel() / 4) : 0; //Set maximum
	ticksRested = 0;
	ticksRefracting = REFRACTORY_PR;
	ticksPerp = 0;
	shouldContinue = false;
	setVisible(true);
}

void Protester::move()
{
	//Check if alive
	if (!isAlive()) {
		shouldContinue = false;
		return;
	}
	//Check to see if in rest state
	if (/*!leaving && */ticksRested < ticksToWait) {
		shouldContinue = false;
		ticksRested++;
		return;
	}
	ticksRested = 0;
	
	//Adjust ticks refracting from yell
	if (!leaving && ticksRefracting < REFRACTORY_PR) {
		ticksRefracting++;
	}

	//Adjust ticks from making perp turn
	ticksPerp++;

	//If they are leaving the field
	if (leaving) {
		if (getX() == 60 && getY() == 60) {
			setDead();
		}
		else {
			switch (getWorld()->determineFirstMoveToExit(getX(), getY()))
			{
			case up:
				setDirection(up);
				moveTo(getX(), getY() + 1);
				break;
			case right:
				setDirection(right);
				moveTo(getX() + 1, getY());
				break;
			case down:
				setDirection(down);
				moveTo(getX(), getY() - 1);
				break;
			case left:
				setDirection(left);
				moveTo(getX() - 1, getY());
				break;
			default:
				moveTo(getX(), getY());
				break;
			}
		}
		shouldContinue = false;
		return;
	}

	// Try to hurt Frackman
	else if (getWorld()->isNearFrackMan(this, YELLING_RADIUS_PR) && getWorld()->facingTowardFrackMan(this)) {
		if (ticksRefracting == REFRACTORY_PR) {
			getWorld()->playSound(SOUND_PROTESTER_YELL);
			getWorld()->annoyFrackMan();
			ticksRefracting = 0;
			shouldContinue = false;
			return;
		}
	}
	shouldContinue = true;
	//Call movePostSpecialization() after specialization has been handled by subclass
}

void Protester::movePostSpecialization()
{
	if (shouldContinue) {
		//Check for FrackMan in line of sight
		GraphObject::Direction lineOfSight = getWorld()->lineOfSightToFrackMan(this);
		if (!getWorld()->isNearFrackMan(this, YELLING_RADIUS_PR) && lineOfSight != GraphObject::none) {
			if (getDirection() != lineOfSight) {
				squaresToMoveCurrentDir = 0;
			}
			setDirection(lineOfSight);
			moveInDirection(getDirection());
			return;
		}

		//If we're here it means the protester can't see FrackMan
		squaresToMoveCurrentDir--;
		if (squaresToMoveCurrentDir <= 0) {
			int newX = getX(), newY = getY();
			GraphObject::Direction newDir;
			//Select new random direction (valid)
			do {
				int dirNum = randIntAc(1, 4);

				switch (dirNum)
				{
				case 1:
					newDir = GraphObject::up;
					newY++;
					break;
				case 2:
					newDir = GraphObject::right;
					newX++;
					break;
				case 3:
					newDir = GraphObject::down;
					newY--;
					break;
				case 4:
					newDir = GraphObject::left;
					newX--;
					break;
				}
			} while (!getWorld()->canActorMoveTo(newX, newY));
			setDirection(newDir);
			squaresToMoveCurrentDir = randIntAc(8, 60);
		}

		//Check to see if in intersection and make a turn if possible
		GraphObject::Direction perpDir = perpDirAvailable();
		if (ticksPerp >= 200 && perpDir != GraphObject::none) {
			setDirection(perpDir);
			squaresToMoveCurrentDir = randIntAc(8, 60);
			ticksPerp = 0;
		}

		//Continue in direction
		moveInDirection(getDirection());
	}
}

bool Protester::shouldCont() const
{
	return shouldContinue;
}

// Return true if Protester dies my annoyance
bool Protester::annoy(unsigned int amount)
{
	if (!leaving) {
		if (!Agent::annoy(amount)) { //Call superclass's annoy and check if it killed
			//If not killed...
			getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
			ticksRested -= 50 > 100 - (getWorld()->getLevel() * 10) ? 50 : 100 - (getWorld()->getLevel() * 10); //Reduce ticks rested by max of the two
		}
		else {
			//If killed
			leaving = true;
			ticksRested/*ToWait*/ = 0;
			//Check what killed it
			Actor* potentialKiller = getWorld()->findNearbyItem(this, 4);
			if (potentialKiller != nullptr) {
				//Implies boulder
				if (!potentialKiller->canActorsPassThroughMe()) {
					getWorld()->increaseScore(500);
				}
				//Implies squirt
				else {
					getWorld()->increaseScore(100);
				}
			}
			return true;
		}
	}
	return false;
}

bool Protester::huntsFrackMan() const
{
	return true;
}

void Protester::setMustLeaveOilField()
{
	leaving = true;
}

void Protester::setTicksToNextMove(int amount)
{
	ticksRested -= amount;
}

GraphObject::Direction Protester::perpDirAvailable() const
{
	if (getDirection() == GraphObject::up || getDirection() == GraphObject::down) {
		bool leftAvailable = getWorld()->canActorMoveTo(getX() - 1, getY());
		bool rightAvailable = getWorld()->canActorMoveTo(getX() + 1, getY());
		//Both available
		if (leftAvailable && rightAvailable) {
			switch (randIntAc(1, 2)) {
			case 1:
				return GraphObject::left;
				break;
			case 2:
				return GraphObject::right;
				break;
			}
		}
		else if (leftAvailable) {
			return GraphObject::left;
		}
		else if (rightAvailable) {
			return GraphObject::right;
		}
	}
	else if (getDirection() == GraphObject::up || getDirection() == GraphObject::down) {
		bool upAvailable = getWorld()->canActorMoveTo(getX(), getY() + 1);
		bool downAvailable = getWorld()->canActorMoveTo(getX(), getY() - 1);
		//Both available
		if (upAvailable && downAvailable) {
			switch (randIntAc(1, 2)) {
			case 1:
				return GraphObject::up;
				break;
			case 2:
				return GraphObject::down;
				break;
			}
		}
		else if (upAvailable) {
			return GraphObject::up;
		}
		else if (downAvailable) {
			return GraphObject::down;
		}
	}
	return GraphObject::none;
}

void Protester::moveInDirection(GraphObject::Direction dir)
{
	int newX = getX();
	int newY = getY();
	switch (dir)
	{
	case GraphObject::up:
		newY++;
		break;
	case GraphObject::right:
		newX++;
		break;
	case GraphObject::down:
		newY--;
		break;
	case GraphObject::left:
		newX--;
		break;
	default:
		break;
	}
	if (getWorld()->canActorMoveTo(this, newX, newY)) {
		setDirection(dir);
		moveTo(newX, newY);
	}
	else {
		//Could not move
		squaresToMoveCurrentDir = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////
// REGULARPROTESTER CLASS extending Protester
//////////////////////////////////////////////////////////////////////////////////////

RegularProtester::RegularProtester(StudentWorld * world, int startX, int startY)
	:Protester(world, startX, startY, IID_PROTESTER, 5)
{
}

void RegularProtester::move()
{
	Protester::move();
	movePostSpecialization(); //Regular protestor has no specialization
}

void RegularProtester::addGold()
{
	getWorld()->increaseScore(25);
	setMustLeaveOilField();
}

//////////////////////////////////////////////////////////////////////////////////////
// HARDCOREPROTESTER CLASS extending Protester
//////////////////////////////////////////////////////////////////////////////////////

HardcoreProtester::HardcoreProtester(StudentWorld * world, int startX, int startY)
	:Protester(world, startX, startY, IID_HARD_CORE_PROTESTER, 20)
{
	homingRange = 16 + (2*getWorld()->getLevel());
}

void HardcoreProtester::move()
{
	Protester::move();

	if (!shouldCont()) {
		return;
	}

	int dist = 9999; //Infinity
	GraphObject::Direction potentialDir = getWorld()->determineFirstMoveToFrackMan(getX(), getY(), dist); //Change value of dist
	if (!getWorld()->findNearbyFrackMan(this, 4) && dist <= homingRange && dist != 9999 && potentialDir != GraphObject::none) {
		moveInDirection(potentialDir);

		return;
	}

	movePostSpecialization();
}

void HardcoreProtester::addGold()
{
	getWorld()->increaseScore(50);
	setTicksToNextMove(50 > 100 - (getWorld()->getLevel() * 10) ? 50 : 100 - (getWorld()->getLevel() * 10));
}
