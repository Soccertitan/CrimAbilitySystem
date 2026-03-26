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
	FGameplayTag Input;
	FGameplayTag Message;

	/**
	 * Ability Tags
	 */
	FGameplayTag Ability_ActivateFail_ActivationGroup;
	FGameplayTag Ability_ActivateFail_IsDead;
	FGameplayTag Ability_GameplayEvent_Death;
	FGameplayTag Ability_GameplayEvent_Revive;
	FGameplayTag Ability_InputBlocked;

	/**
	 * Ability State Tags
	 */
	FGameplayTag Ability_State;
	FGameplayTag Ability_State_Death;
	FGameplayTag Ability_State_Death_Dead;
	FGameplayTag Ability_State_Death_Dying;
	FGameplayTag Ability_State_Death_Reviving;

	/**
	 * Message Tags
	 */
	FGameplayTag Message_Ability_Activate_Failure;
	
	FGameplayTag SetByCaller_Cooldown;
};
