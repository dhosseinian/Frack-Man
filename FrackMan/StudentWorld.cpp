#include "StudentWorld.h"
#include "Actor.h"
#include <string>
#include <random>
using namespace std;

// Return a random int from min to max, inclusive
int randInt(int min, int max)
{
	if (max < min)
		swap(max, min);
	static random_device rd;
	static mt19937 generator(rd());
	uniform_int_distribution<> distro(min, max);
	return distro(generator);
}

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::StudentWorld(std::string assetDir)
	:GameWorld(assetDir)
{
	m_fm = nullptr;
	for (int x = 0; x < VIEW_WIDTH; x++) {
		for (int y = 0; y < VIEW_HEIGHT; y++) {
			m_dirt[x][y] = nullptr;
		}
	}
}

StudentWorld::~StudentWorld()
{
	delete m_fm;
	//Delete dirt
	for (int y = 0; y < VIEW_HEIGHT; y++) {
		for (int x = 0; x < VIEW_WIDTH; x++) {
			delete m_dirt[x][y];
		}
	}
	//Delete actors
	vector<Actor*>::iterator i = actors.begin();
	while (i != actors.end()) {
		delete *i;
		i = actors.erase(i);
		continue;
		i++;
	}
}

int StudentWorld::init()
{
	
	//Establish number of oils
	int barrelLvl = 2 + getLevel();
	m_oil = MAX_BARRELS < barrelLvl ? MAX_BARRELS : barrelLvl; //Set to minimum
	
	//Create new FrackMan and add dirt to the field except in the shaft
	m_fm = new FrackMan(this);

	int msLeftBound = (VIEW_WIDTH / 2) - (SPRITE_WIDTH / 2);
	int msRightBound = (VIEW_WIDTH / 2) + (SPRITE_WIDTH / 2);
	int msBottomBound = SPRITE_HEIGHT;
	for (int x = 0; x < VIEW_WIDTH; x++) {
		int height = (x < msLeftBound || x >= msRightBound) ? VIEW_HEIGHT - SPRITE_HEIGHT : SPRITE_HEIGHT;
		for (int y = 0; y < height; y++) {
			m_dirt[x][y] = new Dirt(this, x, y);
		}
	}
	
	//Create boulders
	int boulderLvl = (getLevel() / 2) + 2;
	int numBoulders = MAX_BOULDERS < boulderLvl ? MAX_BOULDERS : boulderLvl; //Set to the minimum
	for (int i = 0; i < numBoulders; i++) {
		int x = -1;
		int y = -1;
		findNewCoord(true, x, y); //Finds new valid position
		addActor(new Boulder(this, x, y));
	}

	//Create gold nuggets
	int goldLvl = 5 - (getLevel() / 2);
	int numGold = MIN_GOLD > goldLvl ? MIN_GOLD : goldLvl; //Set to maximum
	for (int i = 0; i < numGold; i++) {
		int x = -1;
		int y = -1;
		findNewCoord(false, x, y); //Finds new valid position
		addActor(new GoldNugget(this, x, y, false)); //Create non-temp gold nuggets
	}
	
	//Create oil barrels
	for (int i = 0; i < m_oil; i++) {
		int x = -1;
		int y = -1;
		findNewCoord(false, x, y); //Finds new valid position
		addActor(new OilBarrel(this, x, y)); //Create non-temp gold nuggets
	}

	addActor(new RegularProtester(this, 60, 60)); //First protester
	m_numPR = 1;
	//FOR TESTING PURPOSES: addActor(new HardcoreProtester(this, 60, 60)); //First protester
	
	m_ticksPR = 0;
	m_ticksPerPR = 25 > 200 - getLevel() ? 25 : 200 - getLevel();
	m_maxPR = 15 < 2 + (getLevel() * 1.5) ? 15 : 2 + (getLevel() * 1.5);

	//Determine likelihood of new item
	m_probNewItem = (getLevel() * 25) + 300;

	return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
	//TODOXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	//Update text
	//static int ticks = 0;
	//setGameStatText(std::to_string(ticks)); ticks++;
	setGameStatText("Scr: " + (scoreTxt() +
		"  Lvl: " + levelTxt() +
		"  Lives: " + livesTxt() +
		"  Hlth: " + ((FrackMan*)m_fm)->healthTxt() +
		"  Wtr: " + ((FrackMan*)m_fm)->waterTxt() +
		"  Gld: " + ((FrackMan*)m_fm)->goldTxt() +
		"  Sonar: " + ((FrackMan*)m_fm)->sonarTxt() +
		"  Oil Left: " + oilLeftTxt()));

	m_fm->move();
	if (!m_fm->isAlive()) {
		return GWSTATUS_PLAYER_DIED;
	}

	//See if we need to add protesters
	if (m_ticksPR >= m_ticksPerPR) {
		m_ticksPR = 0;
		if (m_numPR < m_maxPR) {
			int probHC = 90 < (getLevel() * 10) + 30 ? 90 : (getLevel() * 10) + 30;
			int roll = randInt(1, 100);
			if (roll <= probHC) {
				addActor(new HardcoreProtester(this, 60, 60));
			}
			else {
				addActor(new RegularProtester(this, 60, 60));
			}
			m_numPR++;
		}
	}
	m_ticksPR++;

	//Add new sonar or water
	if (randInt(1, m_probNewItem) == 1) {
		if (randInt(1, 5) == 1) {
			addActor(new SonarKit(this, 0, 60));
		}
		else {
			int wX;
			int wY;
			findNewCoordWater(wX, wY);
			addActor(new WaterPool(this, wX, wY));
		}
	}

	//Update protester tracking maps
	updateDistToOrigin();
	updateDistToFM();
	
	//Have each actor move
	vector<Actor*>::iterator i = actors.begin();
	while (i != actors.end()) {
		if ((*i)->isAlive()) {
			(*i)->move();
			if (!m_fm->isAlive()) {
				return GWSTATUS_PLAYER_DIED;
			}
			if (m_oil <= 0) { //No oil left
				return GWSTATUS_FINISHED_LEVEL;
			}
		}
		i++;
	}
	removeDeadItems();
	
	//Check if player died
	if (!m_fm->isAlive()) {
		return GWSTATUS_PLAYER_DIED;
	}
	//Check if level is completed
	if (m_oil <= 0) { //No oil left
		return GWSTATUS_FINISHED_LEVEL;
	}
	
	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
	delete m_fm;
	//Delete dirt
	for (int y = 0; y < VIEW_HEIGHT; y++) {
		for (int x = 0; x < VIEW_WIDTH; x++) {
			delete m_dirt[x][y];
		}
	}
	//Delete actors
	vector<Actor*>::iterator i = actors.begin();
	while (i != actors.end()) {
		delete *i;
		i = actors.erase(i);
		continue;
		i++;
	}
}

