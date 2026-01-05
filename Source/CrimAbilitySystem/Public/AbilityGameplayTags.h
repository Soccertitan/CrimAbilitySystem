// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * AbilityGameplayTags
 * Singleton containing native gameplay tags.
 */

struct CRIMABILITYSYSTEM_API FAbilityGameplayTags
{
	static const FAbilityGameplayTags& Get() {return GameplayTags;}
	static void InitializeNativeGameplayTags();

private:
	static FAbilityGameplayTags GameplayTags;
	
public:

	/**
	 * Generic Root Gameplay Tags
	 */
	FGameplayTag Ability;
	FGameplayTag Gameplay;
	FGameplayTag Input;
	FGameplayTag Message;

	/**
	 * Ability Tags
	 */
	FGameplayTag Ability_ActivateFail_ActivationGroup;
	FGameplayTag Ability_ActivateFail_IsDead;
	FGameplayTag Ability_Cooldown;
	FGameplayTag Ability_GameplayEvent_Death;
	FGameplayTag Ability_GameplayEvent_Revive;
	FGameplayTag Ability_InputBlocked;

	/**
	 * Gameplay State Tags
	 */
	FGameplayTag Gameplay_State_Death;
	FGameplayTag Gameplay_State_Death_Dead;
	FGameplayTag Gameplay_State_Death_Dying;
	FGameplayTag Gameplay_State_Death_Reviving;

	/**
	 * Message Tags
	 */
	FGameplayTag Message_Ability_Activate_Failure;
};
