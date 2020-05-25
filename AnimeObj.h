#ifndef _ANIME_OBJ_H_
#define _ANIME_OBJ_H_

#include "ModelInF.h"
class AnimeObj : public ModelInF
{
public:
	AnimeObj();
	virtual ~AnimeObj()
	{

	}
	struct AnimationStackInfo
	{
		std::string name;
		long long start;
		long long end;
	};
	void SetAnimationStackIndex(int index);
	int GetAnimationStackIndex();
	void SetAnimationTime(long long time);
	long long GetAnimationTime();
	AnimationStackInfo GetAnimationStack(unsigned int i) const;
	unsigned int GetAnimationCount() const;
private:
	std::vector<AnimationStackInfo> m_animationStacks;
	int m_curAnimationStrackIndex;
	long long m_curAnimationTime;
	friend ObjManager;
};
#endif // !_ANIME_OBJ_H_

