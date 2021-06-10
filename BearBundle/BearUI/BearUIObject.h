#pragma once
class BEARUI_API BearUIObject
{
public:
	BearUIObject();
	virtual ~BearUIObject();
	virtual void Load();
	virtual void Unload();
	virtual void Draw();
	virtual void Push(BearUIObject*obj,bool auto_delete=true);
	virtual void Pop(BearUIObject* obj);
protected:
	virtual void Ñleanup();
	BearUIObject* Parent;
	enum class EStateFlags
	{
		Use = 1<<0,
		NoDelete = 1<<1,
		WhenClosingDeleteIt = 1 << 3,
	};

	inline bool IsClosed()const { return !bOpen; }
	inline void Close() { bOpen =false; }
	BearFlags<uint32> StateFlags;

	bool bOpen;
private:
	BearVector<BearUIObject*> m_Objects;
};