void StudentWorld::addActor(Actor * a)
{
	actors.push_back(a);
}

// Precondition: x and y correspond to a position with a valid 4 x 4 grid
void StudentWorld::clearDirt(int x, int y)
{
	bool soundPlayed = false;
	for (int r = x; r < x + SPRITE_WIDTH; r++) {
		for (int c = y; c < y + SPRITE_HEIGHT; c++) {
			//Play digging sound only once
			if (!soundPlayed && m_dirt[r][c] != nullptr) {
				playSound(SOUND_DIG);
				soundPlayed = true;
			}
			delete m_dirt[r][c];
			m_dirt[r][c] = nullptr;
		}
	}
}

// Precondition: Actor a has size of 1 (4 x 4 area)
bool StudentWorld::canActorMoveTo(Actor * a, int x, int y) const
{
	int maxX = VIEW_WIDTH - SPRITE_WIDTH;
	int maxY = VIEW_HEIGHT - SPRITE_HEIGHT;

	//Check if in bounds
	if (!(x >= 0 && x <= maxX && y >= 0 && y <= maxY)) {
		return false;
	}

	//Check to see if an impassable object will be hit
	for (vector<Actor*>::const_iterator it = actors.begin(); it != actors.end(); it++) {
		//Check for aliasing and make only Euclidean distance of 3 impassable
		if (a != *it && !(*it)->canActorsPassThroughMe()) {
			int dx = (*it)->getX() - x;
			int dy = (*it)->getY() - y;
			if (RADIUS_BOULDER*RADIUS_BOULDER >= dx*dx + dy*dy) {
				return false;
			}
		}
	}

	// Implies a FrackMan
	// Won't look at dirt
	if (a->canDigThroughDirt()) {
		return true;
	}
	
	for (int r = x; r < x + SPRITE_WIDTH; r++) {
		for (int c = y; c < y + SPRITE_HEIGHT; c++) {
			if (m_dirt[r][c] != nullptr) {
				return false;
			}
		}
	}

	return true; //No dirt found
}

