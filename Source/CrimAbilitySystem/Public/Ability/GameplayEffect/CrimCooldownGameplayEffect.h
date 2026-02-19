// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "CrimCooldownGameplayEffect.generated.h"

/**
 * A generic class for cooldowns for CrimGameplayAbilities.
 */
UCLASS()
class CRIMABILITYSYSTEM_API UCrimCooldownGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()
	
public:
	UCrimCooldownGameplayEffect();
};
