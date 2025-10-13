// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimGameplayAbility.h"
#include "DeathGameplayAbility.generated.h"

/**
 * Gameplay Ability used for handling death.
 * Ability is activated automatically via the "Ability.GameplayEvent.Death" ability trigger tag.
 */
UCLASS(Abstract)
class CRIMABILITYSYSTEM_API UDeathGameplayAbility : public UCrimGameplayAbility
{
	GENERATED_BODY()
public:

	UDeathGameplayAbility();

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// Starts the death sequence.
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Ability")
	void StartDeath();

	// Finishes the death sequence
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Ability")
	void FinishDeath();

	/**
	 * If enabled, the ability will automatically call StartDeath. FinishDeath is always called when the ability
	 * ends if the death was started.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Death")
	bool bAutoStartDeath = true;
};
