#include "AnimeObj.h"

AnimeObj::AnimeObj() :m_curAnimationStrackIndex(-1),
m_curAnimationTime(-1)
{

}

AnimeObj::AnimationStackInfo AnimeObj::GetAnimationStack(unsigned int i) const
{
	if (i> m_animationStacks.size() - 1)
	{
		return AnimeObj::AnimationStackInfo();
	}
	return m_animationStacks[i];
}
unsigned int AnimeObj::GetAnimationCount() const
{
	return m_animationStacks.size();
}


void AnimeObj::SetAnimationStackIndex(int index)
{
	if (m_animationStacks.size() == 0)
	{
		m_curAnimationStrackIndex = -1;
	}
	else if (index > (int)(m_animationStacks.size() - 1))
	{
		index = m_animationStacks.size() - 1;
	}
	else
	{
		m_curAnimationStrackIndex = index;
	}
	if (m_curAnimationStrackIndex != -1)
	{
		m_curAnimationTime = m_animationStacks[m_curAnimationStrackIndex].start;
	}
	else
	{
		m_curAnimationTime = -1;
	}
}
int AnimeObj::GetAnimationStackIndex()
{
	return m_curAnimationStrackIndex;
}
void AnimeObj::SetAnimationTime(long long time)
{
	if (m_animationStacks.size() == 0 || m_curAnimationStrackIndex == -1)
	{
		m_curAnimationTime = -1;
		return;
	}
	if (time < m_animationStacks[m_curAnimationStrackIndex].start)
	{
		m_curAnimationTime = m_animationStacks[m_curAnimationStrackIndex].start;
	}
	else if (time >= m_animationStacks[m_curAnimationStrackIndex].end)
	{
		m_curAnimationTime = m_animationStacks[m_curAnimationStrackIndex].end;
	}
	else
	{
		m_curAnimationTime = time;
	}
}
long long AnimeObj::GetAnimationTime()
{
	return m_curAnimationTime;
}