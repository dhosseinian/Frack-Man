#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GraphObject.h"
#include "GameWorld.h"
#include <string>
#include <vector>
#include <queue>

//Constants
const int RADIAL_SEPARATION = 6;
const int MAX_BOULDERS = 6;
const int MIN_GOLD = 2;
const int MAX_BARRELS = 20;
const int PROTESTER_DAMAGE = 2;

class Actor;
class FrackMan;

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir);
	virtual ~StudentWorld();

	virtual int init();
	virtual int move();
	virtual void cleanUp();
	
	// Add an actor to the world.
	void addActor(Actor* a);
	
	// Clear a 4x4 region of dirt.
	void clearDirt(int x, int y);

	// Can actor move to x,y?
	bool canActorMoveTo(Actor* a, int x, int y) const;
	bool canActorMoveTo(int x, int y) const; //Generic overload to check if a 4x4 object can move to that position
	
	// Annoy all other actors within radius of annoyer, returning the
	// number of actors annoyed.
	int annoyAllNearbyActors(Actor* annoyer, int points, int radius);
	
	// Reveal all objects within radius of x,y.
	void revealAllNearbyObjects(int x, int y, int radius);
	
	// If the FrackMan is within radius of a, return a pointer to the
	// FrackMan, otherwise null.
	Actor* findNearbyFrackMan(Actor* a, int radius) const;
	
	// If at least one actor that can pick things up is within radius of a,
	// return a pointer to one of them, otherwise null.
	Actor* findNearbyProtester(Actor* a, int radius) const;

	// If at least one actor that cannot pick things up is within radius of a,
	// return a pointer to one of them, otherwise null.
	Actor* findNearbyItem(Actor* a, int radius) const;

	// Annoy the FrackMan.
	void annoyFrackMan();
	
	// Give FrackMan some sonar charges.
	void giveFrackManSonar();

	// Give FrackMan some water.
	void giveFrackManWater();

	// Give FrackMan some gold.
	void giveFrackManGold();

	// Reduce number of oil left
	void reduceOilLeft();

	int getOil() const;

	
	// Is the Actor a facing toward the FrackMan?
	bool facingTowardFrackMan(Actor* a) const;
	
	// If the Actor a has a clear line of sight to the FrackMan, return
	// the direction to the FrackMan, otherwise GraphObject::none.
	GraphObject::Direction lineOfSightToFrackMan(Actor* a) const;
	
	// Return whether the Actor a is within radius of FrackMan.
	bool isNearFrackMan(Actor* a, int radius) const;
	
	// Determine the direction of the first move a quitting protester
	// makes to leave the oil field.
	GraphObject::Direction determineFirstMoveToExit(int x, int y);
	
	// Determine the direction of the first move a hardcore protester
	// makes to approach the FrackMan.
	GraphObject::Direction determineFirstMoveToFrackMan(int x, int y, int& dist);

	//Printing
	std::string adjustTxt(std::string ret, unsigned int targetLength) const {
		while (ret.length() < targetLength) {
			ret = ' ' + ret;
		}
		return ret;
	}
	std::string scoreTxt() const {
		std::string ret = std::to_string(getScore());
		while (ret.length() < 6) ret = '0' + ret; ///DEBUGGING? 6 or 8?
		return ret;
	}
	std::string levelTxt() const { return adjustTxt(std::to_string(getLevel()), 2); }
	std::string livesTxt() const { return std::to_string(getLives()); }
	std::string oilLeftTxt() const { return adjustTxt(std::to_string(getOil()), 2); }
	
private:
	//Struct made for help in searching
	struct Coord
	{
	public:
		Coord(int rr, int cc) : m_r(rr), m_c(cc) {}
		int x() const { return m_r; }
		int y() const { return m_c; }
	private:
		int m_r;
		int m_c;
	};
	GraphObject::Direction distToOrigin[VIEW_WIDTH][VIEW_HEIGHT];
	int distToFrackMan[VIEW_WIDTH][VIEW_HEIGHT];
	
	Actor* m_dirt[VIEW_WIDTH][VIEW_HEIGHT]; //2D dirt array
	Actor* m_fm; //FrackMan
	std::vector<Actor*> actors; //All other actors
	int m_oil;
	
	int m_ticksPR;
	int m_ticksPerPR;
	int m_numPR;
	int m_maxPR;

	int m_probNewItem;

	// Determine whether a certain radius is empty or not
	bool isRadiusEmpty(int x, int y, int radius) const;
	void findNewCoord(bool isBoulder, int& x, int& y) const;
	void findNewCoordWater(int& x, int& y) const;
	void removeDeadItems();

	//Updates the heat map that protesters use to leave the map
	void updateDistToOrigin();
	void updateDistToFM();

	//Determines whether FrackMan is in a line of sight in that direction
	bool lineOfSightDir(GraphObject::Direction dir, int x, int y) const;

	
};

#endif // STUDENTWORLD_H_
