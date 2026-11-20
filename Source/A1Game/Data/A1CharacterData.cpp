#include "Data/A1CharacterData.h"
#include "System/LyraAssetManager.h"
#include "UObject/ObjectSaveContext.h"

const UA1CharacterData& UA1CharacterData::Get()
{
	return ULyraAssetManager::Get().GetCharacterData();
}

#if WITH_EDITOR
void UA1CharacterData::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	Super::PreSave(ObjectSaveContext);

	DefaultArmorMeshMap.KeySort([](const ECharacterSkinType A, const ECharacterSkinType B)
		{
			return (A < B);
		});
}
#endif // WITH_EDITOR

const FA1DefaultArmorMeshSet& UA1CharacterData::GetDefaultArmorMeshSet(ECharacterSkinType CharacterSkinType) const
{
	if (DefaultArmorMeshMap.Contains(CharacterSkinType) == false)
	{
		static FA1DefaultArmorMeshSet EmptyEntry;
		return EmptyEntry;
	}

	return DefaultArmorMeshMap.FindChecked(CharacterSkinType);
}