bool StudentWorld::canActorMoveTo(int x, int y) const
{
	int maxX = VIEW_WIDTH - SPRITE_WIDTH;
	int maxY = VIEW_HEIGHT - SPRITE_HEIGHT;

	//Check if in bounds
	if (!(x >= 0 && x <= maxX && y >= 0 && y <= maxY)) {
		return false;
	}

	//Check to see if an impassable object will be hit
	for (vector<Actor*>::const_iterator it = actors.begin(); it != actors.end(); it++) {
		//Make only Euclidean distance of 3 impassable
		if (!(*it)->canActorsPassThroughMe()) {
			int dx = (*it)->getX() - x;
			int dy = (*it)->getY() - y;
			if (RADIUS_BOULDER*RADIUS_BOULDER >= dx*dx + dy*dy) {
				return false;
			}
		}
	}

	//Is there any dirt?
	for (int r = x; r < x + SPRITE_WIDTH; r++) {
		for (int c = y; c < y + SPRITE_HEIGHT; c++) {
			if (m_dirt[r][c] != nullptr) {
				return false;
			}
		}
	}

	return true; //No dirt found
}

int StudentWorld::annoyAllNearbyActors(Actor * annoyer, int points, int radius)
{
	int numAnnoyed = 0;
	//Loop through all actors and annoy them if close by
	for (vector<Actor*>::const_iterator it = actors.begin(); it != actors.end(); it++) {
		int dx = (*it)->getX() - annoyer->getX(); //Change in x
		int dy = (*it)->getY() - annoyer->getY(); //Change in y
		//Check for aliasing and see if agent is within  Euclidian radius
		if (annoyer != *it && (radius*radius >= dx*dx + dy*dy)) {
			(*it)->annoy(points);
			numAnnoyed++;
		}
	}
	//Attemp to annoy FrackMan
	int dx = m_fm->getX() - annoyer->getX(); //Change in x
	int dy = m_fm->getY() - annoyer->getY(); //Change in y
	if (radius*radius >= dx*dx + dy*dy) {
		m_fm->annoy(points);
		numAnnoyed++;
	}
	return numAnnoyed;
}

void StudentWorld::revealAllNearbyObjects(int x, int y, int radius)
{
	for (vector<Actor*>::const_iterator it = actors.begin(); it != actors.end(); it++) {
		int dx = (*it)->getX() - x; //Change in x
		int dy = (*it)->getY() - y; //Change in y
		//See if agent is within  Euclidian radius
		if (radius*radius >= dx*dx + dy*dy) {
			(*it)->setVisible(true);
		}
	}
}

Actor * StudentWorld::findNearbyFrackMan(Actor * a, int radius) const
{
	int dx = a->getX() - m_fm->getX(); //Change in x
	int dy = a->getY() - m_fm->getY(); //Change in y
	if (radius*radius >= dx*dx + dy*dy) {
		return m_fm;
	}
	return nullptr;
}

Actor * StudentWorld::findNearbyProtester(Actor * a, int radius) const
{
	for (vector<Actor*>::const_iterator it = actors.begin(); it != actors.end(); it++) {
		int dx = a->getX() - (*it)->getX(); //Change in x
		int dy = a->getY() - (*it)->getY(); //Change in y
		//See if agent is within  Euclidian radius
		if ((radius*radius >= dx*dx + dy*dy) && (*it)->huntsFrackMan()) {
			return *it; //Return first protester found
		}
	}
	return nullptr;
}

Actor * StudentWorld::findNearbyItem(Actor * a, int radius) const
{
	for (vector<Actor*>::const_iterator it = actors.begin(); it != actors.end(); it++) {
		int dx = a->getX() - (*it)->getX(); //Change in x
		int dy = a->getY() - (*it)->getY(); //Change in y
											//See if agent is within  Euclidian radius
		if ((radius*radius >= dx*dx + dy*dy) && !(*it)->canPickThingsUp()) {
			return *it; //Return first item found
		}
	}
	return nullptr;
}

void StudentWorld::annoyFrackMan()
{
	m_fm->annoy(PROTESTER_DAMAGE);
}

void StudentWorld::giveFrackManSonar()
{
	((FrackMan*)m_fm)->addSonar();
}

void StudentWorld::giveFrackManWater()
{
	((FrackMan*)m_fm)->addWater();
}

void StudentWorld::giveFrackManGold()
{
	((FrackMan*)m_fm)->addGold();
}

void StudentWorld::reduceOilLeft()
{
	m_oil--;
}

int StudentWorld::getOil() const
{
	return m_oil;
}

