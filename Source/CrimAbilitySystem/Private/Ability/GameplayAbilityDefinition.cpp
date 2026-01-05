// Copyright Soccertitan 2025


#include "Ability/GameplayAbilityDefinition.h"

UGameplayAbilityDefinition::UGameplayAbilityDefinition()
{
	AssetType = TEXT("GameplayAbilityDefinition");
}

FPrimaryAssetId UGameplayAbilityDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(AssetType, GetFName());
}
