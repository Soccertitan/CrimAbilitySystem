// Copyright Soccertitan 2025


#include "Ability/GameplayEffect/CrimCooldownGameplayEffect.h"

#include "Ability/MMC/CrimMMC_AbilityCooldown.h"

UCrimCooldownGameplayEffect::UCrimCooldownGameplayEffect()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	FCustomCalculationBasedFloat CustomCalc;
	CustomCalc.CalculationClassMagnitude = UCrimMMC_AbilityCooldown::StaticClass();
	DurationMagnitude = FGameplayEffectModifierMagnitude(CustomCalc);
}
