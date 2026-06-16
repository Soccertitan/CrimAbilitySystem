// Copyright Soccertitan 2026

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AbilityInputGameplayEventData.generated.h"

class UAbilityInput;
class UCrimAbilitySystemComponent;
struct FGameplayEventData;

/**
 * Used in an AbilityInput to generate GameplayEventData to be used when activating abilities as input.
 */
UCLASS(DefaultToInstanced, Const, Blueprintable, Abstract)
class CRIMABILITYSYSTEM_API UAbilityInputGameplayEventData : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent)
	FGameplayEventData MakeGameplayEventData(UCrimAbilitySystemComponent* AbilitySystemComponent, const UAbilityInput* AbilityInput) const;
	
	FGameplayEventData MakeGameplayEventData_Implementation(UCrimAbilitySystemComponent* AbilitySystemComponent, const UAbilityInput* AbilityInput) const;
};
