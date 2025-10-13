// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimGameplayAbility.h"
#include "ReviveGameplayAbility.generated.h"

/**
 * Gameplay Ability used for handling resurrection.
 * Ability is activated automatically via the "Ability.GameplayEvent.Revive" ability trigger tag.
 */
UCLASS(Abstract)
class CRIMABILITYSYSTEM_API UReviveGameplayAbility : public UCrimGameplayAbility
{
	GENERATED_BODY()
public:

	UReviveGameplayAbility();

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// Starts the resurrection sequence.
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Ability")
	void StartRevive();

	// Finishes the resurrection sequence
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Ability")
	void FinishRevive();

	/**
	 * If enabled, the ability will automatically call StartRevive. FinishRevive is always called when the ability
	 * ends if the resurrection was started.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Revive")
	bool bAutoStartRevive = true;
};
