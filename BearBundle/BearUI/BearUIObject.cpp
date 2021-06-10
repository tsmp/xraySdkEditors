#include "BearUI.hpp"

BearUIObject::BearUIObject()
{
	bOpen = true;
}

BearUIObject::~BearUIObject()
{
	for (BearUIObject*obj:m_Objects)
	{
		BEAR_CHECK(obj->StateFlags.test((uint32)EStateFlags::Use));
		obj->StateFlags.set((uint32)EStateFlags::Use, false);
		if (!obj->StateFlags.test((uint32)EStateFlags::NoDelete))
		{
			bear_delete(obj);
		}
	}
}

void BearUIObject::Unload()
{
	for (BearUIObject* obj : m_Objects)
	{
		obj->Unload();
	}
}

void BearUIObject::Load()
{
	for (BearUIObject* obj : m_Objects)
	{
		obj->Load();
	}
}

void BearUIObject::Draw()
{
	for (BearUIObject* obj : m_Objects)
	{
		obj->Draw();
	}
}

void BearUIObject::Ñleanup()
{
	for (size_t i = m_Objects.size(); i > 0; i--)
	{
		if (m_Objects[i - 1]->IsClosed()&& m_Objects[i - 1]->StateFlags.test((uint32)EStateFlags::WhenClosingDeleteIt))
		{
			if (!m_Objects[i - 1]->StateFlags.test((uint32)EStateFlags::NoDelete))
			{
				bear_delete(m_Objects[i - 1]);
			}
			m_Objects.erase(m_Objects.begin() + (i - 1));
			i = m_Objects.size();
			if (i == 0)return;
		}
	}
	for (BearUIObject* obj : m_Objects)
	{
		obj->Ñleanup();
	}
}

void BearUIObject::Push(BearUIObject* obj, bool auto_delete)
{
	BEAR_CHECK(obj != this);
	BEAR_CHECK(!obj->StateFlags.test((uint32)EStateFlags::Use));
	obj->StateFlags.set( true, (uint32)EStateFlags::Use);
	obj->StateFlags.set( !auto_delete, (uint32)EStateFlags::NoDelete);
	m_Objects.push_back(obj);
}

void BearUIObject::Pop(BearUIObject* obj)
{
	auto Item = bear_find_if(m_Objects.begin(), m_Objects.end(), [obj](BearUIObject* right) {return right == obj; });
	if (Item == m_Objects.end())return;
	BEAR_CHECK(!obj->StateFlags.test((uint32)EStateFlags::Use));
	obj->StateFlags.set(false, (uint32)EStateFlags::Use);
	m_Objects.erase(Item);

}
