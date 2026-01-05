// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "CrimAbilitySystemGlobals.generated.h"

/**
 * 
 */
UCLASS()
class CRIMABILITYSYSTEM_API UCrimAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
