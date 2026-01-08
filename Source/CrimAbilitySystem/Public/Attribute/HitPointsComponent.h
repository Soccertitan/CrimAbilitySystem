// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "CrimAbilitySystemInterface.h"
#include "Components/ActorComponent.h"
#include "HitPointsComponent.generated.h"

struct FOnAttributeChangeData;
struct FGameplayEffectSpec;
class UCrimAbilitySystemComponent;
class UHitPointsAttributeSet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHitPointsDeathEventSignature, AActor*, OwningActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FHitPointsAttributeUpdatedSignature, UHitPointsComponent*, HitPointsComponent, float, OldValue, float, NewValue, AActor*, Instigator);

/**
 * EDeathState: Defines the current state of death.
 */
UENUM(BlueprintType)
enum class EDeathState : uint8
{
	Alive = 0,
	ReviveStarted,
	DeathStarted,
	DeathFinished
};

/**
 * An actor component used to handle anything related to HitPoints.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CRIMABILITYSYSTEM_API UHitPointsComponent : public UActorComponent, public ICrimAbilitySystemInterface
{
	GENERATED_BODY()

public:
	UHitPointsComponent();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	//~ ICrimAbilitySystemInterface
	virtual void InitializeWithAbilitySystem_Implementation(UCrimAbilitySystemComponent* NewAbilitySystemComponent) override;
	//~ end ICrimAbilitySystemInterface
	
	// Returns the Hit Points component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "Crim Ability System|HitPoints")
	static UHitPointsComponent* FindHitPointsComponent(const AActor* Actor);

	// Uninitialize the component, clearing any references to the ability system.
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|HitPoints")
	void UninitializeFromAbilitySystem();

	// Returns the current Hit Points value.
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|HitPoints")
	float GetHitPoints() const;

	// Returns the current maximum Hit Points value.
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|HitPoints")
	float GetMaxHitPoints() const;

	// Returns the current Hit Points in the range [0.0, 1.0].
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|HitPoints")
	float GetHitPointsNormalized() const;

	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|HitPoints")
	EDeathState GetDeathState() const { return DeathState; }

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Crim Ability System|HitPoints", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsDeadOrDying() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Crim Ability System|HitPoints", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsAliveOrReviving() const;

	// Begins the death sequence for the owner.
	virtual void StartDeath();

	// Ends the death sequence for the owner.
	virtual void FinishDeath();

	// Begins the revive sequence for the owner.
	virtual void StartRevive();

	// Ends the revive sequence for the owner.
	virtual void FinishRevive();

	// Applies enough damage to kill the owner.
	// virtual void DamageSelfDestruct(bool bFellOutOfWorld = false);

	// Delegate fired when the HitPoints value has updated. This is called on the client but the instigator may not be valid
	UPROPERTY(BlueprintAssignable, DisplayName = "OnHitPointsUpdated")
	FHitPointsAttributeUpdatedSignature OnHitPointsUpdatedDelegate;

	// Delegate fired when the max Hit Points value has updated. This is called on the client but the instigator may not be valid
	UPROPERTY(BlueprintAssignable, DisplayName = "OnMaxHitPointsUpdated")
	FHitPointsAttributeUpdatedSignature OnMaxHitPointsUpdatedDelegate;

	// Delegate fired when the death sequence has started.
	UPROPERTY(BlueprintAssignable, DisplayName = "OnDeathStarted")
	FHitPointsDeathEventSignature OnDeathStartedDelegate;

	// Delegate fired when the death sequence has finished.
	UPROPERTY(BlueprintAssignable, DisplayName = "OnDeathFinished")
	FHitPointsDeathEventSignature OnDeathFinishedDelegate;

	// Delegate fired when the Revive sequence has started.
	UPROPERTY(BlueprintAssignable, DisplayName = "OnReviveStarted")
	FHitPointsDeathEventSignature OnReviveStartedDelegate;

	// Delegate fired when the Revive sequence has finished.
	UPROPERTY(BlueprintAssignable, DisplayName = "OnReviveFinished")
	FHitPointsDeathEventSignature OnReviveFinishedDelegate;

protected:

	virtual void OnUnregister() override;

	void ClearGameplayTags();

	virtual void OnHitPointsUpdated(const FOnAttributeChangeData& Data);
	virtual void OnMaxHitPointsUpdated(const FOnAttributeChangeData& Data);
	virtual void OnOutOfHitPoints(AActor* Instigator, const FGameplayEffectSpec& EffectSpec, float Magnitude);
	virtual void OnHitPointsUpdatedFromZero(AActor* Instigator, const FGameplayEffectSpec& EffectSpec, float Magnitude);

	UFUNCTION()
	virtual void OnRep_DeathState(EDeathState OldDeathState);

private:
	// Ability system used by this component.
	UPROPERTY()
	TObjectPtr<UCrimAbilitySystemComponent> AbilitySystemComponent;

	// Replicated state used to handle dying.
	UPROPERTY(ReplicatedUsing = OnRep_DeathState)
	EDeathState DeathState = EDeathState::Alive;
};
