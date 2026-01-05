// Copyright Soccertitan 2025


#include "AbilityGameplayTags.h"

#include "GameplayTagsManager.h"

FAbilityGameplayTags FAbilityGameplayTags::GameplayTags;

void FAbilityGameplayTags::InitializeNativeGameplayTags()
{
	GameplayTags.Ability = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability"), FString("Root Gameplay Tag for Ability related events."));
	GameplayTags.Gameplay = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Gameplay"), FString("Root Gameplay Tag to define different Gameplay related GameplayTags."));
	GameplayTags.Input = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Input"), FString("Root Gameplay Tag for input related events."));
	GameplayTags.Message = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Message"), FString("Root Gameplay Tag to send messages via Gameplay Message Subsystem."));

	GameplayTags.Ability_ActivateFail_ActivationGroup = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.ActivateFail.ActivationGroup"), FString("Ability Failed due to activation group requirements."));
	GameplayTags.Ability_ActivateFail_IsDead = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.ActivateFail.IsDead"), FString("Ability failed to activate due to death."));
	GameplayTags.Ability_Cooldown = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Cooldown"), FString("Root gameplay tag for all cooldown ability tags."));
	GameplayTags.Ability_GameplayEvent_Death = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.GameplayEvent.Death"), FString("Triggers death gameplay abilities."));
	GameplayTags.Ability_GameplayEvent_Revive = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.GameplayEvent.Revive"), FString("Triggers revive gameplay abilities."));
	GameplayTags.Ability_InputBlocked = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.InputBlocked"), FString("Blocks abilities from being activated by player input."));
	
	GameplayTags.Gameplay_State_Death = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Gameplay.State.Death"), FString("The character has the death status."));
	GameplayTags.Gameplay_State_Death_Dead = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Gameplay.State.Death.Dead"), FString("The character has finished the death process."));
	GameplayTags.Gameplay_State_Death_Dying = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Gameplay.State.Death.Dying"), FString("The character has begun the death process."));
	GameplayTags.Gameplay_State_Death_Reviving = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Gameplay.State.Death.Reviving"), FString("The character has begun the reviving process."));

	GameplayTags.Message_Ability_Activate_Failure = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Message.Ability.Activate.Failure"), FString("A generic tag indicating the ability failed to activate. Used in the gameplay messaging subsystem."));
}
