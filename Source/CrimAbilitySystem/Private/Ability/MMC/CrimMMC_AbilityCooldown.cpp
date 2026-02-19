// Copyright Soccertitan 2025


#include "Ability/MMC/CrimMMC_AbilityCooldown.h"

#include "Ability/CrimGameplayAbility.h"

float UCrimMMC_AbilityCooldown::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const UCrimGameplayAbility* Ability = Cast<UCrimGameplayAbility>(Spec.GetContext().GetAbilityInstance_NotReplicated());

	if (!Ability)
	{
		return 0.0f;
	}

	return Ability->GetBaseCooldown();
}
