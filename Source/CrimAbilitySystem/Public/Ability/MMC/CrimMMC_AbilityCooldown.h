// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "CrimMMC_AbilityCooldown.generated.h"

/**
 * Gets the base cooldown from the Ability.
 */
UCLASS()
class CRIMABILITYSYSTEM_API UCrimMMC_AbilityCooldown : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
	
public:
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