// Precondition: a is within 4 Euclidean distance of FrackMan
bool StudentWorld::facingTowardFrackMan(Actor * a) const
{
	//Actor coordinates
	int aX = a->getX();
	int aY = a->getY();
	
	//Frackman Coordinates
	int fX = m_fm->getX();
	int fY = m_fm->getY();

	//Differences
	int dX = aX - fX;
	int dY = aY - fY;

	if (dX == 0 && dY == 0) {
		return true;
	}
	if ((dX >= 0 && dY >= 0) && (a->getDirection() == GraphObject::left || a->getDirection() == GraphObject::down)) {
		return true;
	}
	if ((dX <= 0 && dY >= 0) && (a->getDirection() == GraphObject::right || a->getDirection() == GraphObject::down)) {
		return true;
	}
	if ((dX >= 0 && dY <= 0) && (a->getDirection() == GraphObject::left || a->getDirection() == GraphObject::up)) {
		return true;
	}
	if ((dX <= 0 && dY <= 0) && (a->getDirection() == GraphObject::right || a->getDirection() == GraphObject::up)) {
		return true;
	}
	return false;
}

GraphObject::Direction StudentWorld::lineOfSightToFrackMan(Actor * a) const
{
	int x = a->getX();
	int y = a->getY();
	if (lineOfSightDir(GraphObject::up, x, y)) {
		return GraphObject::up;
	}
	else if (lineOfSightDir(GraphObject::right, x, y)) {
		return GraphObject::right;
	}
	else if (lineOfSightDir(GraphObject::left, x, y)) {
		return GraphObject::left;
	}
	else if (lineOfSightDir(GraphObject::down, x, y)) {
		return GraphObject::down;
	}
	else {
		return GraphObject::none;
	}
}

bool StudentWorld::isNearFrackMan(Actor * a, int radius) const
{
	return findNearbyFrackMan(a, radius) != nullptr;
}

// Precondition: x and y correspond to a valid position
GraphObject::Direction StudentWorld::determineFirstMoveToExit(int x, int y)
{
	return distToOrigin[x][y];
}

GraphObject::Direction StudentWorld::determineFirstMoveToFrackMan(int x, int y, int& dist)
{
	int min = 9999; //Set to infinity
	GraphObject::Direction best = GraphObject::none;
	//Return direction with lowest distance
	//Check north
	if (canActorMoveTo(x, y + 1) && min > distToFrackMan[x][y + 1]) {
		min = distToFrackMan[x][y + 1];
		best = GraphObject::up;
	}
	//Check east
	if (canActorMoveTo(x + 1, y) && min > distToFrackMan[x + 1][y]) {
		min = distToFrackMan[x + 1][y];
		best = GraphObject::right;
	}
	//Check south
	if (canActorMoveTo(x, y - 1) && min > distToFrackMan[x][y - 1]) {
		min = distToFrackMan[x][y - 1];
		best = GraphObject::down;
	}
	//Check west
	if (canActorMoveTo(x - 1, y) && min > distToFrackMan[x - 1][y]) {
		min = distToFrackMan[x - 1][y];
		best = GraphObject::left;
	}
	dist = min;
	return best; 
}

bool StudentWorld::isRadiusEmpty(int x, int y, int radius) const
{
	for (vector<Actor*>::const_iterator it = actors.begin(); it != actors.end(); it++) {
		int dx = x - (*it)->getX(); //Change in x
		int dy = y - (*it)->getY(); //Change in y
		//See if coordinate is within  Euclidian radius
		if (radius*radius >= dx*dx + dy*dy) {
			return false; //Return first actor found
		}
	}
	return true;
}

void StudentWorld::findNewCoord(bool isBoulder, int & x, int & y) const
{
	int msLeftBound = (VIEW_WIDTH / 2) - (SPRITE_WIDTH / 2);
	int msRightBound = (VIEW_WIDTH / 2) + (SPRITE_WIDTH / 2);
	int lowestY = isBoulder ? 20 : 0;
	do {
		x = randInt(0, 60);
		y = randInt(lowestY, 56);
	} while (!(x < msLeftBound - SPRITE_WIDTH || x >= msRightBound) || !isRadiusEmpty(x, y, RADIAL_SEPARATION));
}

void StudentWorld::findNewCoordWater(int & x, int & y) const
{
	do {
		x = randInt(0, 60);
		y = randInt(0, 60);
	} while (!canActorMoveTo(x, y));
}

