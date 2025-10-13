// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CrimAttributeSet.h"
#include "HitPointsAttributeSet.generated.h"

/**
 * Class that defines attributes that are necessary for taking damage.
 * Attribute examples include: HitPoints, Shields, and armor parts.
 */
UCLASS()
class CRIMABILITYSYSTEM_API UHitPointsAttributeSet : public UCrimAttributeSet
{
	GENERATED_BODY()

public:

	UHitPointsAttributeSet();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	ATTRIBUTE_ACCESSORS(UHitPointsAttributeSet, HitPoints);
	ATTRIBUTE_ACCESSORS(UHitPointsAttributeSet, MaxHitPoints);
	ATTRIBUTE_ACCESSORS(UHitPointsAttributeSet, Healing);
	ATTRIBUTE_ACCESSORS(UHitPointsAttributeSet, Damage);

	// Delegate when hit points changes due to damage/healing, some information may be missing on the client
	mutable FCrimAttributeSignature OnHitPointsUpdatedDelegate;

	// Delegate when max hit points changes
	mutable FCrimAttributeSignature OnMaxHitPointsUpdatedDelegate;

	// Delegate to broadcast when the HitPoints attribute reaches zero
	mutable FCrimAttributeSignature OnOutOfHitPointsDelegate;

	// Delegate to broadcast when the HitPoints attribute goes from zero to greater than 0.
	mutable FCrimAttributeSignature OnHitPointsUpdatedFromZeroDelegate;

protected:

	UFUNCTION()
	void OnRep_HitPoints(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxHitPoints(const FGameplayAttributeData& OldValue);

	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

	virtual void HandleDamage(const FGameplayEffectModCallbackData& Data, float Magnitude);
	virtual void HandleHealing(const FGameplayEffectModCallbackData& Data, float Magnitude);

private:

	/**
	 * The current HitPoints attribute. The value will be capped by the MaxHitPoints attribute.
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HitPoints, Category = "Crim Ability System||HitPoints", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData HitPoints;

	/**
	 * The current MaxHitPoints attribute. MaxHitPoints is an attribute since gameplay effects can modify it.
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHitPoints, Category = "Crim Ability System||HitPoints", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHitPoints;

	// -------------------------------------------------------------------
	//	Meta Attribute (please keep attributes that aren't 'stateful' below 
	// -------------------------------------------------------------------

	// Incoming healing. This is mapped directly to +Health
	UPROPERTY(BlueprintReadOnly, Category="Crim Ability System||HitPoints", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Healing;

	// Incoming damage. This is mapped directly to -Health
	UPROPERTY(BlueprintReadOnly, Category="Crim Ability System||HitPoints", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Damage;

	// Used to track when HitPoints reaches 0.
	bool bOutOfHitPoints = false;
	
	// Store the HitPoints before any changes
	float HitPointsBeforeAttributeChange = 0.0f;
	float MaxHitPointsBeforeAttributeChange = 0.0f;
};
