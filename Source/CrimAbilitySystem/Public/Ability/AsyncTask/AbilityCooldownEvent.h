// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AbilityCooldownEvent.generated.h"

class UAbilitySystemComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityCooldownEventSignature, float, TimeRemaining);

/**
 * Listens for cooldown gameplay tags and reports the duration remaining.
 */
UCLASS(BlueprintType, meta = (ExposedAsyncProxy = "AsyncTask"))
class CRIMABILITYSYSTEM_API UAbilityCooldownEvent : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FAbilityCooldownEventSignature CooldownStart;
	UPROPERTY(BlueprintAssignable)
	FAbilityCooldownEventSignature CooldownEnd;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UAbilityCooldownEvent* WaitForCooldownChange(UAbilitySystemComponent* InAbilitySystemComponent, const FGameplayTag& InCooldownTag);

	/**
	 * You must call this function manually when you want the AsyncTask to end.
	 * For UMG Widgets, you would call it in the Widget's Destruct event.
	 */
	UFUNCTION(BlueprintCallable)
	void EndTask();
	
protected:

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	FGameplayTag CooldownTag;

	void CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount);
	void OnActiveEffectAdded(UAbilitySystemComponent* TargetAbilitySystemComponent, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveEffectHandle);
};