void StudentWorld::removeDeadItems()
{
	vector<Actor*>::iterator i = actors.begin();
	while (i != actors.end()) {
		if (!(*i)->isAlive()) {
			if ((*i)->huntsFrackMan()) { //Reduce protester count
				m_numPR--;
			}
			delete *i;
			i = actors.erase(i);
			continue;
		}
		i++;
	}
}

void StudentWorld::updateDistToOrigin()
{
	//Initialize the distances to "infinity"
	for (int x = 0; x < VIEW_WIDTH; x++) {
		for (int y = 0; y < VIEW_HEIGHT; y++) {
			distToOrigin[x][y] = GraphObject::none;
		}
	}
	Coord start(60, 60);
	queue<Coord> q;
	q.push(start);
	while (!q.empty()) {
		Coord cur = q.front();
		q.pop();
		int x = cur.x();
		int y = cur.y();
		// Enqueue north pos if valid and not searched
		if (canActorMoveTo(x, y + 1) && distToOrigin[x][y + 1] == GraphObject::none) {
			distToOrigin[x][y + 1] = GraphObject::down; //Position above will need to come down
			q.push(Coord(x, y + 1)); //Push onto queue
		}
		//Same with east
		if (canActorMoveTo(x + 1, y) && distToOrigin[x + 1][y] == GraphObject::none) {
			distToOrigin[x + 1][y] = GraphObject::left; //Position to right will need to come left
			q.push(Coord(x + 1, y));
		}
		//...and south
		if (canActorMoveTo(x, y - 1) && distToOrigin[x][y - 1] == GraphObject::none) {
			distToOrigin[x][y - 1] = GraphObject::up; //Position below will need to come up
			q.push(Coord(x, y - 1)); //Push onto queue
		}
		//...and ya guessed it... WEST... LIKE KANYE
		if (canActorMoveTo(x - 1, y) && distToOrigin[x - 1][y] == GraphObject::none) {
			distToOrigin[x - 1][y] = GraphObject::right; //Postion to left will need to come right
			q.push(Coord(x - 1, y));
		}
	}
}

void StudentWorld::updateDistToFM()
{

	//Initialize the distances to "infinity"
	for (int x = 0; x < VIEW_WIDTH; x++) {
		for (int y = 0; y < VIEW_HEIGHT; y++) {
			distToFrackMan[x][y] = 9999;
		}
	}
	Coord start(m_fm->getX(), m_fm->getY());
	distToFrackMan[start.x()][start.y()] = 0;
	queue<Coord> q;
	q.push(start);
	while (!q.empty()) {
		Coord cur = q.front();
		q.pop();
		int x = cur.x();
		int y = cur.y();
		// Enqueue north pos if valid and not searched
		if (canActorMoveTo(x, y + 1) && distToFrackMan[x][y + 1] == 9999) {
			distToFrackMan[x][y + 1] = distToFrackMan[x][y] + 1; //Distance will be one more than parent
			q.push(Coord(x, y + 1)); //Push onto queue
		}
		//Same with east
		if (canActorMoveTo(x + 1, y) && distToFrackMan[x + 1][y] == 9999) {
			distToFrackMan[x + 1][y] = distToFrackMan[x][y] + 1; //Position to right will need to come left
			q.push(Coord(x + 1, y));
		}
		//...and south
		if (canActorMoveTo(x, y - 1) && distToFrackMan[x][y - 1] == 9999) {
			distToFrackMan[x][y - 1] = distToFrackMan[x][y] + 1; //Position below will need to come up
			q.push(Coord(x, y - 1)); //Push onto queue
		}
		//...and ya guessed it... WEST... LIKE KANYE
		if (canActorMoveTo(x - 1, y) && distToFrackMan[x - 1][y] == 9999) {
			distToFrackMan[x - 1][y] = distToFrackMan[x][y] + 1; //Postion to left will need to come right
			q.push(Coord(x - 1, y));
		}
	}
}

//Precondition: x and y are valid coordinates
bool StudentWorld::lineOfSightDir(GraphObject::Direction dir, int x, int y) const
{
	int fmX = m_fm->getX();
	int fmY = m_fm->getY();

	if (dir == GraphObject::none) {
		return false;
	}

	while (canActorMoveTo(x, y)) {
		if (x == fmX && y == fmY) {
			return true;
		}
		switch (dir)
		{
		case GraphObject::up:
			y++;
			break;
		case GraphObject::right:
			x++;
			break;
		case GraphObject::down:
			y--;
			break;
		case GraphObject::left:
			x--;
			break;
		}
	}
	return false;
}

