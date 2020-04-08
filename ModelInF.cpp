#include "ModelInF.h"

ModelInF::AnimationStackInfo ModelInF::GetAnimationStack(unsigned int i) const
{
	if (i> m_animationStacks.size()-1)
	{
		return ModelInF::AnimationStackInfo();
	}
	return m_animationStacks[i];
}
unsigned int ModelInF::GetAnimationCount() const
{
	return m_animationStacks.size();
}
std::string ModelInF::GetModelIndex() const
{
	return m_modelIndex;
}

void ModelInF::SetAnimationStackIndex(int index)
{
	if (m_animationStacks.size() == 0)
	{
		m_curAnimationStrackIndex = -1;
	}
	else if (index > (int)(m_animationStacks.size()-1))
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
int ModelInF::GetAnimationStackIndex()
{
	return m_curAnimationStrackIndex;
}
void ModelInF::SetAnimationTime(long long time)
{
	if (m_animationStacks.size() == 0 || m_curAnimationStrackIndex == -1)
	{
		m_curAnimationTime = -1;
		return;
	}
	if (time < m_animationStacks[m_curAnimationStrackIndex].start )
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
long long ModelInF::GetAnimationTime()
{
	return m_curAnimationTime;
}