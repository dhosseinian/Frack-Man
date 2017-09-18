#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

class StudentWorld;

class Actor : public GraphObject
{
public:
	Actor(StudentWorld* world, int startX, int startY, Direction startDir,
		/*bool visible,*/ int imageID, double size, int depth); //TODOXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

	virtual ~Actor() {}

	// Action to perform each tick.
	virtual void move() = 0;

	// Is this actor alive?
	bool isAlive() const;

	// Mark this actor as dead.
	void setDead();

	// Annoy this actor.
	virtual bool annoy(unsigned int amt); 

	// Get this actor's world
	StudentWorld* getWorld() const;

	// Can other actors pass through this actor?
	virtual bool canActorsPassThroughMe() const;

	// Can this actor dig through dirt?
	virtual bool canDigThroughDirt() const;

	// Can this actor pick items up?
	virtual bool canPickThingsUp() const;

	// Does this actor hunt the FrackMan?
	virtual bool huntsFrackMan() const;

	// Can this actor need to be picked up to finish the level?
	virtual bool needsToBePickedUpToFinishLevel() const;

	// Move this actor to x,y if possible, and return true; otherwise,
	// return false without moving.
	bool moveToIfPossible(int x, int y);

private:
	StudentWorld* m_world;
	bool m_alive;
};

//Agent constants
const double SIZE_AG = 1.0;

class Agent : public Actor
{
public:
	Agent(StudentWorld* world, int startX, int startY, Direction startDir,
		int imageID, unsigned int hitPoints, int depth);

	virtual ~Agent() {}

	// Pick up a gold nugget.
	virtual void addGold() = 0;

	// How many hit points does this actor have left?
	unsigned int getHitPoints() const;

	virtual bool annoy(unsigned int amount); //TODOXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	virtual bool canPickThingsUp() const;

private:
	int m_hp;
};

//FrackMan constants
const unsigned int DEPTH_FM = 0;
//Default values
const int DEFAULT_FM_HP = 10;
const int DEFAULT_SQUIRTS = 5;
const int DEFAULT_SONAR = 1;
const int DEFAULT_GOLD = 0;
//Default position
const int DEFAULT_FM_X = 30;
const int DEFAULT_FM_Y = 60;
const GraphObject::Direction DEFAULT_DIR_FM = GraphObject::Direction::right;

class FrackMan : public Agent
{
public:
	FrackMan(StudentWorld * world, int startX = DEFAULT_FM_X, int startY = DEFAULT_FM_Y);
	virtual ~FrackMan() {}
	virtual void move();
	//virtual bool annoy(unsigned int amount);
	virtual void addGold();
	virtual bool canDigThroughDirt() const;

	// Pick up a sonar kit.
	void addSonar();

	// Pick up water.
	void addWater();

	// Get amount of gold
	unsigned int getGold() const;

	// Get amount of sonar charges
	unsigned int getSonar() const;

	// Get amount of water
	unsigned int getWater() const;

	//Printing functions

	std::string adjustTxt(std::string ret, unsigned int targetLength) const {
		while (ret.length() < targetLength) {
			ret = ' ' + ret;
		}
		return ret;
	}
	
	
	std::string healthTxt() const { return adjustTxt(std::to_string(getHitPoints() * 10), 3) + "%"; }
	std::string waterTxt() const { return adjustTxt(std::to_string(getWater()), 2); }
	std::string goldTxt() const { return adjustTxt(std::to_string(getGold()), 2); }
	std::string sonarTxt() const { return adjustTxt(std::to_string(getSonar()), 2); }

private:
	int m_gold;
	int m_sonar;
	int m_water;

	// Helper function to handle moving and digging to new position
	void digMoveTo(int x, int y);
};

//Protester constants
const GraphObject::Direction DEFAULT_DIR_PR = GraphObject::Direction::left;
const unsigned int DEPTH_PR = 0;
const int YELLING_RADIUS_PR = 4;
const int REFRACTORY_PR = 15;

class Protester : public Agent
{
public:
	Protester(StudentWorld* world, int startX, int startY, int imageID,
		unsigned int hitPoints);
	virtual void move();
	virtual bool annoy(unsigned int amount);
	virtual void addGold() = 0; //Forces class to be Abstract
	virtual bool huntsFrackMan() const;

	// Set state to having gien up protest
	void setMustLeaveOilField();

