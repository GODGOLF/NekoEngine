/*
SaveLoadMapManager.h

Author: Zhuang Theerapong

*/
#ifndef SAVE_LOAD_MAP_MANAGER
#define SAVE_LOAD_MAP_MANAGER
#include <iostream>
#include <fstream>
#include <vector>
#include "ModelProperty.h"
class PhysicManager;
class SaveLoadMapManager {
public:
	SaveLoadMapManager();
	~SaveLoadMapManager();
	void initial();
	void save(std::vector<ModelProperty*>* objects);
	void load(std::vector<ModelProperty*>* objects);
	void destroy();
private:
	void clearData(std::vector<ModelProperty*>* objects);
};

#endif // !SAVE_LOAD_MAP_MANAGER

