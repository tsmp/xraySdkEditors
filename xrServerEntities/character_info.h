//////////////////////////////////////////////////////////////////////////
// character_info.h			������, ��� ������������� ������������ ��������
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "character_info_defs.h"
#include "shared_data.h"
#include "xml_str_id_loader.h"

class NET_Packet;

#ifndef AI_COMPILER
#include "specific_character.h"
#endif

#ifdef XRGAME_EXPORTS
#include "PhraseDialogDefs.h"
#include "character_community.h"
#include "character_rank.h"
#include "character_reputation.h"
class CSE_ALifeTraderAbstract;
#endif

//////////////////////////////////////////////////////////////////////////
// SCharacterProfile: ������ ������� ���������
//////////////////////////////////////////////////////////////////////////
struct SCharacterProfile : CSharedResource
{
	SCharacterProfile();
	virtual ~SCharacterProfile();

	//���� ������, �� ���������� ������ ����� �������,
	//����� ������ ��������,��������������� �������
	shared_str m_CharacterId;

	//��������� ��������� ���������
	CHARACTER_CLASS m_Class;
	CHARACTER_RANK_VALUE m_Rank;
	CHARACTER_REPUTATION_VALUE m_Reputation;
};

class CInventoryOwner;
class CSE_ALifeTraderAbstract;

class CCharacterInfo : public CSharedClass<SCharacterProfile, shared_str, false>,
					   public CXML_IdToIndex<CCharacterInfo>
{
private:
	typedef CSharedClass<SCharacterProfile, shared_str, false> inherited_shared;
	typedef CXML_IdToIndex<CCharacterInfo> id_to_index;

	friend id_to_index;
	friend CInventoryOwner;
	friend CSE_ALifeTraderAbstract;

public:
	CCharacterInfo();
	~CCharacterInfo();

	virtual void Load(shared_str id);

#ifdef XRGAME_EXPORTS
	void load(IReader &);
	void save(NET_Packet &);

	//������������� ������� �������������
	//�������� ���������������� CSpecificCharacter, ��
	//���������� �������
	void Init(CSE_ALifeTraderAbstract *trader);
	void InitSpecificCharacter(shared_str new_id);
#endif

protected:
	const SCharacterProfile *data() const
	{
		VERIFY(inherited_shared::get_sd());
		return inherited_shared::get_sd();
	}
	SCharacterProfile *data()
	{
		VERIFY(inherited_shared::get_sd());
		return inherited_shared::get_sd();
	}

	static void InitXmlIdToIndex();

	//�������� �� XML �����
	virtual void load_shared(LPCSTR);

	//������ ������������ �������
	shared_str m_ProfileId;

	//������ ������ � ���������� ���������, �������
	//������������ � ������ ���������� ������
	shared_str m_SpecificCharacterId;

#ifdef XRGAME_EXPORTS
	shared_str m_StartDialog;

	//����������� ���������� � ���������� ���������
	CSpecificCharacter m_SpecificCharacter;
#endif

public:
#ifdef XRGAME_EXPORTS
	shared_str Profile() const;
	LPCSTR Name() const;
	shared_str Bio() const;

	const CHARACTER_COMMUNITY &Community() const { return m_CurrentCommunity; }
	const CHARACTER_RANK &Rank() const { return m_CurrentRank; }
	const CHARACTER_REPUTATION &Reputation() const { return m_CurrentReputation; }
	float Sympathy() const { return m_Sympathy; }
	void SetSympathy(float sympathy) { m_Sympathy = sympathy; }

	//�������� ������ � InventoryOwner
protected:
	void SetRank(CHARACTER_RANK_VALUE rank);
	void SetReputation(CHARACTER_REPUTATION_VALUE reputation);
	void SetCommunity(CHARACTER_COMMUNITY_INDEX community);

public:
	const shared_str &IconName() const;

	shared_str StartDialog() const;
	const DIALOG_ID_VECTOR &ActorDialogs() const;
#endif

protected:
#ifdef XRGAME_EXPORTS
	CHARACTER_RANK m_CurrentRank;
	CHARACTER_REPUTATION m_CurrentReputation;
	CHARACTER_COMMUNITY m_CurrentCommunity;
	float m_Sympathy; // % ������� �� �����������
#endif
};