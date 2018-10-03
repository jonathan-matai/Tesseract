#include "..\include\vBox.hpp"

vBox::vBox()
{
	defineBox();
}

vBox::vBox(pvPlane newbox)
{
	defineBox(newbox);
}

vBox::vBox(vPoint * newbox)
{
	defineBox(newbox);
}

vBox::vBox(vVector * newbox)
{
	defineBox(newbox);
}

vPlane vBox::getBoxElement(int planeID)
{
	return m_box.at(static_cast<size_t>(planeID));
}

void vBox::defineBox()
{
	m_box.push_back({ { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 1.0f } });
	m_box.push_back({ { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 1.0f } });
	m_box.push_back({ { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } });
	m_box.push_back({ { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } });
	m_box.push_back({ { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } });
	m_box.push_back({ { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } });

	usedPlanes = static_cast<int>(m_box.size());
}

void vBox::defineBox(pvPlane newbox, int planeElements)
{
	if (newbox == NULL)
	{
		return;
	}

	m_box.clear();

	for (int i = 0; i < planeElements; ++i)
	{
		m_box.push_back(newbox[i]);
	}

	usedPlanes = static_cast<int>(m_box.size());
}

void vBox::defineBox(vPoint * newbox, int pointElements)
{
	if (newbox == NULL)
	{
		return;
	}

	usedPlanes = pointElements / 3;

	for (int i = 0; i < usedPlanes; ++i)
	{
		m_box.push_back({ newbox[i * 3], newbox[(i * 3) + 1], newbox[(i * 3) + 2] });
	}
}

void vBox::defineBox(vVector * newbox)
{

}

void vBox::recalculateBox(pvPlane newbox)
{

}

void vBox::recalculateBox(vPlane newbox, int planeID)
{
}

void vBox::recalculateBox(vPoint * newplane, int planeID)
{
}

void vBox::recalculateBox(vVector newbox, int planeID)
{
}

void vBox::changeBoxDimension(pvPlane newbox, int dimension)
{
}

void vBox::changeBoxDimension(int planeToRemove)
{
}

int vBox::optimizeBox()
{
	return 0;
}

vBox::~vBox()
{
	m_box.clear();

	usedPlanes = 0;
	center = { 0.0f, 0.0f, 0.0f };
}
