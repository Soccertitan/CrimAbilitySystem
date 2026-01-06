// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CrimAttributeSet.h"
#include "TacticalPointsAttributeSet.generated.h"

/**
 * Class that defines attributes that are necessary for activating abilities. Like mana, action points, etc.
 */
UCLASS()
class CRIMABILITYSYSTEM_API UTacticalPointsAttributeSet : public UCrimAttributeSet
{
	GENERATED_BODY()

public:

	UTacticalPointsAttributeSet();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	ATTRIBUTE_ACCESSORS(UTacticalPointsAttributeSet, CurrentPoints);
	ATTRIBUTE_ACCESSORS(UTacticalPointsAttributeSet, MaxPoints);
	ATTRIBUTE_ACCESSORS(UTacticalPointsAttributeSet, Healing);
	ATTRIBUTE_ACCESSORS(UTacticalPointsAttributeSet, Damage);

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

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

	virtual void HandleDamage(const FGameplayEffectModCallbackData& Data, float Magnitude);
	virtual void HandleHealing(const FGameplayEffectModCallbackData& Data, float Magnitude);

private:

	/**
	 * The current TacticalPoints attribute. The value will be capped by the MaxTacticalPoints attribute.
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentPoints, Category = "Crim|TacticalPoints", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CurrentPoints;

	/**
	 * The current MaxTacticalPoints attribute. MaxTacticalPoints is an attribute since gameplay effects can modify it.
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxPoints, Category = "Crim|TacticalPoints", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxPoints;

	// -------------------------------------------------------------------
	//	Meta Attribute (please keep attributes that aren't 'stateful' below 
	// -------------------------------------------------------------------

	// Incoming healing. This is mapped directly to +CurrentPoints
	UPROPERTY(BlueprintReadOnly, Category="Crim|TacticalPoints", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Healing;

	// Incoming damage. This is mapped directly to -CurrentPoints
	UPROPERTY(BlueprintReadOnly, Category="Crim|TacticalPoints", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Damage;
};
