#ifndef _QUAD_TREE_H_
#define _QUAD_TREE_H_
#include <vector>
#include "FrustumCullling.h"

template<typename DATA>
class QuadTree
{
private:
	float m_x;
	float m_z;
	float m_width;
	float m_height;
	std::vector<DATA> m_data;
	// Children
	QuadTree* m_northWest;
	QuadTree* m_northEast;
	QuadTree* m_southWest;
	QuadTree* m_southEast;

	bool ContainPoint(float x, float z);
	bool Intersect(QuadTree& bounadary);

private:
	void SubDivide();

	bool DontNeedSubDivide;

	int capacity;
public:
	QuadTree(float x, float z, float width, float height, int nodeSize)
	{
		this->m_x = x;
		this->m_z = z;
		this->m_width = width;
		this->m_height = height;
		capacity = nodeSize;
		DontNeedSubDivide = false;
		m_northEast = NULL;
		m_northWest = NULL;
		m_southEast = NULL;
		m_southWest = NULL;
	}
	~QuadTree();
	template<typename DATA>
	bool Insert(DATA data, float x, float z)
	{
		if (!ContainPoint(x, z))
		{
			return false;
		}
		if ((int)m_data.size() < capacity)
		{
			m_data.push_back(data);
			return true;
		}
		if (DontNeedSubDivide == false)
		{
			SubDivide();

		}
		if (m_northWest->Insert(data, x, z))
		{
			return true;
		}
		if (m_northEast->Insert(data, x, z))
		{
			return true;
		}
		if (m_southWest->Insert(data, x, z))
		{
			return true;
		}
		if (m_southEast->Insert(data, x, z))
		{
			return true;
		}
		return false;
	}
	template<typename DATA>
	void Query(FrustumCulling* pFrustum, float x, float z, float width, float height, std::vector<DATA> &output)
	{
		float haftWidth = width / 2.f;
		float haftHeight = height / 2.f;
		float xCenter = x + haftWidth;
		// y == z
		float yCenter = z + (haftHeight);
		QuadTree bounadary(x, z, width, height, 0);
		if (!Intersect(bounadary))
		{
			return;
		}
		if (!pFrustum->CheckRectangle(xCenter, 0, yCenter, haftWidth, 0, haftHeight))
		{
			return;
		}
		for (unsigned int i = 0; i < m_data.size(); i++)
		{
			output.push_back(m_data[i]);
		}
		if (m_northEast)
		{
			m_northEast->Query(pFrustum, x, z, width, height, output);
		}
		if (m_northWest)
		{
			m_northWest->Query(pFrustum, x, z, width, height, output);
		}
		if (m_southEast)
		{
			m_southEast->Query(pFrustum, x, z, width, height, output);
		}
		if (m_southWest)
		{
			m_southWest->Query(pFrustum, x, z, width, height, output);
		}
	}
	

};
#endif // !_QUAD_TREE_H_

#ifndef _QUAD_TREE_CPP_
#define _QUAD_TREE_CPP_
template<typename DATA>
QuadTree<DATA>::~QuadTree()
{
	delete m_northWest;
	delete m_northEast;
	delete m_southEast;
	delete m_southWest;
}
template<typename DATA>
void QuadTree<DATA>::SubDivide()
{
	if (DontNeedSubDivide) {
		return;
	}
	float x = m_x;
	float z = m_z;
	float width = m_width / 2;
	float height = m_height / 2;
	m_northWest = new QuadTree(x, z, width, height, capacity);
	x = m_x + m_width / 2;
	z = m_z;
	width = m_width / 2;
	height = m_height / 2;
	m_northEast = new QuadTree(x, z, width, height, capacity);
	x = m_x;
	z = m_z + m_height / 2;
	width = m_width / 2;
	height = m_height / 2;
	m_southWest = new QuadTree(x, z, width, height, capacity);

	x = m_x + m_width / 2;
	z = m_z + m_height / 2;
	width = m_width / 2;
	height = m_height / 2;
	m_southEast = new QuadTree(x, z, width, height, capacity);

	DontNeedSubDivide = true;
}
template<typename DATA>
bool QuadTree<DATA>::ContainPoint(float x, float z)
{
	return  (x <= m_x + m_width)
		&& (x >= m_x)
		&& (z <= m_z + m_height)
		&& (z >= m_z);
}
template<typename DATA>
bool QuadTree<DATA>::Intersect(QuadTree& bounadary)
{
	return (bounadary.m_x <= m_x + m_width) &&
		(bounadary.m_x + bounadary.m_width >= m_x) &&
		(bounadary.m_z <= m_z + m_height) &&
		(bounadary.m_z + bounadary.m_height >= m_z);
}


#endif // !_QUAD_TREE_CPP_