	// Set number of ticks until next move
	void setTicksToNextMove(int amount);

private:
	int squaresToMoveCurrentDir;
	bool leaving;
	int ticksToWait;
	int ticksRested;
	int ticksRefracting;
	int ticksPerp;
	bool shouldContinue;

	//Checks if there's a perp direction available and if so randomly returns one
	GraphObject::Direction perpDirAvailable() const;
	
	
protected:
	void moveInDirection(GraphObject::Direction dir);
	void movePostSpecialization();
	bool shouldCont() const;
};

class RegularProtester : public Protester
{
public:
	RegularProtester(StudentWorld* world, int startX, int startY);
	virtual void move();
	virtual void addGold();
};

class HardcoreProtester : public Protester
{
public:
	HardcoreProtester(StudentWorld* world, int startX, int startY);
	virtual void move();
	virtual void addGold();
private:
	int homingRange;
};


//Dirt constants
const double SIZE_DIRT = .25;
const unsigned int DEPTH_DIRT = 3;
const GraphObject::Direction DIR_DIRT = GraphObject::Direction::none;

class Dirt : public Actor
{
public:
	Dirt(StudentWorld* world, int startX, int startY);
	virtual ~Dirt() {}
	virtual void move();
};

//BOULDER constants
const double SIZE_BOULDER = 1;
const unsigned int DEPTH_BOULDER = 1;
const GraphObject::Direction DIR_BOULDER = GraphObject::Direction::down;
const int TICKS_WAIT_BOULDER = 30;
const int DAMAGE_BOULDER = 100;
const int RADIUS_BOULDER = 3;

class Boulder : public Actor
{
public:
	Boulder(StudentWorld* world, int startX, int startY);
	virtual void move();
	virtual bool canActorsPassThroughMe() const;
private:
	enum State {stable, waiting, falling};
	State m_state;
	int m_ticksWaiting;
};

//SQUIRT constants
const double SIZE_SQUIRT = 1;
const unsigned int DEPTH_SQUIRT = 1;
const int AGE_SQUIRT = 4;
const int DAMAGE_SQUIRT = 2;
const int RADIUS_SQUIRT = 3;

class Squirt : public Actor
{
public:
	Squirt(StudentWorld* world, int startX, int startY, Direction startDir);
	virtual void move();

private:
	int m_age;
};

//ACTIVATINGOBJECT constants
const double SIZE_AO = 1;
const unsigned int DEPTH_AO = 2;
const GraphObject::Direction DIR_AO = GraphObject::Direction::right;
const int RADIUS_AO = 3;
const int VISIBILITY_AO = 4;
const int TICKS_WAIT_MIN_AO = 100;

//OILBARREL constants
const int SCORE_BARREL = 1000;

//GOLDNUGGET constant
const int SCORE_GOLD_PLAYER = 10;
const int SCORE_GOLD_PROTESTER = 25;
const int TICKS_WAIT_GOLD = 100;

//SONARKIT constant
const int SCORE_SONAR = 75;

//WATERPOOl constant
const int SCORE_WATER = 100;

class ActivatingObject : public Actor
{
public:
	ActivatingObject(StudentWorld* world, int startX, int startY, int imageID,
		int soundToPlay, bool activateOnPlayer,
		bool activateOnProtester, bool initallyActive);
	virtual void move();

	// Set number of ticks until this object dies
	void setTicksToLive();

	// Check if object died from expiring
	bool isExpired() const;

private:
	int m_sound;
	bool m_playerObj;
	bool m_protesterObj;
	bool m_isActive; //Is ticking down?
	bool m_expired;
	int m_tickWaiting;
};

class OilBarrel : public ActivatingObject
{
public:
	OilBarrel(StudentWorld* world, int startX, int startY);
	virtual void move();
	virtual bool needsToBePickedUpToFinishLevel() const;
};

class GoldNugget : public ActivatingObject
{
public:
	GoldNugget(StudentWorld* world, int startX, int startY, bool temporary);
	virtual void move();
private:
	bool m_isTemporary;
};

class SonarKit : public ActivatingObject
{
public:
	SonarKit(StudentWorld* world, int startX, int startY);
	virtual void move();
};

class WaterPool : public ActivatingObject
{
public:
	WaterPool(StudentWorld* world, int startX, int startY);
	virtual void move();
};

#endif // ACTOR_H_