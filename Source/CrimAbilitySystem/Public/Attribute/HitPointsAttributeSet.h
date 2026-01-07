// Copyright Soccertitan 2025

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

	ATTRIBUTE_ACCESSORS(UHitPointsAttributeSet, CurrentPoints);
	ATTRIBUTE_ACCESSORS(UHitPointsAttributeSet, MaxPoints);
	ATTRIBUTE_ACCESSORS(UHitPointsAttributeSet, Healing);
	ATTRIBUTE_ACCESSORS(UHitPointsAttributeSet, Damage);

	// Delegate when current points changes due to damage/healing, some information may be missing on the client
	mutable FCrimAttributeSignature OnCurrentPointsUpdatedDelegate;

	// Delegate when max points changes
	mutable FCrimAttributeSignature OnMaxPointsUpdatedDelegate;

	// Delegate to broadcast when the CurrentPoints attribute reaches zero
	mutable FCrimAttributeSignature OnOutOfCurrentPointsDelegate;

	// Delegate to broadcast when the CurrentPoints attribute goes from zero to greater than 0.
	mutable FCrimAttributeSignature OnCurrentPointsUpdatedFromZeroDelegate;

protected:

	UFUNCTION()
	void OnRep_CurrentPoints(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxPoints(const FGameplayAttributeData& OldValue);

	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

	virtual void HandleDamage(const FGameplayEffectModCallbackData& Data, float Magnitude);
	virtual void HandleHealing(const FGameplayEffectModCallbackData& Data, float Magnitude);

private:

	/**
	 * The current HitPoints attribute. The value will be capped by the MaxHitPoints attribute.
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentPoints, Category = "Crim Ability System|HitPoints", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CurrentPoints;

	/**
	 * The current MaxHitPoints attribute. MaxHitPoints is an attribute since gameplay effects can modify it.
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxPoints, Category = "Crim Ability System|HitPoints", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxPoints;

	// -------------------------------------------------------------------
	//	Meta Attribute (please keep attributes that aren't 'stateful' below 
	// -------------------------------------------------------------------

	// Incoming healing. This is mapped directly to +Health
	UPROPERTY(BlueprintReadOnly, Category="Crim Ability System|HitPoints", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Healing;

	// Incoming damage. This is mapped directly to -Health
	UPROPERTY(BlueprintReadOnly, Category="Crim Ability System|HitPoints", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Damage;

	// Used to track when CurrentPoints reaches 0.
	bool bOutOfCurrentPoints = false;
	
	// Store the HitPoints before any changes
	float CurrentPointsBeforeAttributeChange = 0.0f;
	float MaxPointsBeforeAttributeChange = 0.0f;
};
